#pragma once

#ifndef TRACKER_DEVICE_DRIVER_H
#define TRACKER_DEVICE_DRIVER_H

#include "NatNet.h"

#include <array>
#include <string>

#include "openvr/headers/openvr_driver.h"
#include <atomic>
#include <thread>

class OptiTrackTrackerDeviceDriver : public vr::ITrackedDeviceServerDriver
{
public:
	// vr::ITrackedDeviceServerDriver
	OptiTrackTrackerDeviceDriver(unsigned int newTrackerIndex);

	vr::EVRInitError Activate(uint32_t newSteamVRDeviceIndex) override;

	void EnterStandby() override;

	void* GetComponent(const char* pchComponentNameAndVersion) override;

	void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;

	vr::DriverPose_t GetPose() override;

	void Deactivate() override;

	// other implementations

	void UpdateOptiTrackPose(NatNet::RigidBody rigidbody);

	unsigned int trackerIndex;
	std::atomic<bool> isActive;
	std::atomic<vr::TrackedDeviceIndex_t> steamVRDeviceIndex;

	std::atomic<vr::DriverPose_t> lastReceivedPose;

	std::string modelNumber;
	std::string serialNumber;
};

#endif TRACKER_DEVICE_DRIVER_H