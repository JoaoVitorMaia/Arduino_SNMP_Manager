#ifndef SNMP_h
#define SNMP_h

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
    SNMP(const char *community, short snmpVersion) : _community(community), _version(snmpVersion)
    {
    }
    // TODO: throw exception when get no response from oid
    void getString(const char *oid, short int timeout, IPAddress targetIp, char *value)
    {
        this->config(); // throw corrupted error when put in the constructor...
        int time = millis();
        char *valueResponse = value;
        _callback = _snmp.addStringHandler(targetIp, oid, &valueResponse);
        this->send(targetIp);
        this->waitForResponse(timeout, time);
    }
    int getInteger(const char *oid, short int timeout, IPAddress targetIp)
    {
        this->config();
        int time = millis();
        int value;
        _callback = _snmp.addIntegerHandler(targetIp, oid, &value);
        this->send(targetIp);
        return this->waitForResponse(timeout, time) ? value : 0;
    }

    float getFloat(const char *oid, short int timeout, IPAddress targetIp)
    {
        this->config();
        int time = millis();
        float value;
        _callback = _snmp.addFloatHandler(targetIp, oid, &value);
        this->send(targetIp);
        return this->waitForResponse(timeout, time) ? value : 0.0;
    }
    // response on ticks
    uint32_t getTimestamp(const char *oid, short int timeout, IPAddress targetIp)
    {
        this->config();
        int time = millis();
        uint32_t value;
        _callback = _snmp.addTimestampHandler(targetIp, oid, &value);
        this->send(targetIp);
        return this->waitForResponse(timeout, time) ? value : 0;
    }
    uint64_t getCounter64(const char *oid, short int timeout, IPAddress targetIp)
    {
        this->config();
        int time = millis();
        uint64_t value;
        _callback = _snmp.addCounter64Handler(targetIp, oid, &value);
        this->send(targetIp);
        return this->waitForResponse(timeout, time) ? value : 0;

    }
    uint32_t getCounter32(const char *oid, short int timeout, IPAddress targetIp)
    {
        this->config();
        int time = millis();
        uint32_t value;
        _callback = _snmp.addCounter32Handler(targetIp, oid, &value);
        this->send(targetIp);
        return this->waitForResponse(timeout, time) ? value : 0;

    }
    uint32_t getGauge(const char *oid, short int timeout, IPAddress targetIp)
    {
        this->config();
        int time = millis();
        uint32_t value;
        _callback = _snmp.addGaugeHandler(targetIp, oid, &value);
        this->send(targetIp);
        return this->waitForResponse(timeout, time) ? value : 0;

    }

    bool waitForResponse(int timeout, int time){
        while (!_snmp.loop())
        {
            if ((millis() - time) / 1000 >= timeout)
                return false;
        }
        return true;
    }

private:
    void send(IPAddress targetIp, ASN_TYPE_WITH_VALUE type = ASN_TYPE_WITH_VALUE::GetRequestPDU)
    {
        SNMPGet _snmpRequest = SNMPGet(_community, _version);
        _snmpRequest.addOIDPointer(_callback);
        _snmpRequest.setIP(WiFi.localIP());
        _snmpRequest.setUDP(&_udp);
        _snmpRequest.setRequestID(rand() % 5555);
        _snmpRequest.sendTo(targetIp, type);
        _snmpRequest.clearOIDList();
    }
    void config()
    {
        _snmp = SNMPManager(_community);
        _snmp.setUDP(&_udp);
        _snmp.begin();
    }
};
#endif