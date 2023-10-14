#pragma once

#ifndef MAIN_H
#define MAIN_H

#include "NatNet.h"
#include "NatNetMath.h"

void mainExit();

void setOSCTrackerNumber(int oscId, int optitrackId);
int getOSCTrackerNumber(int oscId);

NatNetMath::EulerAngles trackerToVRChat(NatNet::RigidBody rigidbody);

#endif;