#include "TrackerDeviceProvider.h"
#include "TrackerDeviceDriver.h"
#include "NatNetCollections.h"
#include "main.h"
#include <Eigen/Eigen>

vr::EVRInitError OptiTrackTrackerDeviceProvider::Init(vr::IVRDriverContext* pDriverContext)
{
	VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

	for (unsigned int i = 0; i < 8; i++)
	{
		std::unique_ptr<OptiTrackTrackerDeviceDriver> optiTrackTrackerDevice = std::make_unique<OptiTrackTrackerDeviceDriver>(i);

		if (!vr::VRServerDriverHost()->TrackedDeviceAdded(optiTrackTrackerDevice->serialNumber.c_str(),
			vr::TrackedDeviceClass_GenericTracker, optiTrackTrackerDevice.get()))
		{
			return vr::VRInitError_Driver_Unknown;
		}

		optiTrackTrackerDevices.emplace_back(std::move(optiTrackTrackerDevice));
	}

	if (!StartApplicationThread()) {
		return vr::VRInitError_Driver_Unknown;
	}

	return vr::VRInitError_None;
}

const char* const* OptiTrackTrackerDeviceProvider::GetInterfaceVersions()
{
	return vr::k_InterfaceVersions;
}

bool OptiTrackTrackerDeviceProvider::ShouldBlockStandbyMode()
{
	return false;
}

void OptiTrackTrackerDeviceProvider::RunFrame()
{
	vr::VREvent_t vrevent{};
	while (vr::VRServerDriverHost()->PollNextEvent(&vrevent, sizeof(vr::VREvent_t)))
	{
		// no events to process, keep this here to make the steamvr gods happy :)
	}
}

void OptiTrackTrackerDeviceProvider::EnterStandby()
{
}

void OptiTrackTrackerDeviceProvider::LeaveStandby()
{
}

void OptiTrackTrackerDeviceProvider::Cleanup()
{
	for (auto& tracker : optiTrackTrackerDevices)
	{
		tracker = nullptr;
	}

	NatNet::Disconnect();

	ExitApplicationThread();
}