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

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring & functionName, const std::wstring & fileName, int lineNumber);

	std::wstring ToString() const;

	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring Filename;
	int LineNumber = -1;
};

#if DEBUG || _DEBUG
#define ThrowIfFailed(x)										    \
	{ 													    \
		HRESULT hr__ = (x);									    \
		if(FAILED(hr__)) { throw DxException(hr__, L#x, __FILEW__, __LINE__);}	    \
	}
#else
#define ThrowIfFailed(x) x
#endif // DEBUG || _DEBUG

#define ReleaseCom(p) {if(p)p->Release();p = nullptr;}