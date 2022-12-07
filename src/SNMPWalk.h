#ifndef SNMPWalk_h
#define SNMPWalk_h
#include <Arduino_SNMP_Manager.h>
#include <WiFiUdp.h>
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
    ValueCallback *walk(IPAddress ip)
    {
        while (true)
        {
            manager = SNMPManager(_community, _version);
            manager.setUDP(&_udp);
            ValueCallback *callback = manager.getNextRequest(ip, startOID);

            Serial.println(callback->type);
            return callback;
        }
    }
};

#endif

/*switch (callback->type)
        {
        case STRING:
        {
            Serial.printf("Type: %d, value: %s, OID: %s\n", ((StringCallback *)callback)->type, ((StringCallback *)callback)->value, ((StringCallback *)callback)->OID);
        }
        break;
        case INTEGER:
        {
            Serial.printf("Type: %d, value: %d, OID: %s\n", ((IntegerCallback *)callback)->type, ((IntegerCallback *)callback)->value, ((IntegerCallback *)callback)->OID);
        }
        break;
        case COUNTER32:
        {
            Serial.printf("Type: %d, value: %d, OID: %s\n", ((Counter32Callback *)callback)->type, ((Counter32Callback *)callback)->value, ((Counter32Callback *)callback)->OID);
        }
        break;
        case COUNTER64:
        {
            Serial.printf("Type: %d, value: %d, OID: %s\n", ((Counter64Callback *)callback)->type, ((Counter64Callback *)callback)->value, ((Counter64Callback *)callback)->OID);
        }
        break;
        case GAUGE32:
        {
            Serial.printf("Type: %d, value: %d, OID: %s\n", ((Gauge32Callback *)callback)->type, ((Gauge32Callback *)callback)->value, ((Gauge32Callback *)callback)->OID);
        }
        break;
        case TIMESTAMP:
        {
            Serial.printf("Type: %d, value: %d, OID: %s\n", ((TimestampCallback *)callback)->type, ((TimestampCallback *)callback)->value, ((TimestampCallback *)callback)->OID);
        }
        break;
        case OID:
        {
            Serial.printf("Type: %d, value: %s, OID: %s\n", ((OIDCallback *)callback)->type, ((OIDCallback *)callback)->value, ((OIDCallback *)callback)->OID);
        }
        case NETWORK_ADDRESS:
        {
            Serial.printf("Type: %d, value: %s, OID: %s\n", ((NetWorkAddressCallback *)callback)->type, ((NetWorkAddressCallback *)callback)->value.toString(), ((NetWorkAddressCallback *)callback)->OID);
        }
        break;
        default:
            break;
        }*/
/*while (true)
{
    ValueCallbacks *resultsCursor = manager.results;
    if (callback)
    {
        while (resultsCursor->next != 0)
        {
            resultsCursor = resultsCursor->next;
        }
        callback = manager.getNextRequest(ip, callback->OID);
        snmpRequest.addOIDPointer(callback);
        snmpRequest.setRequestID(rand() % 5555);
        snmpRequest.sendTo(ip);
        snmpRequest.clearOIDList();
        int time = millis();
        int timeout = 2;
        while (!manager.loop())
        {
            if ((millis() - time) / 1000 >= timeout)
                Serial.println("ainda aqui, tentando novamente");
            break;
        }
    }
    if (results.containsKey(callback->OID))
    {
        Serial.println("End of MIB");
    }
    switch (callback->type)
    {
    case STRING:
    {
        Serial.printf("Type: %d, value: %s, OID: %s\n", ((StringCallback *)callback)->type, ((StringCallback *)callback)->value, ((StringCallback *)callback)->OID);

    }
    break;
    case INTEGER:
    {
        Serial.printf("Type: %d, value: %d, OID: %s\n", ((IntegerCallback *)callback)->type, ((IntegerCallback *)callback)->value, ((IntegerCallback *)callback)->OID);

    }
    break;
    case COUNTER32:
    {
        Serial.printf("Type: %d, value: %d, OID: %s\n", ((Counter32Callback *)callback)->type, ((Counter32Callback *)callback)->value, ((Counter32Callback *)callback)->OID);

    }
    break;
    case COUNTER64:
    {
        Serial.printf("Type: %d, value: %d, OID: %s\n", ((Counter64Callback *)callback)->type, ((Counter64Callback *)callback)->value, ((Counter64Callback *)callback)->OID);

    }
    break;
    case GAUGE32:
    {
        Serial.printf("Type: %d, value: %d, OID: %s\n", ((Gauge32Callback *)callback)->type, ((Gauge32Callback *)callback)->value, ((Gauge32Callback *)callback)->OID);

    }
    break;
    case TIMESTAMP:
    {
        Serial.printf("Type: %d, value: %d, OID: %s\n", ((TimestampCallback *)callback)->type, ((TimestampCallback *)callback)->value, ((TimestampCallback *)callback)->OID);

    }
    break;
    case OID:
    {
        Serial.printf("Type: %d, value: %s, OID: %s\n", ((OIDCallback *)callback)->type, ((OIDCallback *)callback)->value, ((OIDCallback *)callback)->OID);

    }
    case NETWORK_ADDRESS:
    {
        Serial.printf("Type: %d, value: %s, OID: %s\n", ((NetWorkAddressCallback *)callback)->type, ((NetWorkAddressCallback *)callback)->value.toString(), ((NetWorkAddressCallback *)callback)->OID);

    }
    break;
    default:
        break;
    }

    Serial.println(esp_get_free_heap_size());
}*/