#pragma once

#ifndef NATNET_H
#define NATNET_H

#include <tuple>
#include <string>

#include "NatNetTypes.h"
#include "NatNetCAPI.h"
#include "NatNetClient.h"
#include "NatNetMath.h"

namespace NatNet
{
	typedef struct
	{
		float x;
		float y;
		float z;

		float rx;
		float ry;
		float rz;
		float rw;

		int id;
	} RigidBody;

	typedef struct
	{
		float x;
		float y;
		float z;

		int id;
	} Marker;

	int Connect(int* localAddress, int* serverAddress, bool multiCast);
	void Disconnect();

	bool IsConnected();
	char* GetLocalAddress();
	char* GetServerAddress();
	bool UsingMulticast();

	std::string GetMappedName(int index);

	void NATNET_CALLCONV dataHandler(sFrameOfMocapData* data, void* pUserData);
	void NATNET_CALLCONV messageHandler(Verbosity msgType, const char* msg);
	bool parseRigidBodyDescription(sDataDescriptions* pDataDefs);
}

#endif