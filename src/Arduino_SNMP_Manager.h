#ifndef SNMP_MANAGER_H
#define SNMP_MANAGER_H

#include <Arduino.h>
#include <IPAddress.h>
#include <Udp.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <cstring>
#include <algorithm>

// Configuration macros
#ifndef UDP_TX_PACKET_MAX_SIZE
#define UDP_TX_PACKET_MAX_SIZE 484
#endif

#ifndef SNMP_PACKET_LENGTH
#if defined(ESP32)
#define SNMP_PACKET_LENGTH 1500 // This will limit the size of packets which can be handled.
#else
#define SNMP_PACKET_LENGTH 512 // This value may need to be made smaller for lower memory devices.
#endif
#endif

#ifndef MAX_OID_LENGTH
#define MAX_OID_LENGTH 128
#endif

#define MIN(X, Y) ((X < Y) ? X : Y)

// Forward declarations
#include "BER.h"
#include "VarBinds.h"


//=============================================================================
// ValueCallback Base Class
//=============================================================================
class ValueCallback {
public:
    ValueCallback(ASN_TYPE atype) : type(atype) {}
    virtual ~ValueCallback() {
        if (OID) {
            free(OID);
            OID = nullptr;
        }
    }
    
    IPAddress ip;
    char* OID = nullptr;
    ASN_TYPE type;
    bool overwritePrefix = false;
    bool received = false;
    ASNError error = ASNError::None;
};

//=============================================================================
// Specialized Callback Classes
//=============================================================================
class IntegerCallback : public ValueCallback {
public:
    IntegerCallback() : ValueCallback(INTEGER) {}
    int* value = nullptr;
    bool isFloat = false;
};

class TimestampCallback : public ValueCallback {
public:
    TimestampCallback() : ValueCallback(TIMESTAMP) {}
    uint32_t* value = nullptr;
};

class StringCallback : public ValueCallback {
public:
    StringCallback() : ValueCallback(STRING) {}
    char** value = nullptr;
};

class OIDCallback : public ValueCallback {
public:
    OIDCallback() : ValueCallback(ASN_TYPE::OID) {}
    char* value = nullptr;
};

class Counter32Callback : public ValueCallback {
public:
    Counter32Callback() : ValueCallback(ASN_TYPE::COUNTER32) {}
    uint32_t* value = nullptr;
};

class Gauge32Callback : public ValueCallback {
public:
    Gauge32Callback() : ValueCallback(ASN_TYPE::GAUGE32) {}
    uint32_t* value = nullptr;
};

class Counter64Callback : public ValueCallback {
public:
    Counter64Callback() : ValueCallback(ASN_TYPE::COUNTER64) {}
    uint64_t* value = nullptr;
};

//=============================================================================
// SNMPManager Class
//=============================================================================
class SNMPManager {
public:
    // Constructors
    SNMPManager() : _community("public") {}
    explicit SNMPManager(const char* community) : _community(community) {}
    
    // Destructor
    ~SNMPManager() {
        // Clean up all callbacks
        for (auto* cb : callbacks) {
            delete cb;
        }
        callbacks.clear();
    }
    
    // Setup methods
    void setUDP(UDP* udp);
    bool begin();
    bool loop();
    
    // Handler management
    ValueCallback* addStringHandler(IPAddress ip, const char* oid, char** value);
    ValueCallback* addIntegerHandler(IPAddress ip, const char* oid, int* value);
    ValueCallback* addFloatHandler(IPAddress ip, const char* oid, float* value);
    ValueCallback* addTimestampHandler(IPAddress ip, const char* oid, uint32_t* value);
    ValueCallback* addOIDHandler(IPAddress ip, const char* oid, char* value);
    ValueCallback* addCounter64Handler(IPAddress ip, const char* oid, uint64_t* value);
    ValueCallback* addCounter32Handler(IPAddress ip, const char* oid, uint32_t* value);
    ValueCallback* addGaugeHandler(IPAddress ip, const char* oid, uint32_t* value);
    bool removeHandler(ValueCallback* callback);
    
    std::vector<ASNError> request(IPAddress localIP, std::vector<ValueCallback*> callbacks, short requestID, const IPAddress &destIP, uint32_t communityVersion, uint32_t timeout, const IPAddress &localIp);
    
    ASNError getString(const IPAddress& ip, const char* oid, char** response,uint32_t timeout, uint32_t communityVersion, const IPAddress &localIP);
    ASNError getInteger(const IPAddress& ip, const char* oid, int* response, uint32_t timeout, uint32_t communityVersion, const IPAddress &localIP);
    ASNError getFloat(const IPAddress& ip, const char* oid, float* response,uint32_t timeout, uint32_t communityVersion, const IPAddress &localIP);
    ASNError getTimestamp(const IPAddress& ip, const char* oid, uint32_t* response,   uint32_t timeout, uint32_t communityVersion, const IPAddress &localIP);
    ASNError getOid(const IPAddress& ip, const char* oid, char* response,uint32_t timeout, uint32_t communityVersion, const IPAddress &localIP);
    ASNError getCounter32(const IPAddress& ip, const char* oid, uint32_t* response,   uint32_t timeout, uint32_t communityVersion, const IPAddress &localIP);
    ASNError getCounter64(const IPAddress& ip, const char* oid, uint64_t* response,   uint32_t timeout, uint32_t communityVersion, const IPAddress &localIP);
    ASNError getGauge(const IPAddress& ip, const char* oid, uint32_t* response,uint32_t timeout, uint32_t communityVersion, const IPAddress &localIP);
    
    bool testParsePacket(String testPacket);
    
    const char* _community;
    UDP* _udp = nullptr;
    char OIDBuf[MAX_OID_LENGTH];

private:
    unsigned char _packetBuffer[SNMP_PACKET_LENGTH * 3];
    std::vector<ValueCallback*> callbacks;
    
    // Internal methods
    ValueCallback* findCallback(const IPAddress& ip, const char* oid);
    void addHandler(ValueCallback* callback);
    bool inline receivePacket(int length);
    bool parsePacket();
    void printPacket(int len);
};

#endif // SNMP_MANAGER_H