#include "main.h"

#include "openvr/headers/openvr_driver.h"
#include <windows.h>
#include <Eigen/Eigen>

#include "UI.h"
#include "NatNet.h"
#include "NatNetCollections.h"
#include "NatNetMath.h"
#include "TrackerDeviceProvider.h"
#include "openvr/headers/openvr_driver.h"

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

struct Pose
{
    Eigen::Vector3<float> position;
    Eigen::Quaternion<float> rotation;
};

Pose playspacePose = Pose{
    Eigen::Vector3<float>(0, 0, 0),
    Eigen::Quaternion<float>(0, 0, 0, 1)
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

void uiThreadEntrypoint()
{
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

        trackerFrameReceived();
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

std::string getSteamVRTrackerName(int trackerId)
{
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

Eigen::Matrix4<float> Matrix4TRS(Eigen::Vector3<float> position, Eigen::Quaternion<float> rotation, Eigen::Vector3<float> scale)
{
    Eigen::Matrix4<float> outputMatrix = Eigen::Matrix4<float>();

    Eigen::Vector3<float> column1 = rotation * Eigen::Vector3<float>(scale.x(), 0, 0);
    Eigen::Vector3<float> column2 = rotation * Eigen::Vector3<float>(0, scale.y(), 0);
    Eigen::Vector3<float> column3 = rotation * Eigen::Vector3<float>(0, 0, scale.z());
    Eigen::Vector3<float> column4 = position;

    outputMatrix.col(0) = Eigen::Vector4<float>(column1.x(), column1.y(), column1.z(), 0);
    outputMatrix.col(1) = Eigen::Vector4<float>(column2.x(), column2.y(), column2.z(), 0);
    outputMatrix.col(2) = Eigen::Vector4<float>(column3.x(), column3.y(), column3.z(), 0);
    outputMatrix.col(3) = Eigen::Vector4<float>(column4.x(), column4.y(), column4.z(), 1);

    return outputMatrix;
}

Eigen::Quaternion<float> LookRotation(Eigen::Vector3<float> forward, Eigen::Vector3<float> up)
{
    forward.normalize();

    Eigen::Vector3<float> right = up.cross(forward);
    right.normalize();

    Eigen::Vector3<float> newUp = forward.cross(right);
    newUp.normalize();

    Eigen::Matrix3<float> matrix = Eigen::Matrix3<float>();

    matrix.col(0) = right;
    matrix.col(1) = newUp;
    matrix.col(2) = forward;

    return Eigen::Quaternion<float>(matrix);
}

Pose TransformPlayspace(
    Pose root,
    Pose from,
    Pose to,
    bool enforceUp = true
)
{
    Eigen::Matrix4<float> rigMat = Matrix4TRS(root.position, root.rotation, Eigen::Vector3<float>(1, 1, 1));
    Eigen::Matrix4<float> anchorMat = Matrix4TRS(from.position, from.rotation, Eigen::Vector3<float>(1, 1, 1));
    Eigen::Matrix4<float> desiredMat = Matrix4TRS(to.position, to.rotation, Eigen::Vector3<float>(1, 1, 1));

    if (enforceUp)
    {
        Eigen::Transform<float, 3, Eigen::Affine> anchorMatTransform(anchorMat);
        Eigen::Vector3<float> flatForward = anchorMatTransform * Eigen::Vector3<float>(0, 0, 1);
        flatForward = Eigen::Vector3<float>(flatForward.x(), 0, flatForward.z()).normalized();
        Eigen::Quaternion<float> correctedRot = LookRotation(flatForward, Eigen::Vector3<float>(0, 1, 0));
        anchorMat = Matrix4TRS(anchorMat.col(3).head<3>(), correctedRot, Eigen::Vector3<float>(1, 1, 1));
    }

    // the rig relative to the anchor
    Eigen::Matrix4<float> rigLocalToAnchor = anchorMat.inverse() * rigMat;

    // that relative matrix relative to the desired transform
    Eigen::Matrix4<float> relativeToDesired = desiredMat * rigLocalToAnchor;
    Eigen::Affine3f relativeToDesiredTransform(relativeToDesired);

    return Pose{
        relativeToDesiredTransform.translation(),
        Eigen::Quaternion<float>(relativeToDesiredTransform.rotation())
    };
}


void recenterOptiTrackPlayspace()
{
    Pose from;
    bool foundHead = false;

    for (int i = 0; i < NatNetRigidBodyCollection::GetCount(); i++)
    {
        NatNet::RigidBody activeRigidBody = NatNetRigidBodyCollection::Get(i);

        if (activeRigidBody.id == getSteamVRTrackerNumber(0))
        {

        }
    }

    if (!foundHead) {
        return;
    }

    vr::TrackedDevicePose_t hmdPose{};
	vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0.f, &hmdPose, 1);

    if (!hmdPose.bPoseIsValid) {
        return;
    }

    Eigen::Map<Eigen::Matrix<float, 3, 4, Eigen::RowMajor>> hmdMatrix(&hmdPose.mDeviceToAbsoluteTracking.m[0][0]);

    Eigen::Affine3f hmdTransform(hmdMatrix);

    playspacePose = TransformPlayspace(playspacePose, from, Pose{
            hmdTransform.translation(),
            Eigen::Quaternion<float>(hmdTransform.rotation()),
        });

    optiTrackTrackerDeviceProvider.SetPlayspaceOffset(playspacePose.position, playspacePose.rotation);
}

void resetOptiTrackPlayspace()
{
    playspacePose = Pose{
        Eigen::Vector3<float>(0, 0, 0),
        Eigen::Quaternion<float>(0, 0, 0, 1)
    };

    optiTrackTrackerDeviceProvider.SetPlayspaceOffset(playspacePose.position, playspacePose.rotation);
}

void trackerFrameReceived()
{
    for (int i = 0; i < NatNetRigidBodyCollection::GetCount(); i++)
    {
        for (const auto& tracker : optiTrackTrackerDeviceProvider.optiTrackTrackerDevices)
        {
            NatNet::RigidBody activeRigidBody = NatNetRigidBodyCollection::Get(i);

            if (activeRigidBody.id == getSteamVRTrackerNumber(tracker->trackerIndex + 1))
            {
                tracker->UpdateOptiTrackPose(activeRigidBody);
            }
        }
    }
}