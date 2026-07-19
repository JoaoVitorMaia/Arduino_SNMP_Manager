#ifndef SNMP_MANAGER_H
#define SNMP_MANAGER_H

#include <Arduino.h>
#include <IPAddress.h>
#include <Udp.h>
#include <cstring>

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

typedef struct ValueCallbackList
{
    ~ValueCallbackList()
    {
        delete next;
    }
    ValueCallback *value;
    struct ValueCallbackList *next = 0;
} ValueCallbacks;

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
    
   
    void setUDP(UDP* udp);
    bool begin();
    bool loop();
    
    ValueCallback* addStringHandler(IPAddress ip, const char* oid, char** value);
    ValueCallback* addIntegerHandler(IPAddress ip, const char* oid, int* value);
    ValueCallback* addFloatHandler(IPAddress ip, const char* oid, float* value);
    ValueCallback* addTimestampHandler(IPAddress ip, const char* oid, uint32_t* value);
    ValueCallback* addOIDHandler(IPAddress ip, const char* oid, char* value);
    ValueCallback* addCounter64Handler(IPAddress ip, const char* oid, uint64_t* value);
    ValueCallback* addCounter32Handler(IPAddress ip, const char* oid, uint32_t* value);
    ValueCallback* addGaugeHandler(IPAddress ip, const char* oid, uint32_t* value);
    bool removeHandler(ValueCallback* callback);
    void addHandler(ValueCallback* callback);


    
    bool testParsePacket(String testPacket);
    
    const char* _community;
    UDP* _udp = nullptr;
    char OIDBuf[MAX_OID_LENGTH];

private:
    unsigned char _packetBuffer[SNMP_PACKET_LENGTH * 3];
    ValueCallbackList* callbacks = nullptr;
    
    ValueCallback* findCallback(const IPAddress& ip, const char* oid);
    bool inline receivePacket(int length);
    bool parsePacket();
    void printPacket(int len);
};

#endif // SNMP_MANAGER_H