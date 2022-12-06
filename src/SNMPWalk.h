#ifndef SNMPWalk_h
#define SNMPWalk_h
#include <Arduino_SNMP_Manager.h>

class SNMPWalk
{

    const char *_community;
    short _version;
    IPAddress _targetIp;
    IPAddress _agentIp;
    SNMPManager _snmp;
    WiFiUDP _udp;
    ValueCallback *_callback;

public:
    SNMPWalk(const char *community, short snmpVersion) : _community(community), _version(snmpVersion)
    {
    }
}
#endif