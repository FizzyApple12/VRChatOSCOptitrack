#include "NatNet.h"

#include <windows.h>
#include <winsock.h>
#include <string>

#include "NatNetTypes.h"
#include "NatNetCAPI.h"
#include "NatNetClient.h"

namespace NatNet
{

    NatNetClient natnetClient;

    unsigned char ver[4];

    char natnetLocalAddress[16];
    char natnetServerAddress[16];

    bool connected = false;

    float toMMConversionFactor = 1.0f;

    int upAxis = 1;

    int Connect(int* localAddress, int* serverAddress, bool multiCast)
    {
        NatNet_GetVersion(ver);

        //NatNet_SetLogCallback(MessageHandler);

       // natnetClient.SetFrameReceivedCallback(DataHandler);

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

    int RigidBodyCount()
    {
        return 0;
    }
    RigidBody GetRigidBody(int index)
    {
        return {

        };
    }

    int MarkerCount()
    {
        return 0;
    }
    Marker GetMarker(int index)
    {
        return {

        };
    }
}