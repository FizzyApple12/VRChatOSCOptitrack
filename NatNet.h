#pragma once

#ifndef NATNET_H
#define NATNET_H

#include <tuple>

#include "NatNetTypes.h"
#include "NatNetCAPI.h"
#include "NatNetClient.h"
#include "NatNetMath.h"

namespace NatNet
{
	typedef struct
	{
		std::tuple<float, float, float> position;
		std::tuple<float, float, float> rotation;

		int id;

		char* name;
	} RigidBody;

	typedef struct
	{
		std::tuple<float, float, float> position;

		int id;
	} Marker;

	int Connect(int* localAddress, int* serverAddress, bool multiCast);
	void Disconnect();

	bool IsConnected();
	char* GetLocalAddress();
	char* GetServerAddress();
	bool UsingMulticast();

	void NATNET_CALLCONV dataHandler(sFrameOfMocapData* data, void* pUserData);
	void NATNET_CALLCONV messageHandler(Verbosity msgType, const char* msg);
	bool parseRigidBodyDescription(sDataDescriptions* pDataDefs);
}

#endif