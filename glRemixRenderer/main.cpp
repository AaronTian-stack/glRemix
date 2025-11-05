#include <iostream>
#include <windows.h>
#include "imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include "rt_app.h"

namespace
{
	LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch (msg)
		{
		case WM_CLOSE:
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}
		// TODO: Somehow get this event from the host app so we can pipe events to ImGui
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
            return 1;
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
}

#include <thread>
#include <chrono>
#include <vector>
#include <conio.h>

int main()
{
	// Create test window
	// TODO: Delete this after shim/IPC is integrated
	WNDCLASS wc{};
	wc.lpfnWndProc = window_proc;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.lpszClassName = L"glRemixTestWindow";
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		0, L"glRemixTestWindow", L"glRemix",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
		nullptr, nullptr, GetModuleHandle(nullptr), nullptr
	);

	ShowWindow(hwnd, SW_SHOW);

	// Run the renderer
	glRemix::glRemixRenderer renderer;
	// TODO: Make sure to disable the debug layer on release, but we may want it in release builds for testing
	renderer.run(hwnd, true);

	DestroyWindow(hwnd);

	return 0;
}
