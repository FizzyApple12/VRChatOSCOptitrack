# VRChat OSC OptiTrack

This program allows you to take tracker data from OptiTrack Motive and send it to VRChat's OSC Tracker endpoint.

Note: This program is very primitive and needs some work to get correct. Most notably: there is no playspace calibration. To calibrate the playspaces between the Headset and the OptiTrack System, you should use the headset recenter button to move the headset playspace until the skeleton visually lines up with your body in game. Make sure to avoid using tools like Playspace Drag to prevent the playspaces from becoming misaligned.

## Usage

### Compiling

The code should compile properly under Visual Studio 2022, but there are some steps you need to take:

1. After cloning the repository, you need to initialize the submodules in this project using the command ``git submodule update --init --recursive``

2. After the tool has been built, it needs a copy of [version 3.1 of the NatNet SDK](https://d2mzlempwep3hb.cloudfront.net/NatNetSDK/NatNet_SDK_3.1.zip), copy the file ``lib/NatNetLib.dll`` from the zip file to the same folder as the program.

### Running

To maximize compatibility with different Skeleton types, the program uses the raw RigidBodies on a skeleton to represent "trackers". Each joint in VRChat needs to be assigned a tracker with the "Tracking Setup" menu. In the case of multiple Skeletons in the scene, as long as the IDs of the RigidBodies are persisted, the program should be able to find the correct skeleton.
