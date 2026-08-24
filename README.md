# VRChat OSC Optitrack

## This branch uses a SteamVR driver, it's better for retaining tracking stability and has better VR environment integration. If you need to drive standalone headsets or want to use a single computer for sending OSC data, I recommend using the [main](https://github.com/FizzyApple12/VRChatOSCOptitrack/tree/main) branch that uses VRChat's OSC tracker endpoints.

This program allows you to take tracker data from OptiTrack Motive and send it to SteamVR as Trackers.

## Usage

### Compiling

The code should compile properly under Visual Studio 2022, but there are some steps you need to take:

1. After cloning the repository, you need to initialise the submodules in this project using the command ``git submodule update --init --recursive``

2. After the tool has been built, it needs a copy of [version 3.1 of the NatNet SDK](https://d2mzlempwep3hb.cloudfront.net/NatNetSDK/NatNet_SDK_3.1.zip), copy the file ``lib/NatNetLib.dll`` from the zip file to the same folder as the program.

### Running

To register the driver with SteamVR, do the following steps:

1. Compile and copy ``bin/x64/VRChatOSCOptitrack.dll`` to ``vrchatoscoptitrack/bin/win64/driver_vrchatoscoptitrack.dll``

2. Copy ``bin/x64/NatNetLib.dll`` to ``vrchatoscoptitrack/bin/win64/NatNetLib.dll``

3. Run ``& "C:\Program Files (x86)\Steam\steamapps\common\SteamVR\bin\win64\vrpathreg.exe" adddriver "C:\path\to\your\cloned\VRChatOSCOptitrack\vrchatoscoptitrack\"`` in PowerShell to register the driver with SteamVR

4. Run SteamVR

To maximize compatibility with different Skeleton types, the program uses the raw RigidBodies on a skeleton to represent "trackers". Each joint in VRChat needs to be assigned a tracker with the "Tracking Setup" menu. In the case of multiple Skeletons in the scene, as long as the IDs of the RigidBodies are persisted, the program should be able to find the correct skeleton.
