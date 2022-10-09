// #define DEBUG_BER

#ifndef SNMPManager_h
#define SNMPManager_h

#ifndef UDP_TX_PACKET_MAX_SIZE
#define UDP_TX_PACKET_MAX_SIZE 484
#endif

#ifndef SNMP_PACKET_LENGTH
#if defined(ESP32)
#define SNMP_PACKET_LENGTH 1500 // This will limit the size of packets which can be handled.
#else
#define SNMP_PACKET_LENGTH 512 // This value may need to be made smaller for lower memory devices.
#endif
#endif

#define MIN(X, Y) ((X < Y) ? X : Y)

#include <Udp.h>

#include "BER.h"
#include "VarBinds.h"
#include "SNMPGet.h"
#include "SNMPGetResponse.h"
#include "CallbackTypes.h"

class SNMPManager
{
public:
    SNMPManager(){};
    SNMPManager(const char *community) : _community(community){};
    const char *_community;

    ValueCallbacks *callbacks = new ValueCallbacks();
    ValueCallbacks *callbacksCursor = callbacks;
    ValueCallback *findCallback(IPAddress ip, const char *oid)
    {
        callbacksCursor = callbacks;

        if (callbacksCursor->value)
        {
            while (true)
            {
                memset(OIDBuf, 0, MAX_OID_LENGTH);
                strcat(OIDBuf, callbacksCursor->value->OID);
                if ((strcmp(OIDBuf, oid) == 0) && (callbacksCursor->value->ip == ip))
                {
// Found
#ifdef DEBUG
                    Serial.println(F("[DEBUG] Found callback with matching IP"));
#endif
                    return callbacksCursor->value;
                }
                if (callbacksCursor->next)
                {
                    callbacksCursor = callbacksCursor->next;
                }
                else
                {
#ifdef DEBUG
                    Serial.println(F("[DEBUG] No matching callback found. Must be from GetNextRequest"));
#endif
                    return callbacksCursor->value;
                }
            }
        }
        return 0;
    }

    ValueCallback *addStringHandler(IPAddress ip, const char *oid, char **value)
    {
        ValueCallback *callback = new StringCallback();
        ((StringCallback *)callback)->value = value;
        return configHandler(ip, oid, callback);
    }

    ValueCallback *addIntegerHandler(IPAddress ip, const char *oid, int *value)
    {
        ValueCallback *callback = new IntegerCallback();
        ((IntegerCallback *)callback)->value = value;
        ((IntegerCallback *)callback)->isFloat = false;
        return configHandler(ip, oid, callback);
    }

    ValueCallback *addFloatHandler(IPAddress ip, const char *oid, float *value)
    {
        ValueCallback *callback = new IntegerCallback();
        ((IntegerCallback *)callback)->value = (int *)value;
        ((IntegerCallback *)callback)->isFloat = true;
        return configHandler(ip, oid, callback);
    }

    ValueCallback *addTimestampHandler(IPAddress ip, const char *oid, uint32_t *value)
    {
        ValueCallback *callback = new TimestampCallback();
        ((TimestampCallback *)callback)->value = value;
        return configHandler(ip, oid, callback);
    }

    ValueCallback *addOIDHandler(IPAddress ip, const char *oid, char *value)
    {
        ValueCallback *callback = new OIDCallback();
        ((OIDCallback *)callback)->value = value;
        return configHandler(ip, oid, callback);
    }

    ValueCallback *addCounter64Handler(IPAddress ip, const char *oid, uint64_t *value)
    {
        ValueCallback *callback = new Counter64Callback();
        ((Counter64Callback *)callback)->value = value;
        return configHandler(ip, oid, callback);
    }

    ValueCallback *addCounter32Handler(IPAddress ip, const char *oid, uint32_t *value)
    {
        ValueCallback *callback = new Counter32Callback();
        ((Counter32Callback *)callback)->value = value;
        return configHandler(ip, oid, callback);
    }

    ValueCallback *addGuageHandler(IPAddress ip, const char *oid, uint32_t *value)
    {
        ValueCallback *callback = new Guage32Callback();
        ((Guage32Callback *)callback)->value = value;
        return configHandler(ip, oid, callback);
    }
    ValueCallback *addNextRequestHandler(IPAddress ip, const char *oid, ValueCallback **value)
    {
        ValueCallback *callback = new StringCallback();
        ((NextRequestOID *)callback)->value = value;
        return configHandler(ip, oid, callback);
    }

    ValueCallback *configHandler(IPAddress ip, const char *oid, ValueCallback *callback)
    {
        callback->OID = (char *)malloc((sizeof(char) * strlen(oid)) + 1);
        callback->ip = ip;
        strcpy(callback->OID, oid);
        addHandler(callback);
        return callback;
    }
    void addHandler(ValueCallback *callback)
    {
        callbacksCursor = callbacks;
        if (callbacksCursor->value)
        {
            while (callbacksCursor->next != 0)
            {
                callbacksCursor = callbacksCursor->next;
            }
            callbacksCursor->next = new ValueCallbacks();
            callbacksCursor = callbacksCursor->next;
            callbacksCursor->value = callback;
            callbacksCursor->next = 0;
        }
        else
            callbacks->value = callback;
    }
    void setUDP(UDP *udp)
    {
        if (_udp)
        {
            _udp->stop();
        }
        _udp = udp;
        this->begin();
    }

    bool begin()
    {
        if (!_udp)
            return false;
        _udp->begin(162);
        return true;
    }

    bool loop()
    {
        if (!_udp)
        {
            return false;
        }
        return receivePacket(_udp->parsePacket());
    }

    bool testParsePacket(String testPacket)
    {
        // Function to test sample packet, each byte to be seperated with a space:
        // e.g. "32 02 01 01 04 06 70 75 62 6c 69 63 a2 25 02 02 0c 01 02 01 00 02 c1 00 30 19 30 17 06 11 2b 06 01 04 01 81 9e 16 02 03 01 01 01 02 03 01 00 02 02 14 9f";
        int len = testPacket.length() + 1;
        memset(_packetBuffer, 0, SNMP_PACKET_LENGTH * 3);
        char charArrayPacket[len];
        testPacket.toCharArray(charArrayPacket, len);
        // split charArray at each ' ' and convert to uint8_t
        char *p = strtok(charArrayPacket, " ");
        int i = 0;
        while (p != NULL)
        {
            _packetBuffer[i++] = strtoul(p, NULL, 16);
            p = strtok(NULL, " ");
        }
        _packetBuffer[i] = 0; // null terminate the buffer
#ifdef DEBUG
        printPacket(len);
#endif

        return parsePacket();
    }

    char OIDBuf[MAX_OID_LENGTH];
    UDP *_udp;

private:
    unsigned char _packetBuffer[SNMP_PACKET_LENGTH * 3];
    bool inline receivePacket(int packetLength)
    {
        if (!packetLength)
        {
            return false;
        }
#ifdef DEBUG
        Serial.print(F("[DEBUG] Packet Length: "));
        Serial.print(packetLength);
        Serial.print(F(" From Address: "));
        Serial.println(_udp->remoteIP());
#endif

        memset(_packetBuffer, 0, SNMP_PACKET_LENGTH * 3);
        int len = packetLength;
        _udp->read(_packetBuffer, MIN(len, SNMP_PACKET_LENGTH));
        _udp->flush();
        _packetBuffer[len] = 0; // null terminate the buffer

#ifdef DEBUG
        printPacket(len);
#endif

        return parsePacket();
    }

    bool parsePacket()
    {
        SNMPGetResponse *snmpgetresponse = new SNMPGetResponse();
        if (snmpgetresponse->parseFrom(_packetBuffer))
        {
            if (snmpgetresponse->requestType == GetResponsePDU)
            {
                if (!(snmpgetresponse->version != 1 || snmpgetresponse->version != 2) || strcmp(_community, snmpgetresponse->communityString) != 0)
                {
                    Serial.print(F("Invalid community or version - Community: "));
                    Serial.print(snmpgetresponse->communityString);
                    Serial.print(F(" - Version: "));
                    Serial.println(snmpgetresponse->version);
                    delete snmpgetresponse;
                    return false;
                }
#ifdef DEBUG
                Serial.print(F("[DEBUG] Community: "));
                Serial.println(snmpgetresponse->communityString);
                Serial.print(F("[DEBUG] SNMP Version: "));
                Serial.println(snmpgetresponse->version);
#endif
                int varBindIndex = 1;
                snmpgetresponse->varBindsCursor = snmpgetresponse->varBinds;
                while (true)
                {
                    char *responseOID = snmpgetresponse->varBindsCursor->value->oid->_value;
                    IPAddress responseIP = _udp->remoteIP();
                    ASN_TYPE responseType = snmpgetresponse->varBindsCursor->value->type;
                    BER_CONTAINER *responseContainer = snmpgetresponse->varBindsCursor->value->value;
#ifdef DEBUG
                    Serial.print(F("[DEBUG] Response from: "));
                    Serial.print(responseIP);
                    Serial.print(F(" - OID: "));
                    Serial.println(responseOID);
#endif
                    ValueCallback *callback = findCallback(responseIP, responseOID);
                    if (!callback)
                    {
                        Serial.print(F("Matching callback not found for recieved SNMP response. Response OID: "));
                        Serial.print(responseOID);
                        Serial.print(F(" - From IP Address: "));
                        Serial.println(responseIP);
                        Serial.print(F("Request type:"));
                        Serial.println(snmpgetresponse->requestType);
                        delete snmpgetresponse;
                        return false;
                    }
                    ASN_TYPE callbackType = callback->type;
                    if (callbackType != responseType)
                    {
                        switch (responseType)
                        {
                        case NOSUCHOBJECT:
                        {
                            Serial.print(F("No such object: "));
                        }
                        break;
                        case NOSUCHINSTANCE:
                        {
                            Serial.print(F("No such instance: "));
                        }
                        break;
                        case ENDOFMIBVIEW:
                        {
                            Serial.print(F("End of MIB view when calling: "));
                        }
                        break;
                        default:
                        {
                            Serial.print(F("Incorrect Callback type. Expected: "));
                            Serial.print(callbackType);
                            Serial.print(F(" Received: "));
                            Serial.print(responseType);
                            Serial.print(F(" - When calling: "));
                        }
                        }
                        delete snmpgetresponse;
                        snmpgetresponse = 0;
                        return false;
                    }
                    switch (callbackType)
                    {
                    case STRING:
                    {
#ifdef DEBUG
                        Serial.println("[DEBUG] Type: String");
#endif

                        // Note: Requires that the size of the variable used to store the response is big enough.
                        // Otherwise move responsibility for the creation of the variable to store the value here, but this would put the onus on the caller to free and reset to null.
                        //*((StringCallback *)callback)->value = (char *)malloc(64); // Allocate memory for string, caller will need to free. Malloc updated to incoming message size.
                        strncpy(*((StringCallback *)callback)->value, ((OctetType *)responseContainer)->_value, strlen(((OctetType *)responseContainer)->_value));
                        OctetType *value = new OctetType(*((StringCallback *)callback)->value);
                        delete value;
                    }
                    break;
                    case INTEGER:
                    {
#ifdef DEBUG
                        Serial.println("[DEBUG] Type: Integer");
#endif
                        IntegerType *value = new IntegerType();
                        if (!((IntegerCallback *)callback)->isFloat)
                        {
                            *(((IntegerCallback *)callback)->value) = ((IntegerType *)responseContainer)->_value;
                            value->_value = *(((IntegerCallback *)callback)->value);
                        }
                        else
                        {
                            *(((IntegerCallback *)callback)->value) = (float)(((IntegerType *)responseContainer)->_value / 10);
                            value->_value = *(float *)(((IntegerCallback *)callback)->value) * 10;
                        }
                        delete value;
                    }
                    break;
                    case COUNTER32:
                    {
#ifdef DEBUG
                        Serial.println("[DEBUG] Type: Counter32");
#endif
                        Counter32 *value = new Counter32();
                        *(((Counter32Callback *)callback)->value) = ((Counter32 *)responseContainer)->_value;
                        value->_value = *(((Counter32Callback *)callback)->value);
                        delete value;
                    }
                    break;
                    case COUNTER64:
                    {
#ifdef DEBUG
                        Serial.println("[DEBUG] Type: Counter64");
#endif
                        Counter64 *value = new Counter64();
                        *(((Counter64Callback *)callback)->value) = ((Counter64 *)responseContainer)->_value;
                        value->_value = *(((Counter64Callback *)callback)->value);
                        delete value;
                    }
                    break;
                    case GUAGE32:
                    {
#ifdef DEBUG
                        Serial.println("[DEBUG] Type: Guage32");
#endif
                        Guage *value = new Guage();
                        *(((Guage32Callback *)callback)->value) = ((Guage *)responseContainer)->_value;
                        value->_value = *(((Guage32Callback *)callback)->value);
                        delete value;
                    }
                    break;
                    case TIMESTAMP:
                    {
#ifdef DEBUG
                        Serial.println("[DEBUG] Type: TimeStamp");
#endif
                        TimestampType *value = new TimestampType();
                        *(((TimestampCallback *)callback)->value) = ((TimestampType *)responseContainer)->_value;
                        value->_value = *(((TimestampCallback *)callback)->value);
                        delete value;
                    }
                    break;
                    default:
                    {
                        Serial.print(F("Unsupported Type: "));
                        Serial.print(callbackType);
                    }
                    break;
                    }
                    snmpgetresponse->varBindsCursor = snmpgetresponse->varBindsCursor->next;
                    if (!snmpgetresponse->varBindsCursor->value)
                    {
                        break;
                    }
                    varBindIndex++;
                } // End while
            }     // End if GetResponsePDU
        }
        else
        {
            Serial.println(F("SNMPGETRESPONSE: FAILED TO PARSE"));
            delete snmpgetresponse;
            return false;
        }
#ifdef DEBUG
        Serial.println(F("[DEBUG] SNMPGETRESPONSE: SUCCESS"));
#endif
        delete snmpgetresponse;
        return true;
    }
    void printPacket(int len)
    {
        Serial.print("[DEBUG] packet: ");
        for (int i = 0; i < len; i++)
        {
            Serial.printf("%02x ", _packetBuffer[i]);
        }
        Serial.println();
    }
};
#endif
