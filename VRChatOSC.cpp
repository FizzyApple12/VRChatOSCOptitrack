#include "VRChatOSC.h"

#include <windows.h>
#include <winsock.h>
#include "tinyosc.h"
#include <string>

#define SEND_BUFFER_SIZE 1024

#define VRCHAT_SEND_PORT 9000
#define VRCHAT_RECV_PORT 9001

namespace VRChatOSC
{
    SOCKET vrchatSocket = -1;
    struct sockaddr_in vrchatSocketServer;
    bool connected = false;

    char vrchatTargetAddress[32];

    char oscBuffer[SEND_BUFFER_SIZE];
    tosc_bundle oscBundle;

	int Connect(int* ipAddress)
	{
        WSADATA Data;
        WSAStartup(0x202, &Data);

        if ((vrchatSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        {
            return 1;
        }
        sprintf(vrchatTargetAddress, "%d.%d.%d.%d", ipAddress[0], ipAddress[1], ipAddress[2], ipAddress[3]);

        memset(&vrchatSocketServer, 0, sizeof(vrchatSocketServer));

        vrchatSocketServer.sin_family = AF_INET;
        vrchatSocketServer.sin_port = htons(VRCHAT_SEND_PORT);
        vrchatSocketServer.sin_addr.s_addr = inet_addr(vrchatTargetAddress);

        if (connect(vrchatSocket, (struct sockaddr*)&vrchatSocketServer, sizeof(vrchatSocketServer)) == SOCKET_ERROR)
        {
            return 2;
        }

        connected = true;

        return 0;
	}

    void Disconnect()
    {
        if (!connected)
            return;

        shutdown(vrchatSocket, 2);
        closesocket(vrchatSocket);

        vrchatSocket = -1;
        connected = false;
    }

    bool IsConnected()
    {
        return connected;
    }

    char* GetAddress()
    {
        return vrchatTargetAddress;
    }

    void NewMessage()
    {
        memset(&oscBuffer, 0, sizeof(oscBuffer));

        tosc_writeBundle(&oscBundle, 0, oscBuffer, sizeof(oscBuffer));
    }

    void WritePosition(int trackerID, float x, float y, float z)
    {
        char positionAddress[32];
        sprintf(positionAddress, "/tracking/trackers/%d/position", trackerID);

        tosc_writeNextMessage(&oscBundle, positionAddress, "fff", x, y, z);
    }

    void WriteRotation(int trackerID, float x, float y, float z)
    {
        char rotationAddress[32];
        sprintf(rotationAddress, "/tracking/trackers/%d/rotation", trackerID);

        tosc_writeNextMessage(&oscBundle, rotationAddress, "fff", x, y, z);
    }

    void SendMessage()
    {
        if (connected)
            send(vrchatSocket, oscBuffer, tosc_getBundleLength(&oscBundle), 0);
    }
}