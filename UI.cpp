#include "UI.h"

#include "math.h"
#include "time.h"

#include <windows.h>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_win32.h"

#include <GL/GL.h>

#include "main.h"
#include "VRChatOSC.h"
#include "NatNet.h"

#define CLAMP_INT(x, min, max) if (x < min) x = min; if (x > max) x = max;

namespace UI
{

    WNDCLASSEXW mainWindowClass;

    HWND mainWindow;
    HDC mainDeviceContext;

    HGLRC glContext;

    int width;
    int height;

    clock_t currentTime;

    bool CreateUI()
    {
        mainWindowClass =
        {
            sizeof(mainWindowClass),
            CS_OWNDC,
            wndProc,
            0L,
            0L,
            GetModuleHandle(NULL),
            NULL,
            NULL,
            NULL,
            NULL,
            L"VRChatOSCOptitrack",
            NULL
        };
        ::RegisterClassExW(&mainWindowClass);

        mainWindow = ::CreateWindowW(mainWindowClass.lpszClassName, L"VRChat OptiTrack Interface", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, mainWindowClass.hInstance, NULL);

        if (!createDevice(mainWindow, &mainDeviceContext))
        {
            cleanupDevice(mainWindow, mainDeviceContext);

            ::DestroyWindow(mainWindow);
            ::UnregisterClassW(mainWindowClass.lpszClassName, mainWindowClass.hInstance);
            return false;
        }

        if (!glContext)
            glContext = wglCreateContext(mainDeviceContext);

        wglMakeCurrent(mainDeviceContext, glContext);

        ::ShowWindow(mainWindow, SW_SHOWDEFAULT);
        ::UpdateWindow(mainWindow);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui::StyleColorsDark();

        ImGui_ImplWin32_InitForOpenGL(mainWindow);
        ImGui_ImplOpenGL3_Init();

        return true;
    }

    void DestroyUI()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        cleanupDevice(mainWindow, mainDeviceContext);
        wglDeleteContext(glContext);
        ::DestroyWindow(mainWindow);
        ::UnregisterClassW(mainWindowClass.lpszClassName, mainWindowClass.hInstance);
    }

    void RenderUI()
    {
        currentTime = clock();

        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                mainExit();
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        //ImGui::ShowDemoWindow();

        //ImGui::BeginMainMenuBar();

        ImGui::SetNextWindowPos({ 0, 0 });
        if (ImGui::Begin("Configuration", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            {// VRChat OSC UI Code
                if (VRChatOSC::IsConnected())
                    ImGui::PushStyleColor(ImGuiCol_Separator, (ImVec4)ImColor::HSV(0.2f / 0.7f, 0.6f, 0.6f));
                else
                    ImGui::PushStyleColor(ImGuiCol_Separator, (ImVec4)ImColor::HSV(0, 0.6f + sin(currentTime / 100.0f) * 0.2f, 0.6f + sin(currentTime / 100.0f) * 0.2f));

                ImGui::SeparatorText("VRChat OSC");

                ImGui::PopStyleColor(1);

                static int vrChatIpAddress[4] = { 127, 0, 0, 1 };
                ImGui::InputInt4("IP Address", vrChatIpAddress);

                CLAMP_INT(vrChatIpAddress[0], 0, 255);
                CLAMP_INT(vrChatIpAddress[1], 0, 255);
                CLAMP_INT(vrChatIpAddress[2], 0, 255);
                CLAMP_INT(vrChatIpAddress[3], 0, 255);

                if (VRChatOSC::IsConnected())
                {
                    if (ImGui::Button("Disconnect"))
                        VRChatOSC::Disconnect();

                    ImGui::Text("Currently Connected to:");
                    ImGui::SameLine();
                    ImGui::Text(VRChatOSC::GetAddress());
                }
                else
                {
                    if (ImGui::Button("Connect"))
                        VRChatOSC::Connect(vrChatIpAddress);

                    ImGui::Text("Currently Disconnected");
                }
            }

            {// OptiTrack Motive UI Code 
                if (NatNet::IsConnected())
                    ImGui::PushStyleColor(ImGuiCol_Separator, (ImVec4)ImColor::HSV(0.2f / 0.7f, 0.6f, 0.6f));
                else
                    ImGui::PushStyleColor(ImGuiCol_Separator, (ImVec4)ImColor::HSV(0, 0.6f + sin(currentTime / 100.0f) * 0.2f, 0.6f + sin(currentTime / 100.0f) * 0.2f));

                ImGui::SeparatorText("OptiTrack Motive");

                ImGui::PopStyleColor(1);

                static int natNetLocalAddress[4] = { 127, 0, 0, 1 };
                ImGui::InputInt4("Local Address", natNetLocalAddress);

                CLAMP_INT(natNetLocalAddress[0], 0, 255);
                CLAMP_INT(natNetLocalAddress[1], 0, 255);
                CLAMP_INT(natNetLocalAddress[2], 0, 255);
                CLAMP_INT(natNetLocalAddress[3], 0, 255);

                static int natNetServerAddress[4] = { 127, 0, 0, 1 };
                ImGui::InputInt4("Server Address", natNetServerAddress);

                CLAMP_INT(natNetServerAddress[0], 0, 255);
                CLAMP_INT(natNetServerAddress[1], 0, 255);
                CLAMP_INT(natNetServerAddress[2], 0, 255);
                CLAMP_INT(natNetServerAddress[3], 0, 255);

                const char* castMode[] = { "Multicast", "Unicast" };
                static int selectedCastMode = 0;
                ImGui::Combo("Connection Type", &selectedCastMode, castMode, 2);

                if (NatNet::IsConnected())
                {
                    if (ImGui::Button("Disconnect"))
                        NatNet::Disconnect();

                    ImGui::Text("Currently Connected to:");

                    ImGui::Text("Local:");
                    ImGui::SameLine();
                    ImGui::Text(NatNet::GetLocalAddress());

                    ImGui::Text("Server:");
                    ImGui::SameLine();
                    ImGui::Text(NatNet::GetServerAddress());

                    ImGui::Text("Connection Type:");
                    ImGui::SameLine();
                    if (NatNet::UsingMulticast())
                        ImGui::Text("Multicast");
                    else
                        ImGui::Text("Unicast");
                }
                else
                {
                    if (ImGui::Button("Connect"))
                        NatNet::Connect(natNetLocalAddress, natNetServerAddress, 1 - selectedCastMode);

                    ImGui::Text("Currently Disconnected");
                }
            }
        }
        ImVec2 configWindowSize = ImGui::GetWindowSize();

        ImGui::End();

        /*ImGui::SetCursorPosX(ImGui::GetCursorPosX() + configWindowSize.x);

        if (ImGui::BeginMenu("Tracking Info")) {

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();*/

        ImGui::Render();
        glViewport(0, 0, width, width);
        glClearColor(0.05f, 0.05f, 0.05f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        ::SwapBuffers(mainDeviceContext);
    }

    bool createDevice(HWND window, HDC* deviceContext)
    {
        *deviceContext = ::GetDC(mainWindow);

        PIXELFORMATDESCRIPTOR pixelFormatDescriptor = { 0 };

        pixelFormatDescriptor.nSize = sizeof(pixelFormatDescriptor);
        pixelFormatDescriptor.nVersion = 1;
        pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
        pixelFormatDescriptor.cColorBits = 32;

        int pixelFormat = ::ChoosePixelFormat(*deviceContext, &pixelFormatDescriptor);

        if (pixelFormat == 0)
            return false;

        if (::SetPixelFormat(*deviceContext, pixelFormat, &pixelFormatDescriptor) == FALSE)
            return false;

        ::ReleaseDC(mainWindow, *deviceContext);

        *deviceContext = ::GetDC(mainWindow);

        return true;
    }

    void cleanupDevice(HWND window, HDC deviceContext)
    {
        wglMakeCurrent(NULL, NULL);
        ::ReleaseDC(window, deviceContext);
    }

    LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return true;

        switch (msg)
        {
            case WM_SIZE:
            {
                if (wParam != SIZE_MINIMIZED)
                {
                    width = LOWORD(lParam);
                    height = HIWORD(lParam);
                }
                return 0;
            }

            case WM_SYSCOMMAND:
            {
                if ((wParam & 0xfff0) == SC_KEYMENU)
                    return 0;
                break;
            }

            case WM_DESTROY:
            {
                ::PostQuitMessage(0);
                return 0;
            }
        }

        return ::DefWindowProcW(hWnd, msg, wParam, lParam);
    }

}