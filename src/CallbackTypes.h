#ifndef CallbackTypes_h
#define CallbackTypes_h
#ifndef MAX_OID_LENGTH
#define MAX_OID_LENGTH 128
#endif
#ifndef SNMP_OCTETSTRING_MAX_LENGTH
#define SNMP_OCTETSTRING_MAX_LENGTH 1024
#endif
class ValueCallback
{
public:
    ValueCallback(ASN_TYPE atype) : type(atype){};
    IPAddress ip;
    char *OID;
    ASN_TYPE type;
    bool overwritePrefix = false;
};

class IntegerCallback : public ValueCallback
{
public:
    IntegerCallback() : ValueCallback(INTEGER){};
    int value;
    bool isFloat = false;
};

class TimestampCallback : public ValueCallback
{
public:
    TimestampCallback() : ValueCallback(TIMESTAMP){};
    uint32_t value;
};

class StringCallback : public ValueCallback
{
public:
    StringCallback() : ValueCallback(STRING){};
    char value[SNMP_OCTETSTRING_MAX_LENGTH] = {0};
};

class OIDCallback : public ValueCallback
{
public:
    OIDCallback() : ValueCallback(ASN_TYPE::OID){};
    char value[MAX_OID_LENGTH];
};

class Counter32Callback : public ValueCallback
{
public:
    Counter32Callback() : ValueCallback(ASN_TYPE::COUNTER32){};
    uint32_t value;
};

class Gauge32Callback : public ValueCallback
{
public:
    Gauge32Callback() : ValueCallback(ASN_TYPE::GAUGE32){};
    uint32_t value;
};

class Counter64Callback : public ValueCallback
{
public:
    Counter64Callback() : ValueCallback(ASN_TYPE::COUNTER64){};
    uint64_t value;
};

class NetWorkAddressCallback : public ValueCallback{
    public:
    NetWorkAddressCallback() : ValueCallback(ASN_TYPE::NETWORK_ADDRESS){};
    IPAddress value;
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

#endif