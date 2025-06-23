#include "main.h"

#include "openvr/headers/openvr_driver.h"
#include <windows.h>

#include "UI.h"
#include "NatNet.h"
#include "NatNetCollections.h"
#include "NatNetMath.h"
#include "TrackerDeviceProvider.h"

#if defined( _WIN32 )
#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )
#define HMD_DLL_IMPORT extern "C" __declspec( dllimport )
#elif defined( __GNUC__ ) || defined( COMPILER_GCC ) || defined( __APPLE__ )
#define HMD_DLL_EXPORT extern "C" __attribute__( ( visibility( "default" ) ) )
#define HMD_DLL_IMPORT extern "C"
#else
#error "Unsupported Platform."
#endif

OptiTrackTrackerDeviceProvider optiTrackTrackerDeviceProvider;

bool running = true;

int headOptiTrackId = 5;
int optiTrackIds[8] = {
    1,  // hip

    3,  // chest

    16, // left foot
    20, // right foot

    15, // left knee
    19, // right knee

    8,  // left elbow
    12, // right elbow
};

HMD_DLL_EXPORT void* HmdDriverFactory(const char* pInterfaceName, int* pReturnCode)
{
    if (0 == strcmp(vr::IServerTrackedDeviceProvider_Version, pInterfaceName))
    {
        return &optiTrackTrackerDeviceProvider;
    }

    if (pReturnCode)
        *pReturnCode = vr::VRInitError_Init_InterfaceNotFound;

    return NULL;
}

std::thread uiThread;

void uiThreadEntrypoint() {
    UI::CreateUI();

    while (running)
    {
        UI::RenderEnvironment();

        for (int i = 0; i < NatNetRigidBodyCollection::GetCount(); i++)
        {
            NatNet::RigidBody activeRigidBody = NatNetRigidBodyCollection::Get(i);

            UI::RenderRigidBody(activeRigidBody);
        }

        for (int i = 0; i < NatNetMarkerCollection::GetCount(); i++)
        {
            UI::RenderMarker(NatNetMarkerCollection::Get(i));
        }

        UI::RenderUI();
    }

    UI::DestroyUI();
}

bool StartApplicationThread()
{
    uiThread = std::thread(uiThreadEntrypoint);

    return true;
}

void ExitApplicationThread()
{
    running = false;

    uiThread.join();
}

void setSteamVRTrackerNumber(int trackerId
    , int optitrackId)
{
    const int oscOptiTrackIdsLength = sizeof(optiTrackIds) / sizeof(int);

    if (trackerId <= 0) {
        headOptiTrackId = optitrackId;
        return;
    }
    if (trackerId > oscOptiTrackIdsLength) {
        optiTrackIds[oscOptiTrackIdsLength - 1] = optitrackId;
        return;
    }

    optiTrackIds[trackerId - 1] = optitrackId;
}

int getSteamVRTrackerNumber(int trackerId)
{
    const int oscOptiTrackIdsLength = sizeof(optiTrackIds) / sizeof(int);

    if (trackerId <= 0) return headOptiTrackId;
    if (trackerId > oscOptiTrackIdsLength) return optiTrackIds[oscOptiTrackIdsLength - 1];

    return optiTrackIds[trackerId - 1];
}

std::string getSteamVRTrackerName(int trackerId) {
    switch (trackerId) {
    case 0:
        return "Head";
    case 1:
        return "Hip";
    case 2:
        return "Chest";
    case 3: 
        return "Left Foot";
    case 4: 
        return "Right Foot";
    case 5: 
        return "Left Knee";
    case 6: 
        return "Right Knee";
    case 7: 
        return "Left Elbow";
    case 8: 
        return "Right Elbow";
    }

    return "Unkown ID: " + std::to_string(trackerId);
}