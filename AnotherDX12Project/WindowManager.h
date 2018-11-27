#pragma once
#include <Windows.h>
class WindowManager
{
private:
	HWND handle;
	int width, height;
public:
	WindowManager();
	~WindowManager();

	bool Init(HINSTANCE instance, int nShowCmd, int width, int height, WNDPROC wndproc);

	HWND & GetHandle();
	int GetWidth() const;
	int GetHeight() const;
};

