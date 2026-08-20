#include "NatNetCollections.h"
#include "NatNet.h"

#define MAX_MARKER_COUNT 4096
#define MAX_RIGIDBODY_COUNT 4096

namespace NatNetRigidBodyCollection
{

	NatNet::RigidBody rigidBodies[MAX_RIGIDBODY_COUNT];
	int numberActiveRigidBodies = 0;

	bool Update(NatNet::RigidBody rigidBody)
	{
		for (int i = 0; i < numberActiveRigidBodies; i++)
		{
			if (rigidBodies[i].id == rigidBody.id)
			{
				rigidBodies[i] = rigidBody;

				return true;
			}
		}

		return false;
	}

	bool Append(NatNet::RigidBody rigidBody)
	{
		if (numberActiveRigidBodies < MAX_RIGIDBODY_COUNT)
		{
			rigidBodies[numberActiveRigidBodies] = rigidBody;

			numberActiveRigidBodies++;

			return true;
		}

		return false;
	}

	int GetCount()
	{
		return numberActiveRigidBodies;
	}

	int GetMax()
	{
		return MAX_RIGIDBODY_COUNT;
	}

	NatNet::RigidBody Get(int index)
	{
		return rigidBodies[index];
	}

}

namespace NatNetMarkerCollection
{

	NatNet::Marker markers[MAX_MARKER_COUNT];
	int numberActiveMarkers = 0;

	void Clear()
	{
		numberActiveMarkers = 0;
	}

	bool Append(NatNet::Marker marker)
	{
		if (numberActiveMarkers < MAX_RIGIDBODY_COUNT)
		{
			markers[numberActiveMarkers];

			numberActiveMarkers++;

			return true;
		}

		return false;
	}

	int GetCount()
	{
		return numberActiveMarkers;
	}

	int GetMax()
	{
		return MAX_MARKER_COUNT;
	}

	NatNet::Marker Get(int index)
	{
		return markers[index];
	}

}