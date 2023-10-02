#include "main.h"

#include <windows.h>

#include "UI.h"
#include "VRChatOSC.h"
#include "NatNet.h"

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

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    UI::CreateUI();

    while (running)
    {
        UI::RenderEnvironment();

        for (int i = 0; i < NatNet::MarkerCount(); i++)
        {
            UI::RenderMarker(NatNet::GetMarker(i));
        }

        for (int i = 0; i < NatNet::RigidBodyCount(); i++)
        {
            NatNet::RigidBody activeRigidBody = NatNet::GetRigidBody(i);

            UI::RenderRigidBody(activeRigidBody);

            float positionX, positionY, positionZ;
            std::tie(positionX, positionY, positionZ) = activeRigidBody.position;

            float rotationX, rotationY, rotationZ;
            std::tie(rotationX, rotationY, rotationZ) = activeRigidBody.rotation;

            VRChatOSC::NewMessage();

            for (int j = 0; j < 8; j++) 
            {
                if (activeRigidBody.Id == oscOptiTrackIds[i])
                {
                    NatNet::RigidBody activeRigidBody = NatNet::GetRigidBody(oscOptiTrackIds[i]);

                    VRChatOSC::WritePosition(1, positionX, positionY, positionZ);
                    VRChatOSC::WriteRotation(1, rotationX, -rotationY, -rotationZ);
                }
            }

            VRChatOSC::SendMessage();
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