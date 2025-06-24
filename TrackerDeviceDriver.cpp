#include "TrackerDeviceDriver.h"

#include "NatNet.h"
#include "main.h"

static const char* mainSettingsSection = "driver_vrchatoscoptitrack";
static const char* settingsKeyModelNumber = "optitrack_tracker_model_number";

OptiTrackTrackerDeviceDriver::OptiTrackTrackerDeviceDriver(unsigned int newTrackerIndex)
{
	isActive = false;

	trackerIndex = newTrackerIndex;

	char model_number[1024];
	vr::VRSettings()->GetString(
		mainSettingsSection, settingsKeyModelNumber, model_number, sizeof(model_number));
	modelNumber = model_number;

	serialNumber = modelNumber + " " + getSteamVRTrackerName(trackerIndex + 1);
}

vr::EVRInitError OptiTrackTrackerDeviceDriver::Activate(uint32_t newSteamVRDeviceIndex)
{
	isActive = true;
	steamVRDeviceIndex = newSteamVRDeviceIndex;

	vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer(steamVRDeviceIndex);

	vr::VRProperties()->SetStringProperty(container, vr::Prop_ModelNumber_String, modelNumber.c_str());
	vr::VRProperties()->SetStringProperty(container, vr::Prop_SerialNumber_String, serialNumber.c_str());

	vr::VRProperties()->SetStringProperty(container, vr::Prop_InputProfilePath_String, "{vrchatoscoptitrack}/input/optitrack_tracker_profile.json");

	return vr::VRInitError_None;
}

void* OptiTrackTrackerDeviceDriver::GetComponent(const char* pchComponentNameAndVersion)
{
	return nullptr;
}

void OptiTrackTrackerDeviceDriver::DebugRequest(
	const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
	if (unResponseBufferSize >= 1)
		pchResponseBuffer[0] = 0;
}

vr::DriverPose_t OptiTrackTrackerDeviceDriver::GetPose()
{
	return lastReceivedPose;
}

void OptiTrackTrackerDeviceDriver::UpdateOptiTrackPose(NatNet::RigidBody rigidbody)
{
	if (isActive)
	{
		vr::DriverPose_t pose = { 0 };

		pose.qWorldFromDriverRotation.w = 1.f;
		pose.qDriverFromHeadRotation.w = 1.f;

		// if fucked up try the comments
		pose.vecPosition[0] = rigidbody.x; // make -
		pose.vecPosition[1] = rigidbody.y; // make +
		pose.vecPosition[2] = rigidbody.z; // make +

		// if fucked up try the comments
		pose.qRotation.x = rigidbody.rx; // make +
		pose.qRotation.y = rigidbody.ry; // make -
		pose.qRotation.z = rigidbody.rz; // make -
		pose.qRotation.w = rigidbody.rw; // make +

		pose.poseIsValid = true;
		pose.deviceIsConnected = true;
		pose.result = vr::TrackingResult_Running_OK;

		vr::VRServerDriverHost()->TrackedDevicePoseUpdated(steamVRDeviceIndex, pose, sizeof(vr::DriverPose_t));

		lastReceivedPose = pose;
	}
}

void OptiTrackTrackerDeviceDriver::EnterStandby()
{

}

void OptiTrackTrackerDeviceDriver::Deactivate()
{
	isActive = false;

	steamVRDeviceIndex = vr::k_unTrackedDeviceIndexInvalid;
}