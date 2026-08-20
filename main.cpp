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

Eigen::Affine3f playspacePose = Eigen::Affine3f::Identity();

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

Eigen::Affine3f TransformPlayspace(
	Eigen::Affine3f root,
	Eigen::Affine3f from,
	Eigen::Affine3f to
)
{
	Eigen::Matrix4<float> rigMat = root.matrix();
	Eigen::Matrix4<float> anchorMat = from.matrix();
	Eigen::Matrix4<float> desiredMat = to.matrix();

	// the rig relative to the anchor
	Eigen::Matrix4<float> rigLocalToAnchor = anchorMat.inverse() * rigMat;

	// that relative matrix relative to the desired transform
	Eigen::Matrix4<float> relativeToDesired = desiredMat * rigLocalToAnchor;
	Eigen::Affine3f relativeToDesiredTransform(relativeToDesired);

	return relativeToDesiredTransform;
}


void recenterOptiTrackPlayspace()
{
	Eigen::Affine3f from;
	bool foundHead = false;

	for (int i = 0; i < NatNetRigidBodyCollection::GetCount(); i++)
	{
		NatNet::RigidBody activeRigidBody = NatNetRigidBodyCollection::Get(i);

		if (activeRigidBody.id == getSteamVRTrackerNumber(0))
		{
			Eigen::Vector3f pos = Eigen::Vector3f(activeRigidBody.x, activeRigidBody.y, activeRigidBody.z);
			Eigen::Quaternionf rot = Eigen::Quaternionf(activeRigidBody.rw, activeRigidBody.rx, activeRigidBody.ry, activeRigidBody.rz);

			from = Eigen::Affine3f::Identity();
			from.translate(pos).rotate(rot);

			foundHead = true;
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

	playspacePose = TransformPlayspace(Eigen::Affine3f::Identity(), from, hmdTransform);
}

void resetOptiTrackPlayspace()
{
	playspacePose = Eigen::Affine3f::Identity();
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
				tracker->UpdateOptiTrackPose(activeRigidBody, playspacePose);
			}
		}
	}
}