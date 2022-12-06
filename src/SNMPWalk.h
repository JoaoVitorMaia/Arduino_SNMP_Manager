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
    const char *startOID = ".1";

public:
    SNMPWalk(const char *community, short snmpVersion) : _community(community), _version(snmpVersion)
    {
    }
    void walk(IPAddress ip)
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
        if (manager.results->value)
        {
            switch (manager.results->value->type)
            {
            case STRING:

                Serial.println(((StringCallback *)manager.results->value)->value);
                break;

            default:
            Serial.print(manager.callbacksCursor->value->type);
                break;
            }
        }else{
            Serial.println("no value");
        }
        Serial.println(((StringCallback *)manager.results->value)->value);
    }
};
#endif