#ifndef ValueCallback_h
#define ValueCallback_h

class ValueCallback
{
public:
    ValueCallback(ASN_TYPE atype) : type(atype){};
    IPAddress ip;
    char *OID;
    ASN_TYPE type;
    bool overwritePrefix = false;
};

class NextRequestOID : public ValueCallback
{
    public:
        NextRequestOID() : ValueCallback(ASN_TYPE::OID){};
        ValueCallback **value;
};

class IntegerCallback : public ValueCallback
{
public:
    IntegerCallback() : ValueCallback(INTEGER){};
    int *value;
    bool isFloat = false;
};

class TimestampCallback : public ValueCallback
{
public:
    TimestampCallback() : ValueCallback(TIMESTAMP){};
    uint32_t *value;
};

class StringCallback : public ValueCallback
{
public:
    StringCallback() : ValueCallback(STRING){};
    char **value;
};

class OIDCallback : public ValueCallback
{
public:
    OIDCallback() : ValueCallback(ASN_TYPE::OID){};
    char *value;
};

class Counter32Callback : public ValueCallback
{
public:
    Counter32Callback() : ValueCallback(ASN_TYPE::COUNTER32){};
    uint32_t *value;
};

class Guage32Callback : public ValueCallback
{
public:
    Guage32Callback() : ValueCallback(ASN_TYPE::GUAGE32){};
    uint32_t *value;
};

class Counter64Callback : public ValueCallback
{
public:
    Counter64Callback() : ValueCallback(ASN_TYPE::COUNTER64){};
    uint64_t *value;
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
