#include "main.h"

#include <windows.h>

#include "UI.h"
#include "VRChatOSC.h"

bool running = true;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    UI::CreateUI();
    while (running)
    {
        VRChatOSC::NewMessage();

        VRChatOSC::WritePosition(1, 0, 0, 0);
        VRChatOSC::WriteRotation(1, 0, 0, 0);

        VRChatOSC::SendMessage();

        UI::RenderUI();
    }

    UI::DestroyUI();

    return 0;
}

void mainExit()
{
    running = false;
}