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
        SNMP(const char *community, short snmpVersion) : _community(community), _version(snmpVersion){
        }
        //TODO: throw exception when get no response from oid
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
            while(!_snmp.loop()){
                if((millis()-time)/1000 >= timeout)
                    return "";
            }
            return String(valueResponse);
        }
        int getInt(const char *oid, short int timeout, IPAddress targetIp)
        {
            int time = millis();
            int value;
            _callback = _snmp.addIntegerHandler(targetIp, oid, &value);
            send(targetIp);
            while(!_snmp.loop()){
                if((millis()-time)/1000 >= timeout)
                    return 0;
            }
            return value;
        }
        
        float getFloat(const char *oid, short int timeout, IPAddress targetIp){
            int time=millis();
            float value;
            _callback = _snmp.addFloatHandler(targetIp, oid, &value);
            send(targetIp);
            while(!_snmp.loop()){
                if((millis()-time)/1000 >= timeout)
                    return 0.0;
            }
            return value;
        }
        //response on ticks
        uint32_t getTimestamp(const char *oid, short int timeout, IPAddress targetIp){
            int time = millis();
            uint32_t value;
            _callback = _snmp.addTimestampHandler(targetIp, oid, &value);
            send(targetIp);
            while(!_snmp.loop()){
                if((millis()-time)/1000 >= timeout)
                    return 0.0;
            }
            return value;
        }
        OIDCallback getOid(const char *oid, short int timeout, IPAddress targetIp);
        uint64_t getCounter64(const char *oid, short int timeout, IPAddress targetIp);
        uint32_t getCounter43(const char *oid, short int timeout, IPAddress targetIp);
        uint32_t getGuage(const char *oid, short int timeout, IPAddress targetIp);
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