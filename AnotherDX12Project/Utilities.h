#pragma once
#include <string>
//#include <cstdio>
#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <comdef.h>
#include <vector>
#include <exception>
#include <cassert>

#if DEBUG || _DEBUG && 0
#define ThrowIfFailed(hr)\
	if (FAILED(hr))											\
	{ 													\
		_com_error err(hr); 									\
		std::wstring text = L"ERROR: ";							\
		text += err.ErrorMessage();								\
		text += __FILEW__;									\
		text += L" " + std::to_wstring(__LINE__) + L".\n";				\
		OutputDebugString(text.c_str()); 							\
		throw std::exception();									\
	}
#else
#define ThrowIfFailed(x) x
#endif // DEBUG || _DEBUG

#define ReleaseCom(p) {if(p)p->Release();p = nullptr;}