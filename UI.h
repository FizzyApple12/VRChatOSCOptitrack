#pragma once

#ifndef UI_H
#define UI_H

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_win32.h"

#include <windows.h>

#include "NatNet.h"

namespace UI
{
	bool CreateUI();
	void DestroyUI();

	void RenderEnvironment();
	void RenderMarker(NatNet::Marker marker);
	void RenderRigidBody(NatNet::RigidBody rigidBody);
	void RenderUI();

	void createTrackerSelecter(const char* label, int oscId);
	
	bool createDevice(HWND window, HDC* deviceContext);
	void cleanupDevice(HWND window, HDC deviceContext);
	
	LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#endif

