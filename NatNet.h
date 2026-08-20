#pragma once

#ifndef NATNET_H
#define NATNET_H

namespace NatNet
{
	int Connect(int* localAddress, int* serverAddress, bool multiCast);
	void Disconnect();

	bool IsConnected();
	char* GetLocalAddress();
	char* GetServerAddress();
	bool UsingMulticast();
}

#endif