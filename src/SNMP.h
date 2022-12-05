#include <Arduino.h>
#include <WiFi.h>       
#include <Arduino_SNMP_Manager.h>

class SNMP
{
    
    const char *_community;
    short _version;    
    IPAddress _targetIp;
    IPAddress _agentIp;
    SNMPManager _snmp;
    WiFiUDP _udp;
    ValueCallback *_callback;
    public:
        SNMP(const char *community, short snmpVersion) : _community(community), _version(snmpVersion){}
        String getString(const char *oid, short int timeout, IPAddress targetIp)
        {
            //test
            char value[50];
            char *valueResponse = value;
            _snmp = SNMPManager(_community);
             SNMPGet _snmpRequest = SNMPGet(_community, _version);
            _snmp.setUDP(&_udp);
            _snmp.begin();
            _callback = _snmp.addStringHandler(targetIp, oid, &valueResponse);
            _snmpRequest.addOIDPointer(_callback);
            _snmpRequest.setIP(WiFi.localIP());
            _snmpRequest.setUDP(&_udp);
            _snmpRequest.setRequestID(rand() % 5555);
            _snmpRequest.sendTo(targetIp);
            _snmpRequest.clearOIDList();
            _snmp.loop();
            while(!_snmp.loop()){
                Serial.println("Null, delaying 1 sec");
                delay(1000);
            }
            return String(valueResponse);
        }

        bool loop(){
            return _snmp.loop();
        }
        
};