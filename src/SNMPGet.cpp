#include "SNMPGet.h"


bool SNMPGet::sendTo(IPAddress ip)
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

void SNMPGet::clearOIDList()
{ // this just removes the list, does not kill the values in the list
	ValueCallbackList* current = callbacks;
    
    while (current != nullptr) {
        ValueCallbackList* next = current->next;
        
        current->next = nullptr;
        
        // Only delete the list node, keep the callback objects alive
        // The caller is responsible for managing the callback objects
        delete current;
        
        current = next;
    }
    
    callbacks = nullptr;
}

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

	callbacksCursor = callbacks;
	if (callbacksCursor->value)
	{
		while (true)
		{
			ComplexType *varBind = new ComplexType(STRUCTURE);
			varBind->addValueToList(new OIDType(callbacksCursor->value->OID));
			// Value can be null for Request payload.
			BER_CONTAINER *value = new NullType();
			varBind->addValueToList(value);
			varBindList->addValueToList(varBind);

			if (callbacksCursor->next)
			{
				callbacksCursor = callbacksCursor->next;
			}
			else
			{
				break;
			}
		}
	}
	getPDU->addValueToList(varBindList);
	packet->addValueToList(getPDU);
	return true;
}

void SNMPGet::addOIDPointer(ValueCallback *callback)
{
	callbacksCursor = callbacks;
	if (callbacksCursor->value)
	{
		while (callbacksCursor->next != 0)
		{
			callbacksCursor = callbacksCursor->next;
		}
		callbacksCursor->next = new ValueCallbacks();
		callbacksCursor = callbacksCursor->next;
		callbacksCursor->value = callback;
		callbacksCursor->next = 0;
	}
	else
		callbacks->value = callback;
}