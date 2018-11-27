#include <stdio.h>
#include "WindowManager.h"
#include "Utilities.h"

WindowManager::WindowManager()
{
}


WindowManager::~WindowManager()
{
}

bool WindowManager::Init(HINSTANCE instance, int nShowCmd, int width, int height, WNDPROC wndproc)
{
	this->width = width;
	this->height = height;

	WNDCLASSEX wndClass;
	ZeroMemory(&wndClass, sizeof(WNDCLASSEX));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = wndproc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = instance;
	wndClass.hIcon = LoadIcon(instance, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(0, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName = 0;
	wndClass.lpszClassName = L"wndClass";
	wndClass.hIconSm = 0;

	if (!RegisterClassEx(&wndClass))
	{
		puts("Failed to register window class.");
		return false;
	}

	handle = CreateWindow("wndClass", "D3D12 Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, instance, 0);
	if (!handle)
	{
		puts("Failed to register window class");
		return false;
	}

	ShowWindow(handle, nShowCmd);
	UpdateWindow(handle);

	return true;
}

HWND & WindowManager::GetHandle()
{
	return handle;
}

int WindowManager::GetWidth() const
{
	return width;
}

int WindowManager::GetHeight() const
{
	return height;
}
