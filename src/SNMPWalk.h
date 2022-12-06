#ifndef SNMPWalk_h
#define SNMPWalk_h
#include <Arduino_SNMP_Manager.h>
#include <WiFiUdp.h>

class SNMPWalk
{

    const char *_community;
    short _version;
    WiFiUDP _udp;
    SNMPManager manager;
    SNMPGetNext snmpRequest;
    const char *startOID = ".1.3.6.1.2.1.1.2.0";

public:
    SNMPWalk(const char *community, short snmpVersion) : _community(community), _version(snmpVersion)
    {
    }
    ValueCallback *walk(IPAddress ip)
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
        while (true)
        {
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
                while (!manager.loop())
                {
                }
            }
        }
        /*ValueCallbacks *resultsCursor = manager.results;
        if (resultsCursor->value)
        {
            while (resultsCursor->next != 0)
            {
                switch (resultsCursor->value->type)
                {
                case STRING:
                    Serial.println(((StringCallback *)manager.results->value)->value);
                    break;
                case INTEGER:
                    callback = addHandler(responseIP, responseOID, INTEGER);
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
                default:
                    break;
                default:
                    Serial.print(manager.callbacksCursor->value->type);
                    break;
                }
            }
        }*/
    }
};
#endif