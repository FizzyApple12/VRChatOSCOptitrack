#pragma once

#ifndef NATNETCOLLECTIONS_H
#define NATNETCOLLECTIONS_H

#include "NatNet.h"

namespace NatNetRigidBodyCollection
{
	bool Update(NatNet::RigidBody rigidBody);

	bool Append(NatNet::RigidBody rigidBody);

	int GetCount();
	int GetMax();

	NatNet::RigidBody Get(int index);
}

namespace NatNetMarkerCollection
{
	void Clear();

	bool Append(NatNet::Marker marker);

	int GetCount();
	int GetMax();

	NatNet::Marker Get(int index);
}

#endif