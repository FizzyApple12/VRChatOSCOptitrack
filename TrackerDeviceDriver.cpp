#include "TrackerDeviceDriver.h"

#include "NatNet.h"
#include "main.h"
#include <Eigen/Eigen>

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

void OptiTrackTrackerDeviceDriver::UpdateOptiTrackPose(NatNet::RigidBody rigidbody, Eigen::Affine3f motiveStageTransform)
{
	if (isActive)
	{
		vr::DriverPose_t pose = { 0 };

		Eigen::Affine3f inv = motiveStageTransform.inverse();

		Eigen::Affine3f rbody = Eigen::Affine3f::Identity();
		rbody.translate(Eigen::Vector3f(rigidbody.x, rigidbody.y, rigidbody.z));
		rbody.rotate(Eigen::Quaternionf(rigidbody.rw, rigidbody.rx, rigidbody.ry, rigidbody.rz));


		inv = Eigen::Affine3f::Identity();
		inv.rotate(Eigen::AngleAxisf(3.14159265358979323846f, Eigen::Vector3f(0, 1, 0)));
		rbody = inv * rbody;

		Eigen::Vector3f trans = rbody.translation();
		Eigen::Quaternionf rot = Eigen::Quaternionf(rbody.rotation());

		pose.vecWorldFromDriverTranslation[0] = 0.f;
		pose.vecWorldFromDriverTranslation[1] = 0.f;
		pose.vecWorldFromDriverTranslation[2] = 0.f;

		pose.qWorldFromDriverRotation.x = 0.f;
		pose.qWorldFromDriverRotation.y = 0.f;
		pose.qWorldFromDriverRotation.z = 0.f;
		pose.qWorldFromDriverRotation.w = 1.f;

		pose.qDriverFromHeadRotation.x = 0.f;
		pose.qDriverFromHeadRotation.y = 0.f;
		pose.qDriverFromHeadRotation.z = 0.f;
		pose.qDriverFromHeadRotation.w = 1.f;

		
		// if fucked up try the comments
		pose.vecPosition[0] = trans.x(); // make -
		pose.vecPosition[1] = trans.y(); // make +
		pose.vecPosition[2] = trans.z(); // make +

		// if fucked up try the comments
		pose.qRotation.x = rot.x(); // make +
		pose.qRotation.y = rot.y(); // make -
		pose.qRotation.z = rot.z(); // make -
		pose.qRotation.w = rot.w(); // make +

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