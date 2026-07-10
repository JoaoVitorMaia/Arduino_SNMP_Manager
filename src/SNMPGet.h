#ifndef SNMPGet_h
#define SNMPGet_h

enum SNMPExpect
{
	HEADER,
	SNMPVERSION,
	COMMUNITY,
	PDU,
	REQUESTID,
	ERRORSTATUS,
	ERRORID,
	VARBINDS,
	VARBIND,
	DONE
};

class SNMPGet
{
public:
	SNMPGet(const char *community, short version) : _community(community), _version(version)
	{
		if (version == 0)
		{
			version1 = true;
		}
		if (version == 1)
		{
			version2 = true;
		}
	};
	const char *_community;
	short _version;
	IPAddress agentIP;
	short port = 161;
	short requestID;
	short errorID = 0;
	short errorIndex = 0;

	// the setters that need to be configured for each Get.

	void setRequestID(short request)
	{
		requestID = request;
	}

	void setIP(IPAddress ip)
	{
		agentIP = ip;
	}

	void setPort(short portnumber)
	{
		port = portnumber;
	}

	void setUDP(UDP *udp)
	{
		_udp = udp;
	}

	void addOIDPointer(ValueCallback *callback);
	std::vector<ValueCallback*> callbacks;

	UDP *_udp = 0;
	bool sendTo(IPAddress ip)
	{
		if (!_udp)
		{
			return false;
		}
		if (!build())
		{
			Serial.println(F("Failed Building packet.."));
			delete packet;
			packet = 0;
			return false;
		}
		unsigned char _packetBuffer[SNMP_PACKET_LENGTH * 3];
		memset(_packetBuffer, 0, SNMP_PACKET_LENGTH * 3);
		int length = packet->serialise(_packetBuffer);
		delete packet;
		packet = 0;
#ifdef DEBUG
    Serial.print(F("[DEBUG] SNMPGet: Sending UDP packet to: "));
    Serial.print(ip);
    Serial.print(F(":"));
    Serial.println(port);
		Serial.print("[DEBUG] composed packet: ");
    for (int i = 0; i < length; i++)
    {
        Serial.printf("%02x ", _packetBuffer[i]);
    }
    Serial.println();
#endif
		_udp->beginPacket(ip, port);
		_udp->write(_packetBuffer, length);
		return _udp->endPacket();
	}

	ComplexType *packet = 0;
	bool build();

	bool version1 = false;
	bool version2 = false;

	void clearOIDList()
	{ 
		callbacks.clear();
	}
};

bool SNMPGet::build()
{
	// Build packet for making GetRequest
	if (packet)
	{
		packet = 0;
	}
	packet = new ComplexType(STRUCTURE);
	packet->addValueToList(new IntegerType((int)_version));
	packet->addValueToList(new OctetType((char *)_community));
	ComplexType *getPDU;
	getPDU = new ComplexType(GetRequestPDU);
	getPDU->addValueToList(new IntegerType(requestID));
	getPDU->addValueToList(new IntegerType(errorID));
	getPDU->addValueToList(new IntegerType(errorIndex));
	ComplexType *varBindList = new ComplexType(STRUCTURE);

	for (ValueCallback* callback : callbacks)
    {
        ComplexType *varBind = new ComplexType(STRUCTURE);
		varBind->addValueToList(new OIDType(callback->OID));
		// Value can be null for Request payload.
		BER_CONTAINER *value = new NullType();
		varBind->addValueToList(value);
		varBindList->addValueToList(varBind);
    }
	getPDU->addValueToList(varBindList);
	packet->addValueToList(getPDU);
	return true;
}

void SNMPGet::addOIDPointer(ValueCallback *callback)
{
	callbacks.push_back(callback);
}

#endif