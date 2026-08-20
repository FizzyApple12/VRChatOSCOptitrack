#pragma once

#ifndef VRCHATOSC_H
#define VRCHATOSC_H

namespace VRChatOSC
{
	int Connect(int* ipAddress);
	void Disconnect();

	bool IsConnected();
	char* GetAddress();

	void NewMessage();

	void WritePosition(int trackerID, float x, float y, float z);
	void WriteRotation(int trackerID, float x, float y, float z);

	void SendMessage();
}

#endif