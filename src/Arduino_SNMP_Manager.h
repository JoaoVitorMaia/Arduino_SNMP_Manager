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
    const char *_community;
    SNMPGet *_snmpRequest;
    ValueCallbacks *callbacks = new ValueCallbacks();
    ValueCallbacks *callbacksCursor = callbacks;
    char OIDBuf[MAX_OID_LENGTH];
    UDP *_udp = 0;

public:
    SNMPManager(){};
    SNMPManager(const char *community, int snmpVersion) : _community(community)
    {
        _snmpRequest = new SNMPGet(community, snmpVersion);
    }
    ValueCallback *addStringHandler(IPAddress ip, const char *oid, int timeout)
    {
        ValueCallback *callback = new StringCallback();
        callback->OID = (char *)malloc((sizeof(char) * strlen(oid)) + 1);
        strcpy(callback->OID, oid);
        callback->ip = ip;
        addHandler(callback);
        send(ip, callback);
        return this->receivePacket(timeout);
    }

    ValueCallback *addIntegerHandler(IPAddress ip, const char *oid, int timeout)
    {
        ValueCallback *callback = new IntegerCallback();
        callback->OID = (char *)malloc((sizeof(char) * strlen(oid)) + 1);
        strcpy(callback->OID, oid);
        ((IntegerCallback *)callback)->isFloat = false;
        callback->ip = ip;
        addHandler(callback);
        send(ip, callback);
        return this->receivePacket(timeout);
    }
    ValueCallback *addFloatHandler(IPAddress ip, const char *oid, int timeout)
    {
        ValueCallback *callback = new IntegerCallback();
        callback->OID = (char *)malloc((sizeof(char) * strlen(oid)) + 1);
        strcpy(callback->OID, oid);
        ((IntegerCallback *)callback)->isFloat = true;
        callback->ip = ip;
        addHandler(callback);
        send(ip, callback);
        return this->receivePacket(timeout);
    }

    ValueCallback *addTimestampHandler(IPAddress ip, const char *oid, int timeout)
    {
        ValueCallback *callback = new TimestampCallback();
        callback->OID = (char *)malloc((sizeof(char) * strlen(oid)) + 1);
        strcpy(callback->OID, oid);
        callback->ip = ip;
        addHandler(callback);
        send(ip, callback);
        return this->receivePacket(timeout);
    }

    ValueCallback *addOIDHandler(IPAddress ip, const char *oid, int timeout)
    {
        ValueCallback *callback = new OIDCallback();
        callback->OID = (char *)malloc((sizeof(char) * strlen(oid)) + 1);
        callback->ip = ip;
        addHandler(callback);
        send(ip, callback);
        return this->receivePacket(timeout);
    }

    ValueCallback *addCounter64Handler(IPAddress ip, const char *oid, int timeout)
    {
        ValueCallback *callback = new Counter64Callback();
        callback->OID = (char *)malloc((sizeof(char) * strlen(oid)) + 1);
        strcpy(callback->OID, oid);
        callback->ip = ip;
        addHandler(callback);
        send(ip, callback);
        return this->receivePacket(timeout);
    }

    ValueCallback *addCounter32Handler(IPAddress ip, const char *oid, int timeout)
    {
        ValueCallback *callback = new Counter32Callback();
        callback->OID = (char *)malloc((sizeof(char) * strlen(oid)) + 1);
        strcpy(callback->OID, oid);
        callback->ip = ip;
        addHandler(callback);
        send(ip, callback);
        return this->receivePacket(timeout);
    }

    ValueCallback *addGaugeHandler(IPAddress ip, const char *oid, int timeout)
    {
        ValueCallback *callback = new Gauge32Callback();
        callback->OID = (char *)malloc((sizeof(char) * strlen(oid)) + 1);
        strcpy(callback->OID, oid);
        callback->ip = ip;
        addHandler(callback);
        send(ip, callback);
        return this->receivePacket(timeout);
    }
    void deleteCallback(IPAddress ip, const char *oid)
    {
        callbacksCursor = callbacks;
        ValueCallbacks *previous = nullptr;

        while (callbacksCursor != nullptr)
        {
            if (strcmp(callbacksCursor->value->OID, oid) == 0 && callbacksCursor->value->ip == ip)
            {
                // Match found, delete the callback
                ValueCallbacks *toDelete = callbacksCursor;

                // Update the linked list
                if (previous != nullptr)
                {
                    previous->next = callbacksCursor->next;
                }
                else
                {
                    callbacks = callbacksCursor->next;
                }

                callbacksCursor = callbacksCursor->next;

                delete toDelete->value;
                delete toDelete;
            }
            else
            {
                previous = callbacksCursor;
                callbacksCursor = callbacksCursor->next;
            }
        }
        if(callbacks == nullptr)
            callbacks = new ValueCallbackList();
    }

    void setUDP(UDP *udp)
    {
        if (_udp)
        {
            _udp->stop();
        }
        _snmpRequest->setUDP(udp);
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

private:
    unsigned char _packetBuffer[SNMP_PACKET_LENGTH * 3];
    void send(IPAddress ip, ValueCallback *callback)
    {
        _snmpRequest->addOIDPointer(callback);
        _snmpRequest->setIP(WiFi.localIP());
        _snmpRequest->setRequestID(rand() % 5555);
        _snmpRequest->sendTo(ip);
        _snmpRequest->clearOIDList();
    }
    ValueCallback *receivePacket(int timeout)
    {
        int time = millis();

        int packetLength = _udp->parsePacket();
        while (!packetLength)
        {
            if ((millis() - time) / 1000 >= timeout)
                return NULL;
            packetLength = _udp->parsePacket();
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
    ValueCallback *parsePacket()
    {
        SNMPGetResponse *snmpgetresponse = new SNMPGetResponse();
        ValueCallback *callback;
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
                    return NULL;
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
                    callback = findCallback(responseIP, responseOID);
                    if (!callback)
                    {
                        Serial.print(F("Matching callback not found for recieved SNMP response. Response OID: "));
                        Serial.print(responseOID);
                        Serial.print(F(" - From IP Address: "));
                        Serial.println(responseIP);
                        delete snmpgetresponse;
                        return NULL;
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
                        Serial.println(responseOID);
                        delete snmpgetresponse;
                        snmpgetresponse = 0;
                        return NULL;
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
                        strncpy(((StringCallback *)callback)->value, ((OctetType *)responseContainer)->_value, strlen(((OctetType *)responseContainer)->_value));
                        OctetType *value = new OctetType(((StringCallback *)callback)->value);
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
                            (((IntegerCallback *)callback)->value) = ((IntegerType *)responseContainer)->_value;
                            value->_value = (((IntegerCallback *)callback)->value);
                        }
                        else
                        {
                            (((IntegerCallback *)callback)->value) = (float)(((IntegerType *)responseContainer)->_value / 10);
                            value->_value = (float)(((IntegerCallback *)callback)->value) * 10;
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
                        (((Counter32Callback *)callback)->value) = ((Counter32 *)responseContainer)->_value;
                        value->_value = (((Counter32Callback *)callback)->value);
                        delete value;
                    }
                    break;
                    case COUNTER64:
                    {
#ifdef DEBUG
                        Serial.println("[DEBUG] Type: Counter64");
#endif
                        Counter64 *value = new Counter64();
                        (((Counter64Callback *)callback)->value) = ((Counter64 *)responseContainer)->_value;
                        value->_value = (((Counter64Callback *)callback)->value);
                        delete value;
                    }
                    break;
                    case GAUGE32:
                    {
#ifdef DEBUG
                        Serial.println("[DEBUG] Type: Gauge32");
#endif
                        Gauge *value = new Gauge();
                        (((Gauge32Callback *)callback)->value) = ((Gauge *)responseContainer)->_value;
                        value->_value = (((Gauge32Callback *)callback)->value);
                        delete value;
                    }
                    break;
                    case TIMESTAMP:
                    {
#ifdef DEBUG
                        Serial.println("[DEBUG] Type: TimeStamp");
#endif
                        TimestampType *value = new TimestampType();
                        (((TimestampCallback *)callback)->value) = ((TimestampType *)responseContainer)->_value;
                        value->_value = (((TimestampCallback *)callback)->value);
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
            return NULL;
        }
#ifdef DEBUG
        Serial.println(F("[DEBUG] SNMPGETRESPONSE: SUCCESS"));
#endif
        delete snmpgetresponse;
        return callback;
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
                    Serial.println(F("[DEBUG] No matching callback found."));
#endif
                    break;
                }
            }
        }
        return 0;
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
};
#endif