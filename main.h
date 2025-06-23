#pragma once

#ifndef MAIN_H
#define MAIN_H

#include "NatNet.h"
#include "NatNetMath.h"

bool StartApplicationThread();
void ExitApplicationThread();

void setSteamVRTrackerNumber(int trackerId, int optitrackId);
int getSteamVRTrackerNumber(int trackerId);
std::string getSteamVRTrackerName(int trackerId);

#endif;