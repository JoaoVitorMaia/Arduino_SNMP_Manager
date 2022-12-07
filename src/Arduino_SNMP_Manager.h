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
#include "SNMPGetNext.h"
#include "SNMPGetResponse.h"
#include "CallbackTypes.h"

class SNMPManager
{
public:
    SNMPManager(){};
    SNMPManager(const char *community) : _community(community){};
    const char *_community;
    ValueCallbacks *results = new ValueCallbacks();
    ValueCallbacks *resultsCursor = results;
    ValueCallback *addNextRequestHandler(IPAddress ip, const char *oid);
    ValueCallback *addHandler(IPAddress ip, const char *oid, ASN_TYPE type);
    void addHandlerToResult(ValueCallback *callback);

    void setUDP(UDP *udp);
    bool begin();
    bool loop();
    bool testParsePacket(String testPacket);
    char OIDBuf[MAX_OID_LENGTH];
    UDP *_udp;
private:
    unsigned char _packetBuffer[SNMP_PACKET_LENGTH * 3];
    bool inline receivePacket(int length);
    bool parsePacket();
    void printPacket(int len);
};

void SNMPManager::setUDP(UDP *udp)
{
    if (_udp)
    {
        _udp->stop();
    }
    _udp = udp;
    this->begin();
}

bool SNMPManager::begin()
{
    if (!_udp)
        return false;
    _udp->begin(162);
    return true;
}

bool SNMPManager::loop()
{
    if (!_udp)
    {
        return false;
    }
    return receivePacket(_udp->parsePacket());
}

void SNMPManager::printPacket(int len)
{
    Serial.print("[DEBUG] packet: ");
    for (int i = 0; i < len; i++)
    {
        Serial.printf("%02x ", _packetBuffer[i]);
    }
    Serial.println();
}

bool inline SNMPManager::receivePacket(int packetLength)
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

bool SNMPManager::parsePacket()
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
                ValueCallback *callback = 0;
                switch (responseType)
                {
                case INTEGER:
                    callback = addHandler(responseIP, responseOID, INTEGER);
                    break;
                case STRING:
                    callback = addHandler(responseIP, responseOID, STRING);
                    break;
                case OID:
                    callback = addHandler(responseIP, responseOID, OID);
                    break;
                case COUNTER32:
                    callback = addHandler(responseIP, responseOID, COUNTER32);
                    break;
                case GAUGE32:
                    callback = addHandler(responseIP, responseOID, GAUGE32);
                    break;
                case TIMESTAMP:
                    callback = addHandler(responseIP, responseOID, TIMESTAMP);
                    break;
                case COUNTER64:
                    callback = addHandler(responseIP, responseOID, COUNTER64);
                    break;
                case NETWORK_ADDRESS:
                    callback = addHandler(responseIP, responseOID, NETWORK_ADDRESS);
                    break;
                default:
                    break;
                }
                if (!callback)
                {
                    Serial.print(F("Matching callback not found for received SNMP response. Response OID: "));
                    Serial.print(responseOID);
                    Serial.print(F(" - From IP Address: "));
                    Serial.println(responseIP);
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
                    Serial.println(responseOID);
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
                case OID:
                {
#ifdef DEBUG
                    Serial.println("[DEBUG] Type: OID");
#endif
                    OIDType *value = new OIDType();
                    strncpy(((OIDCallback *)callback)->value, ((OIDType *)responseContainer)->_value, strlen(((OIDType *)responseContainer)->_value));
                    Serial.println(((OIDCallback *)callback)->value);
                    delete value;
                }
                case NETWORK_ADDRESS:
                {
#ifdef DEBUG
                    Serial.println("[DEBUG] Type: NETWORK_ADDRESS");
#endif

                    NetworkAddress *value = new NetworkAddress();
                    (((NetWorkAddressCallback *)callback)->value) = ((NetworkAddress *)responseContainer)->_value;
                    value->_value = (((NetWorkAddressCallback *)callback)->value);
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
                addHandlerToResult(callback);
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
ValueCallback *SNMPManager::addNextRequestHandler(IPAddress ip, const char *oid)
{
    // TODO: Whats the type in the request for the get next request?
    ValueCallback *callback = new ValueCallback(GetNextRequestPDU);
    callback->OID = (char *)malloc((sizeof(char) * strlen(oid)) + 1);
    strcpy(callback->OID, oid);
    callback->ip = ip;
    return callback;
}
ValueCallback *SNMPManager::addHandler(IPAddress ip, const char *oid, ASN_TYPE type)
{
    ValueCallback *callback;
    switch (type)
    {
    case INTEGER:
        callback = new IntegerCallback();
        ((IntegerCallback *)callback)->isFloat = false;
        break;
    case STRING:
        callback = new StringCallback();
        break;
    case COUNTER32:
        callback = new Counter32Callback();
        break;
    case GAUGE32:
        callback = new Gauge32Callback();
        break;
    case TIMESTAMP:
        callback = new TimestampCallback();
        break;
    case COUNTER64:
        callback = new Counter64Callback();
        break;
    case OID:
        callback = new OIDCallback();
        break;
    case NETWORK_ADDRESS:
        callback = new NetWorkAddressCallback();
        break;
    default:
        break;
    }
    callback->OID = (char *)malloc((sizeof(char) * strlen(oid)) + 1);
    strcpy(callback->OID, oid);
    callback->ip = ip;
    return callback;
}

void SNMPManager::addHandlerToResult(ValueCallback *callback)
{
    resultsCursor = results;
    if (resultsCursor->value)
    {
        while (resultsCursor->next != 0)
        {
            resultsCursor = resultsCursor->next;
        }
        resultsCursor->next = new ValueCallbacks();
        resultsCursor = resultsCursor->next;
        resultsCursor->value = callback;
        resultsCursor->next = 0;
    }
    else
    {
        results->value = callback;
    }
}

#endif
