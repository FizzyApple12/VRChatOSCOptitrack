#pragma once

#ifndef NATNET_H
#define NATNET_H

#include <tuple>

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

	int RigidBodyCount();
	RigidBody GetRigidBody(int index);

	int MarkerCount();
	Marker GetMarker(int index);
}

#endif