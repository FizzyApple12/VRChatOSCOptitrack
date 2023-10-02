#include "UI.h"

#include "math.h"
#include "time.h"

#include <windows.h>

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_win32.h"

#include <GL/GL.h>
#include <GL/glu.h>

#include "main.h"
#include "VRChatOSC.h"
#include "NatNet.h"
#include <stdio.h>
#include "NatNetCollections.h"
#include "DrawingFunctions.h"

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

    int selectedTrackerId = 0;

    float cameraPositionX = 0, cameraPositionY = -1, cameraPositionZ = -5;
    float cameraRotationX = 0, cameraRotationY = 0;

    bool enableFlyControl = false;

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
        ImGuiIO& io = ImGui::GetIO(); (void)io;
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

        ReleaseDC(mainWindow, mainDeviceContext);
    }

    void RenderEnvironment()
    {
        glViewport(0, 0, width, width);
        glClearColor(0.05f, 0.05f, 0.05f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        {
            glLoadIdentity();

            GLfloat glfLightPos[] = { -4.0f, 4.0f, 4.0f, 0.0f };
            GLfloat glfLightAmb[] = { .3f, .3f, .3f, 1.0f };

            glLightfv(GL_LIGHT0, GL_AMBIENT, glfLightAmb);
            glLightfv(GL_LIGHT1, GL_POSITION, glfLightPos);

            glEnable(GL_COLOR_MATERIAL);

            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

            glPushMatrix();
        }

        {
            glRotatef(cameraRotationX, 1, 0, 0);
            glRotatef(cameraRotationY, 0, 1, 0);
            glTranslatef(cameraPositionX * 1000, cameraPositionY * 1000, cameraPositionZ * 1000);
        }

        {
            glLineWidth(3.0f);
            glBegin(GL_LINES);

            glColor3f(.8f, 0.0f, 0.0f);
            glVertex3f(0, 0, 0);
            glVertex3f(300, 0, 0);

            glColor3f(0.0f, .8f, 0.0f);
            glVertex3f(0, 0, 0);
            glVertex3f(0, 300, 0);

            glColor3f(0.0f, 0.0f, .8f);
            glVertex3f(0, 0, 0);
            glVertex3f(0, 0, 300);

            glEnd();
        }

        {
            glLineWidth(1.0f);

            glPushAttrib(GL_ALL_ATTRIB_BITS);
            glPushMatrix();

            float halfSize = 2000.0f;      // world is in mms - set to 2 cubic meters
            float step = 100.0f;           // line every .1 meter
            float major = 200.0f;          // major every .2 meters
            float yloc = 0.0f;

            glEnable(GL_LINE_STIPPLE);
            glLineWidth(0.25);
            glDepthMask(true);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_ALWAYS);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            glEnable(GL_COLOR_MATERIAL);

            float r, g, b, a;
            r = g = b = a = 0.7f;

            for (float x = -halfSize; x <= halfSize; x += step)
            {
                if ((x == 0) || (x == -halfSize) || (x == halfSize))         // edge or center line
                {
                    glColor4f(.76f * r, .76f * g, .76f * b, .76f * a);
                }
                else
                {
                    float ff = fmod(x, major);
                    if (ff == 0.0f)                                        // major line
                    {
                        glColor4f(.55f * r, 0.55f * g, 0.55f * b, 0.55f * a);
                    }
                    else                                                // minor line
                    {
                        glColor4f(0.3f * r, 0.3f * g, 0.3f * b, 0.3f * a);
                    }
                }

                glBegin(GL_LINES);
                glVertex3f(x, 0, halfSize);	    // vertical
                glVertex3f(x, 0, -halfSize);
                glVertex3f(halfSize, 0, x);     // horizontal
                glVertex3f(-halfSize, 0, x);
                glEnd();

            }

            glPopAttrib();
            glPopMatrix();
        }
    }

    void RenderMarker(NatNet::Marker marker)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPushMatrix();

        float x, y, z;
        std::tie(x, y, z) = marker.position;

        glTranslatef(x * 1000.0f, y * 1000.0f, z * 1000.0f);

        DrawingFunctions::DrawSphere(1, 5.0f);

        glPopMatrix();
        glPopAttrib();
    }

    void RenderRigidBody(NatNet::RigidBody rigidBody)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPushMatrix();

        float x, y, z;
        std::tie(x, y, z) = rigidBody.position;

        glTranslatef(x * 1000.0f, y * 1000.0f, z * 1000.0f);

        std::tie(x, y, z) = rigidBody.rotation;

        glRotatef(z, 0.0f, 0.0f, 1.0f);
        glRotatef(x, 1.0f, 0.0f, 0.0f);
        glRotatef(y, 0.0f, 1.0f, 0.0f);

        bool isActive = false;

        for (int i = 0; i < 9; i++)
        {
            if (rigidBody.id == getOSCTrackerNumber(i))
                isActive = true;
        }

        DrawingFunctions::DrawCube(50.0f, isActive, rigidBody.id == selectedTrackerId);

        glPopMatrix();
        glPopAttrib();
    }

    void createTrackerSelecter(const char* label, int oscId)
    {
        int optitrackId = getOSCTrackerNumber(oscId);

        ImGui::PushID("intinput", label);

        if (ImGui::InputInt("", &optitrackId))
        {
            setOSCTrackerNumber(oscId, optitrackId);
            selectedTrackerId = optitrackId;
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TRACKER_ID"))
                setOSCTrackerNumber(oscId, *(const int*) payload->Data);

            ImGui::EndDragDropTarget();
        }

        ImGui::PopID();

        ImGui::SameLine();

        if (ImGui::Selectable(label, selectedTrackerId == optitrackId))
            selectedTrackerId = optitrackId;
    }

    void RenderUI()
    {
        clock_t nextTime = clock();

        clock_t deltaTime = nextTime - currentTime;

        currentTime = nextTime;

        static float lookSensitivity = 2.0f, moveSensitivity = 1.0f;

        if (enableFlyControl)
        {
            RECT viewportRect;
            GetWindowRect(mainWindow, &viewportRect);

            float centerX = viewportRect.left + ((viewportRect.right - viewportRect.left) / 2.0f);
            float centerY = viewportRect.top + ((viewportRect.bottom - viewportRect.top) / 2.0f);

            POINT cursorPosition = { centerX, centerY };
            GetCursorPos(&cursorPosition);

            float deltaX = cursorPosition.x - centerX;
            float deltaY = cursorPosition.y - centerY;

            SetCursorPos(centerX, centerY);

            cameraRotationY += deltaX * lookSensitivity * ((float) deltaTime / (float) CLOCKS_PER_SEC);
            cameraRotationX += deltaY * lookSensitivity * ((float) deltaTime / (float)CLOCKS_PER_SEC);
        }

        MSG message;
        while (::PeekMessage(&message, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&message);
            ::DispatchMessage(&message);
            if (message.message == WM_QUIT)
                mainExit();
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos({ 0, 0 });
        ImGui::SetNextWindowCollapsed(false, ImGuiCond_Once);
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

                    ImGui::Text("Currently Connected to: %s", VRChatOSC::GetAddress());
                }
                else
                {
                    ImGui::PushID("VRChatConnect");

                    if (ImGui::Button("Connect"))
                        VRChatOSC::Connect(vrChatIpAddress);

                    ImGui::PopID();

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
                    ImGui::Text("Local: %s", NatNet::GetLocalAddress());
                    ImGui::Text("Server: %s", NatNet::GetServerAddress());
                    ImGui::Text("Connection Type: %s", NatNet::UsingMulticast() ? "Multicast" : "Unicast");
                }
                else
                {
                    ImGui::PushID("OptiTrackConnect");

                    if (ImGui::Button("Connect"))
                        NatNet::Connect(natNetLocalAddress, natNetServerAddress, 1 - selectedCastMode);

                    ImGui::PopID();

                    ImGui::Text("Currently Disconnected");
                }
            }
        }
        ImVec2 configWindowSize = ImGui::GetWindowSize();

        ImGui::End();

        ImGui::SetNextWindowPos({ 0, configWindowSize.y });
        ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
        if (ImGui::Begin("Tracking Setup", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            createTrackerSelecter("Head", 0);

            createTrackerSelecter("Hips", 1);
            createTrackerSelecter("Chest", 2);

            createTrackerSelecter("Left Foot", 3);
            createTrackerSelecter("Right Foot", 4);

            createTrackerSelecter("Left Knee", 5);
            createTrackerSelecter("Right Knee", 6);

            createTrackerSelecter("Left Elbow", 7);
            createTrackerSelecter("Right Elbow", 8);

            ImGui::SeparatorText("Detected Trackers");

            ImGui::PushID("TrackerListBox");
            if (ImGui::BeginListBox(""))
            {
                for (int i = 0; i < NatNetRigidBodyCollection::GetCount(); i++)
                {
                    ImGui::PushID(i);

                    NatNet::RigidBody activeRigidBody = NatNetRigidBodyCollection::Get(i);
                    char rigidBodyName[64];

                    sprintf(rigidBodyName, "(%03d) %s", activeRigidBody.id, activeRigidBody.name);

                    if (ImGui::Selectable(rigidBodyName, selectedTrackerId == activeRigidBody.id))
                        selectedTrackerId = activeRigidBody.id;

                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        ImGui::SetDragDropPayload("TRACKER_ID", &activeRigidBody.id, sizeof(int));

                        ImGui::Text(rigidBodyName);
                        ImGui::EndDragDropSource();
                    }

                    ImGui::PopID();
                }
                ImGui::EndListBox();
            }
            ImGui::PopID();
        }

        ImVec2 setupWindowSize = ImGui::GetWindowSize();

        ImGui::End();

        ImGui::SetNextWindowPos({ 0, configWindowSize.y + setupWindowSize.y });
        ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
        if (ImGui::Begin("View", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (ImGui::Button("Reset View Point"))
            {
                cameraPositionX = 0, cameraPositionY = -1, cameraPositionZ = -5;
                cameraRotationX = 0, cameraRotationY = 0;
            }

            ImGui::SliderFloat("Look Sensitivity", &lookSensitivity, 0.1f, 5.0f);
            ImGui::SliderFloat("Move Sensitivity", &moveSensitivity, 0.1f, 1.0f);
        }

        ImGui::End();

        ImGui::Render();

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

                    if (hWnd != nullptr)
                    {
                        wglMakeCurrent(mainDeviceContext, glContext);

                        RECT viewportRect;
                        GetWindowRect(mainWindow, &viewportRect);

                        float aspectRatio = ((float)(viewportRect.right - viewportRect.left)) / ((float)(viewportRect.bottom - viewportRect.top));

                        glMatrixMode(GL_PROJECTION);

                        glLoadIdentity();

                        gluPerspective(40.0f, aspectRatio, 1.0f, 10000.0f);
                        glViewport(viewportRect.left, viewportRect.top, viewportRect.right - viewportRect.left, viewportRect.bottom - viewportRect.top);

                        glMatrixMode(GL_MODELVIEW);
                    }
                }
                return 0;
            }

            case WM_RBUTTONDOWN:
            {
                enableFlyControl = true;

                RECT viewportRect;
                GetWindowRect(mainWindow, &viewportRect);

                ShowCursor(false);

                SetCursorPos(viewportRect.left + ((viewportRect.right - viewportRect.left) / 2.0f), viewportRect.top + ((viewportRect.bottom - viewportRect.top) / 2.0f));

                break;
            }

            case WM_RBUTTONUP:
            {
                enableFlyControl = false;

                ShowCursor(true);

                break;
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