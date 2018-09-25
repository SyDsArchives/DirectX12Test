#include <iostream>

#include <Windows.h>
#include "App.h"
#include "Geometory.h"
#include "MyDirectX12.h"


LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

App::App()
{
	CoInitializeEx(0,COINITBASE_MULTITHREADED);
}


App::~App()
{
}

HWND App::GetHWND()
{
	return hwnd;
}

void App::Initialize()
{
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc;
	w.lpszClassName = ("DirectX12Test");
	w.hInstance = GetModuleHandle(0);
	RegisterClassEx(&w);

	RECT wrc = { 0,0,WindowWidth,WindowHeight };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	hwnd = CreateWindow(w.lpszClassName, ("DirectX12Test"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr, nullptr,
		w.hInstance, nullptr);
	
	//é∏îsÇµÇΩèÍçá
	if (hwnd == nullptr)
	{
		LPVOID messageBuffer = nullptr;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR)&messageBuffer,
			0,
			nullptr);

		OutputDebugString((TCHAR*)messageBuffer);
		std::cout << (TCHAR*)messageBuffer << std::endl;
		LocalFree(messageBuffer);
	}
	
}

void App::Run()
{
	dx12 = std::make_shared<MyDirectX12>(hwnd);
	
	dx12->OutLoopDx12();

	ShowWindow(hwnd, SW_SHOW);
	while (true)
	{
		dx12->InLoopDx12();
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
		{
			break;
		}
	}
	dx12->GetDevice()->Release();
}

void App::Terminate()
{
	UnregisterClass(w.lpszClassName, w.hInstance);
}
