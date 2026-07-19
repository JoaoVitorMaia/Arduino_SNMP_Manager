#ifndef SNMP_H
#define SNMP_H

#include "Arduino_SNMP_Manager.h"
#include "SNMPGet.h"
#include "WiFi.h"
#include <vector>


class SNMP{
    const char * _community;
    const uint8_t _snmpVersion;
    WiFiUDP *_udp = nullptr;
    public:
        SNMP(const char * community, const uint8_t snmpVersion, WiFiUDP *udp) : _community(community), _snmpVersion(snmpVersion), _udp(udp){
            snmpManager = new SNMPManager(community);
            snmpManager->setUDP(_udp);
            snmpManager->begin();
        }
        SNMPManager *snmpManager = nullptr;
        ASNError getString(const IPAddress& ip, const char* oid, char** response,uint32_t timeout, uint32_t communityVersion, const IPAddress &localIP);
        ASNError getInteger(const IPAddress& ip, const char* oid, int* response, uint32_t timeout, uint32_t communityVersion, const IPAddress &localIP);
        ASNError getFloat(const IPAddress& ip, const char* oid, float* response,uint32_t timeout, uint32_t communityVersion, const IPAddress &localIP);
        ASNError getTimestamp(const IPAddress& ip, const char* oid, uint32_t* response,   uint32_t timeout, uint32_t communityVersion, const IPAddress &localIP);
        ASNError getOid(const IPAddress& ip, const char* oid, char* response,uint32_t timeout, uint32_t communityVersion, const IPAddress &localIP);
        ASNError getCounter32(const IPAddress& ip, const char* oid, uint32_t* response,   uint32_t timeout, uint32_t communityVersion, const IPAddress &localIP);
        ASNError getCounter64(const IPAddress& ip, const char* oid, uint64_t* response,   uint32_t timeout, uint32_t communityVersion, const IPAddress &localIP);
        ASNError getGauge(const IPAddress& ip, const char* oid, uint32_t* response,uint32_t timeout, uint32_t communityVersion, const IPAddress &localIP);
        std::vector<ASNError> request(IPAddress localIP, std::vector<ValueCallback*> callbacks, short requestID, const IPAddress &destIP, uint32_t communityVersion, uint32_t timeout, const IPAddress &localIp);
};

#endif