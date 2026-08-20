#pragma once

#ifndef TRACKER_DEVICE_PROVIDER_H
#define TRACKER_DEVICE_PROVIDER_H

#include <memory>

#include "openvr/headers/openvr_driver.h"
#include "TrackerDeviceDriver.h"
#include <Eigen/Eigen>

class OptiTrackTrackerDeviceProvider : public vr::IServerTrackedDeviceProvider
{
public:
	// vr::IServerTrackedDeviceProvider
	vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext) override;
	const char* const* GetInterfaceVersions() override;

	void RunFrame() override;

	bool ShouldBlockStandbyMode() override;
	void EnterStandby() override;
	void LeaveStandby() override;

	void Cleanup() override;

	// other implementations

	std::vector<std::unique_ptr<OptiTrackTrackerDeviceDriver>> optiTrackTrackerDevices;
};

#endif TRACKER_DEVICE_DRIVER_H