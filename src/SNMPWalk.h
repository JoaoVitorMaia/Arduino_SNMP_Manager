#ifndef SNMPWalk_h
#define SNMPWalk_h
#include <Arduino_SNMP_Manager.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <Arduino.h>
class SNMPWalk
{

    const char *_community;
    short _version;
    WiFiUDP _udp;
    SNMPManager manager;
    SNMPGetNext snmpRequest;
    const char *startOID = ".1";

public:
    SNMPWalk(const char *community, short snmpVersion) : _community(community), _version(snmpVersion)
    {
    }
    DynamicJsonDocument walk(IPAddress ip)
    {
        manager = SNMPManager(_community);
        snmpRequest = SNMPGetNext(_community, _version);
        manager.setUDP(&_udp);
        manager.begin();
        ValueCallback *callback = manager.addNextRequestHandler(ip, startOID);
        snmpRequest.addOIDPointer(callback);
        snmpRequest.setIP(WiFi.localIP());
        snmpRequest.setUDP(&_udp);
        snmpRequest.setRequestID(rand() % 5555);
        snmpRequest.sendTo(ip);
        snmpRequest.clearOIDList();
        while (!manager.loop())
        {
        }
        DynamicJsonDocument results(2048);
        while (true)
        {
            delay(100);
            ValueCallbacks *resultsCursor = manager.results;
            if (resultsCursor->value)
            {

                while (resultsCursor->next != 0)
                {
                    resultsCursor = resultsCursor->next;
                }
                callback = manager.addNextRequestHandler(ip, resultsCursor->value->OID);
                snmpRequest.addOIDPointer(callback);
                snmpRequest.setRequestID(rand() % 5555);
                snmpRequest.sendTo(ip);
                snmpRequest.clearOIDList();
                int time = millis();
                int timeout = 2;
                while (!manager.loop())
                {
                    if ((millis() - time) / 1000 >= timeout)
                        Serial.println("ainda aqui");
                }
            }
            if (results.containsKey(resultsCursor->value->OID))
            {
                Serial.println("End of MIB");
                return results;
            }
            switch (resultsCursor->value->type)
            {
            case STRING:
            {
                Serial.printf("Type: %d, value: %s, OID: %s\n", ((StringCallback *)resultsCursor->value)->type, ((StringCallback *)resultsCursor->value)->value, ((StringCallback *)callback)->OID);

                /*JsonObject oid = results.createNestedObject(((StringCallback *)callback)->OID);
                oid["type"] = ((StringCallback *)resultsCursor->value)->type;
                oid["value"] = ((StringCallback *)resultsCursor->value)->value;*/
            }
            break;
            case INTEGER:
            {
                Serial.printf("Type: %d, value: %d, OID: %s\n", ((IntegerCallback *)resultsCursor->value)->type, ((IntegerCallback *)resultsCursor->value)->value, ((IntegerCallback *)callback)->OID);
                /*JsonObject oid = results.createNestedObject(((IntegerCallback *)callback)->OID);
                oid["type"] = ((IntegerCallback *)resultsCursor->value)->type;
                oid["value"] = ((IntegerCallback *)resultsCursor->value)->value;*/
            }
            break;
            case COUNTER32:
            {
                Serial.printf("Type: %d, value: %d, OID: %s\n", ((Counter32Callback *)resultsCursor->value)->type, ((Counter32Callback *)resultsCursor->value)->value, ((Counter32Callback *)callback)->OID);
                /*JsonObject oid = results.createNestedObject(((Counter32Callback *)callback)->OID);
                oid["type"] = ((Counter32Callback *)resultsCursor->value)->type;
                oid["value"] = ((Counter32Callback *)resultsCursor->value)->value;*/
            }
            break;
            case COUNTER64:
            {
                Serial.printf("Type: %d, value: %d, OID: %s\n", ((Counter64Callback *)resultsCursor->value)->type, ((Counter64Callback *)resultsCursor->value)->value, ((Counter64Callback *)callback)->OID);

                /*JsonObject oid = results.createNestedObject(((Counter64Callback *)callback)->OID);
                oid["type"] = ((Counter64Callback *)resultsCursor->value)->type;
                oid["value"] = ((Counter64Callback *)resultsCursor->value)->value;*/
            }
            break;
            case GAUGE32:
            {
                Serial.printf("Type: %d, value: %d, OID: %s\n", ((Gauge32Callback *)resultsCursor->value)->type, ((Gauge32Callback *)resultsCursor->value)->value, ((Gauge32Callback *)callback)->OID);

                /*JsonObject oid = results.createNestedObject(((Gauge32Callback *)callback)->OID);
                oid["type"] = ((Gauge32Callback *)resultsCursor->value)->type;
                oid["value"] = ((Gauge32Callback *)resultsCursor->value)->value;*/
            }
            break;
            case TIMESTAMP:
            {
                Serial.printf("Type: %d, value: %d, OID: %s\n", ((TimestampCallback *)resultsCursor->value)->type, ((TimestampCallback *)resultsCursor->value)->value, ((TimestampCallback *)callback)->OID);

                /*JsonObject oid = results.createNestedObject(((TimestampCallback *)callback)->OID);
                oid["type"] = ((TimestampCallback *)resultsCursor->value)->type;
                oid["value"] = ((TimestampCallback *)resultsCursor->value)->value;*/
            }
            break;
            case OID:
            {
                Serial.printf("Type: %d, value: %s, OID: %s\n", ((OIDCallback *)resultsCursor->value)->type, ((OIDCallback *)resultsCursor->value)->value, ((OIDCallback *)callback)->OID);
                /*JsonObject oid = results.createNestedObject(((OIDCallback *)callback)->OID);
                oid["type"] = ((OIDCallback *)resultsCursor->value)->type;
                oid["value"] = ((OIDCallback *)resultsCursor->value)->value;*/
            }
            case NETWORK_ADDRESS:
            {
                Serial.printf("Type: %d, value: %s, OID: %s\n", ((NetWorkAddressCallback *)resultsCursor->value)->type, ((NetWorkAddressCallback *)resultsCursor->value)->value.toString(), ((NetWorkAddressCallback *)callback)->OID);
                /*JsonObject oid = results.createNestedObject(((NetWorkAddressCallback *)callback)->OID);
                oid["type"] = ((NetWorkAddressCallback *)resultsCursor->value)->type;
                oid["value"] = ((NetWorkAddressCallback *)resultsCursor->value)->value.toString();*/
            }
            break;
            default:
                break;
            }
            Serial.println(ESP.getFreeHeap());
        }
    }
};
#endif
