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
    ValueCallback *_callback;

public:
    SNMP(const char *community, short snmpVersion) : _community(community), _version(snmpVersion)
    {
        _snmp = SNMPManager(community, snmpVersion);
    }
    void setUDP(UDP *udp)
    {
        _snmp.setUDP(udp);
    }
    // TODO: throw exception when get no response from oid
    String getString(const char *oid, short int timeout, IPAddress targetIp)
    {
        ValueCallback *callback = _snmp.addStringHandler(targetIp, oid, timeout);
        if (!callback)
            return "";
        String value = ((StringCallback *)callback)->value;
        _snmp.deleteCallback(callback->ip, callback->OID);
        return value;
    }
    int getInteger(const char *oid, short int timeout, IPAddress targetIp)
    {
        ValueCallback *callback = _snmp.addIntegerHandler(targetIp, oid, timeout);
        if (!callback)
            return NULL;
        return ((IntegerCallback *)callback)->value;
    }

    float getFloat(const char *oid, short int timeout, IPAddress targetIp)
    {
        ValueCallback *callback = _snmp.addFloatHandler(targetIp, oid, timeout);
        if (!callback)
            return NULL;
        return ((IntegerCallback *)callback)->value;
    }
    uint32_t getTimestamp(const char *oid, short int timeout, IPAddress targetIp)
    {
        ValueCallback *callback = _snmp.addTimestampHandler(targetIp, oid, timeout);
        if (!callback)
            return NULL;
        return ((TimestampCallback *)callback)->value;
    }
    uint64_t getCounter64(const char *oid, short int timeout, IPAddress targetIp)
    {
        ValueCallback *callback = _snmp.addCounter64Handler(targetIp, oid, timeout);
        if (!callback)
            return NULL;
        return ((Counter64Callback *)callback)->value;
    }
    uint32_t getCounter32(const char *oid, short int timeout, IPAddress targetIp)
    {
        ValueCallback *callback = _snmp.addCounter32Handler(targetIp, oid, timeout);
        if (!callback)
            return NULL;
        uint32_t value = ((Counter32Callback *)callback)->value;
        _snmp.deleteCallback(callback->ip, callback->OID);
        return value;
    }
    uint32_t getGauge(const char *oid, short int timeout, IPAddress targetIp)
    {
        ValueCallback *callback = _snmp.addGaugeHandler(targetIp, oid, timeout);
        if (!callback)
            return NULL;
        return ((Gauge32Callback *)callback)->value;
    }
};

#endif