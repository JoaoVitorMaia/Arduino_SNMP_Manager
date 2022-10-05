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
            int time = millis();
            char value[50];
            char *valueResponse = value;
            _snmp = SNMPManager(_community);
            _snmp.setUDP(&_udp);
            _snmp.begin();
            _callback = _snmp.addStringHandler(targetIp, oid, &valueResponse);
            send(targetIp);
            _snmp.loop();
            while(!_snmp.loop()){
                if((millis()-time)/1000 >= timeout)
                    return "";
            }
            return String(valueResponse);
        }

        bool loop(){
            return _snmp.loop();
        }
    private:
        void send(IPAddress targetIp)
        {
            SNMPGet _snmpRequest = SNMPGet(_community, _version);
            _snmpRequest.addOIDPointer(_callback);
            _snmpRequest.setIP(WiFi.localIP());
            _snmpRequest.setUDP(&_udp);
            _snmpRequest.setRequestID(rand() % 5555);
            _snmpRequest.sendTo(targetIp);
            _snmpRequest.clearOIDList();
        }
};