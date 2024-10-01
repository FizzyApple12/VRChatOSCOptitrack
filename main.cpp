#include "main.h"

#include <windows.h>

#include "UI.h"
#include "VRChatOSC.h"
#include "NatNet.h"
#include "NatNetCollections.h"
#include "NatNetMath.h"

bool running = true;

int oscHeadOptiTrackId = 5;
int oscOptiTrackIds[8] = {
    1,  // hip

    3,  // chest

    16, // left foot
    20, // right foot

    15, // left knee
    19, // right knee

    8,  // left elbow
    12, // right elbow
};

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    UI::CreateUI();

    while (running)
    {
        UI::RenderEnvironment();

        for (int i = 0; i < NatNetRigidBodyCollection::GetCount(); i++)
        {
            NatNet::RigidBody activeRigidBody = NatNetRigidBodyCollection::Get(i);

            UI::RenderRigidBody(activeRigidBody);

            VRChatOSC::NewMessage();

            for (int j = 0; j < 8; j++) 
            {
                if (activeRigidBody.id == oscOptiTrackIds[j])
                {
                    VRChatOSC::WritePosition(j, -activeRigidBody.x, activeRigidBody.y, activeRigidBody.z);

                    NatNetMath::EulerAngles convertedAngles = trackerToVRChat(activeRigidBody);

                    VRChatOSC::WriteRotation(j, convertedAngles.x, convertedAngles.y, convertedAngles.z);
                }
            }

            VRChatOSC::SendMessage();
        }

        for (int i = 0; i < NatNetMarkerCollection::GetCount(); i++)
        {
            UI::RenderMarker(NatNetMarkerCollection::Get(i));
        }

        UI::RenderUI();
    }

    VRChatOSC::Disconnect();
    NatNet::Disconnect();

    UI::DestroyUI();

    return 0;
}

void mainExit()
{
    running = false;
}

void setOSCTrackerNumber(int oscId, int optitrackId)
{
    if (oscId == 0) oscHeadOptiTrackId = optitrackId;

    oscOptiTrackIds[oscId - 1] = optitrackId;
}

int getOSCTrackerNumber(int oscId)
{
    if (oscId == 0) return oscHeadOptiTrackId;

    return oscOptiTrackIds[oscId - 1];
}

NatNetMath::EulerAngles trackerToVRChat(NatNet::RigidBody rigidbody)
{
    NatNetMath::Quaternion rotation = { // this had odd negatives because we need a left-handed rotation
        rigidbody.rx,
        -rigidbody.ry,
        -rigidbody.rz,
        rigidbody.rw
    };

    return NatNetMath::Eul_FromQuat(rotation);
}