#pragma once
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <comdef.h>
#include "Utilities.h"
#include <wrl.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using Microsoft::WRL::ComPtr;

class D3D12AppA
{
private:
	DXGI_FORMAT backBufferFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	bool enableMSAA = true;
	UINT msaa4xQuality;
	static const UINT swapChainBufferCount = 2;

	int currentBackBuffer = 0;

	ComPtr<ID3D12Device5> device;
	ComPtr<IDXGIFactory7> dxgiFactory;
	ComPtr<ID3D12Fence1> fence;

	// Resized with the window
	ComPtr<IDXGISwapChain> swapChain;
	// Descriptor Heaps (like views in D3D11)
	ComPtr<ID3D12DescriptorHeap> rtvHeap;
	ComPtr<ID3D12DescriptorHeap> dsvHeap;
	ComPtr<ID3D12Resource> swapChainBuffers[swapChainBufferCount];
	ComPtr<ID3D12Resource> depthStencilBuffer;

	// Command Objects
	ComPtr<ID3D12CommandQueue> commandQueue;
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ComPtr<ID3D12GraphicsCommandList4> commandList;

	// Descriptor size Cached values
	UINT rtvDescriptorSize;
	UINT dsvDescriptorSize;
	UINT cbvDescriptorSize;
public:
	D3D12AppA();
	~D3D12AppA();

	bool InitD3D();
	bool OnResize(HWND window, UINT width, UINT height);
private:
	bool CreateCommandObjects();

	//bool CreateSwapChain(HWND window, UINT width, UINT height);
	//bool CreateRtvAndDsvDescriptorHeaps();

	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;

	//ComPtr<const ID3D12Device5> GetDevice();
};

