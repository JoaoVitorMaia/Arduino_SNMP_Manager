#ifndef SNMPGet_h
#define SNMPGet_h
#include <IPAddress.h>

#include <Arduino_SNMP_Manager.h>

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
	ValueCallbacks *callbacks = new ValueCallbacks();;
	ValueCallbacks *callbacksCursor = callbacks;


	UDP *_udp = 0;
	bool sendTo(IPAddress ip);

	ComplexType *packet = 0;
	bool build();

	bool version1 = false;
	bool version2 = false;

	void clearOIDList();
};


#endif