#include <iostream>
#include <windows.h>

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
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
}

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
		0, L"glRemixTestWindow", L"glRemix Renderer - Test",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
		nullptr, nullptr, GetModuleHandle(nullptr), nullptr
	);

	ShowWindow(hwnd, SW_SHOW);

	// Run the renderer
	glremix::glRemixRenderer renderer;
	renderer.run(hwnd, true);

	DestroyWindow(hwnd);

	return 0;
}
