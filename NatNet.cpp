#include "NatNet.h"

#include <windows.h>
#include <winsock.h>
#include <string>

#include "NatNetTypes.h"
#include "NatNetCAPI.h"
#include "NatNetClient.h"
#include "NatNetCollections.h"
#include <map>

namespace NatNet
{

    NatNetClient natnetClient;

    unsigned char ver[4];

    char natnetLocalAddress[32];
    char natnetServerAddress[32];

    bool connected = false;

    float toMMConversionFactor = 1.0f;

    int upAxis = 1;

    std::map<int, std::string> idToNameMap;

    int Connect(int* localAddress, int* serverAddress, bool multiCast)
    {
        NatNet_GetVersion(ver);

        NatNet_SetLogCallback(messageHandler);

        natnetClient.SetFrameReceivedCallback(dataHandler);

        sprintf(natnetLocalAddress, "%d.%d.%d.%d", localAddress[0], localAddress[1], localAddress[2], localAddress[3]);
        sprintf(natnetServerAddress, "%d.%d.%d.%d", serverAddress[0], serverAddress[1], serverAddress[2], serverAddress[3]);

        sNatNetClientConnectParams connectParams;

        connectParams.connectionType = multiCast ? ConnectionType_Multicast : ConnectionType_Unicast; //
        connectParams.localAddress = natnetLocalAddress;
        connectParams.serverAddress = natnetServerAddress;

        if (natnetClient.Connect(connectParams) != ErrorCode_OK)
            return 1;


        sServerDescription ServerDescription;

        memset(&ServerDescription, 0, sizeof(ServerDescription));

        natnetClient.GetServerDescription(&ServerDescription);

        if (!ServerDescription.HostPresent)
            return 2;



        sDataDescriptions* pDataDefs = NULL;

        if (natnetClient.GetDataDescriptionList(&pDataDefs) != ErrorCode_OK || 
           parseRigidBodyDescription(pDataDefs) == false)
            return 3;

        NatNet_FreeDescriptions(pDataDefs);

        pDataDefs = NULL;


        void* response;
        int nBytes;

        if (natnetClient.SendMessageAndWait("UnitsToMillimeters", &response, &nBytes) == ErrorCode_OK)
            toMMConversionFactor = *(float*) response;

        if (natnetClient.SendMessageAndWait("UpAxis", &response, &nBytes) == ErrorCode_OK)
            upAxis = *(long*) response;

        connected = true;

        return 0;
    }

    void Disconnect()
    {
        if (!connected)
            return;

        natnetClient.Disconnect();

        connected = false;
    }

    bool IsConnected()
    {
        return connected;
    }

    char* GetLocalAddress()
    {
        return natnetLocalAddress;
    }

    char* GetServerAddress()
    {
        return natnetServerAddress;
    }

    bool UsingMulticast()
    {
        return false;
    }

    void NATNET_CALLCONV messageHandler(Verbosity msgType, const char* msg)
    {
        // Don't delete, might be useful later...
    }


    void dataHandler(sFrameOfMocapData* data, void* pUserData)
    {
        int markerCount = min(NatNetMarkerCollection::GetMax(), data->MocapData->nMarkers);

        NatNetMarkerCollection::Clear();

        for (int i = 0; i < markerCount; i++)
        {
            float x = *data->MocapData->Markers[0];
            float y = *data->MocapData->Markers[1];
            float z = *data->MocapData->Markers[2];

            if (upAxis == 2)
                NatNetMath::ConvertRHSPosZupToYUp(x, y, z);

            NatNetMarkerCollection::Append({
                {
                    x, y, z
                },

                i
            });
        }

        int rigidBodyCount = min(512, data->MocapData->nMarkers);

        for (int i = 0; i < data->nSkeletons; i++)
        {
            for (size_t j = 0; j < data->Skeletons[i].nRigidBodies; ++j)
            {
                float x = data->Skeletons[i].RigidBodyData[j].x;
                float y = data->Skeletons[i].RigidBodyData[j].y;
                float z = data->Skeletons[i].RigidBodyData[j].z;

                NatNetMath::Quaternion rotation = {
                    data->Skeletons[i].RigidBodyData[j].qz,
                    data->Skeletons[i].RigidBodyData[j].qy,
                    data->Skeletons[i].RigidBodyData[j].qz,
                    data->Skeletons[i].RigidBodyData[j].qw
                };

                if (upAxis == 2)
                {
                    NatNetMath::ConvertRHSPosZupToYUp(x, y, z);
                    NatNetMath::ConvertRHSRotZUpToYUp(rotation.x, rotation.y, rotation.z, rotation.w);
                }

                NatNetMath::EulerAngles rotationEuler = NatNetMath::Eul_FromQuat(rotation, EulOrdZYXr);

                NatNet::RigidBody rigidBody = {
                    {
                        x, y, z
                    },

                    {
                        rotationEuler.x * (180.0f / MATH_PI),
                        rotationEuler.y * (180.0f / MATH_PI),
                        rotationEuler.z * (180.0f / MATH_PI)
                    },

                    data->Skeletons[i].RigidBodyData[j].ID,

                    idToNameMap[data->Skeletons[i].RigidBodyData[j].ID]
                };

                if (!NatNetRigidBodyCollection::Update(rigidBody))
                    NatNetRigidBodyCollection::Append(rigidBody);
            }
        }
    }

    bool parseRigidBodyDescription(sDataDescriptions* dataDefinitions)
    {
        idToNameMap.clear();

        if (dataDefinitions == NULL || dataDefinitions->nDataDescriptions <= 0)
            return false;

        for (int i = 0; i < dataDefinitions->nDataDescriptions; i++)
        {
            switch (dataDefinitions->arrDataDescriptions[i].type)
            {
                case Descriptor_RigidBody:
                {
                    sRigidBodyDescription* rigidBodyDescription = dataDefinitions->arrDataDescriptions[i].Data.RigidBodyDescription;

                    idToNameMap[rigidBodyDescription->ID] = std::string(rigidBodyDescription->szName);

                    break;
                }
                case Descriptor_Skeleton:
                {
                    sSkeletonDescription* skeletonDescription = dataDefinitions->arrDataDescriptions[i].Data.SkeletonDescription;

                    for (int j = 0; j < skeletonDescription->nRigidBodies; j++)
                    {
                        int id = skeletonDescription->RigidBodies[j].ID | (skeletonDescription->skeletonID << 16);

                        idToNameMap[id] = std::string(skeletonDescription->RigidBodies[j].szName);
                    }

                    break;
                }
            }
        }

        return true;
    }

    std::string GetMappedName(int index)
    {
        return idToNameMap.at(index);
    }
}