#include "Arduino_SNMP_Manager.h"
#include "SNMPGet.h"
#include "SNMP.h"

std::vector<ASNError> SNMP::request(IPAddress localIP, std::vector<ValueCallback*> callbacks, short requestID, const IPAddress &destIP, uint32_t communityVersion, uint32_t timeout, const IPAddress &localIp){
    SNMPGet *request = new SNMPGet(_community, communityVersion);
    std::vector<ASNError> err;
    request->setIP(localIP); // IP of the listening MCU
    request->setUDP(_udp);
    
    for (ValueCallback* callback : callbacks) {
        request->addOIDPointer(callback);
    }
    request->setRequestID(requestID);
    request->sendTo(destIP);
    request->clearOIDList();
    uint32_t start = millis();
    bool allCallbackReceived = false;
    while ((millis() - start < (timeout*1000))) {
        snmpManager->loop();
        allCallbackReceived = true;
        for (ValueCallback* callback : callbacks) {
            if(!callback->received)
                allCallbackReceived = false;
                break;;
        }
        if(allCallbackReceived)
            break;

        vTaskDelay(1);
    }
    
    if (!allCallbackReceived){
#ifdef DEBUG
        Serial.printf("Timeout requesting multiple oids from %s\n", destIP.toString().c_str());
#endif
        err.push_back(ASNError::RequestTimedOut);
    }else{
        for (ValueCallback* callback : callbacks) {
            err.push_back(callback->error);
        }
    }
    for (ValueCallback* callback : callbacks) {
        snmpManager->removeHandler(callback);
    }
    delete request;
    return err;
}

ASNError SNMP::getString(const IPAddress &ip, const char *oid, char **response, uint32_t timeout, uint32_t communityVersion,  const IPAddress &localIp){
    return request(localIp, std::vector<ValueCallback*>{snmpManager->addStringHandler(ip, oid, response)}, rand() % 5555, ip, communityVersion, timeout, localIp)[0];
}

ASNError SNMP::getInteger(const IPAddress &ip, const char *oid, int *response,  uint32_t timeout, uint32_t communityVersion,  const IPAddress &localIp){
    return request(localIp, std::vector<ValueCallback*>{snmpManager->addIntegerHandler(ip, oid, response)}, rand() % 5555, ip, communityVersion, timeout, localIp)[0];
}
ASNError SNMP::getCounter32(const IPAddress &ip, const char *oid, uint32_t *response,  uint32_t timeout, uint32_t communityVersion,  const IPAddress &localIp){
    return request(localIp, std::vector<ValueCallback*>{snmpManager->addCounter32Handler(ip, oid, response)}, rand() % 5555, ip, communityVersion, timeout, localIp)[0];
}
ASNError SNMP::getCounter64(const IPAddress &ip, const char *oid, uint64_t *response,  uint32_t timeout, uint32_t communityVersion,  const IPAddress &localIp){
    return request(localIp, std::vector<ValueCallback*>{snmpManager->addCounter64Handler(ip, oid, response)}, rand() % 5555, ip, communityVersion, timeout, localIp)[0];
}
ASNError SNMP::getFloat(const IPAddress &ip, const char *oid, float *response,  uint32_t timeout, uint32_t communityVersion,  const IPAddress &localIp){
    return request(localIp, std::vector<ValueCallback*>{snmpManager->addFloatHandler(ip, oid, response)}, rand() % 5555, ip, communityVersion, timeout, localIp)[0];
}
ASNError SNMP::getTimestamp(const IPAddress &ip, const char *oid, uint32_t *response,  uint32_t timeout, uint32_t communityVersion,  const IPAddress &localIp){
    return request(localIp, std::vector<ValueCallback*>{snmpManager->addTimestampHandler(ip, oid, response)}, rand() % 5555, ip, communityVersion, timeout, localIp)[0];
}
ASNError SNMP::getOid(const IPAddress &ip, const char *oid, char *response, uint32_t timeout, uint32_t communityVersion,  const IPAddress &localIp){
    return request(localIp, std::vector<ValueCallback*>{snmpManager->addOIDHandler(ip, oid, response)}, rand() % 5555, ip, communityVersion, timeout, localIp)[0];
}
ASNError SNMP::getGauge(const IPAddress &ip, const char *oid, uint32_t *response,  uint32_t timeout, uint32_t communityVersion,  const IPAddress &localIp){
    return request(localIp, std::vector<ValueCallback*>{snmpManager->addGaugeHandler(ip, oid, response)}, rand() % 5555, ip, communityVersion, timeout, localIp)[0];
}