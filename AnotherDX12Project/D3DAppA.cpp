#include "D3DAppA.h"
#include <assert.h>
//

D3D12AppA::D3D12AppA()
{
}


D3D12AppA::~D3D12AppA()
{

}

bool D3D12AppA::InitD3D()
{
	// Create Device
	//=====================================================================================================
#if DEBUG || _DEBUG
	ComPtr<ID3D12Debug3> debugController;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();
#endif // DEBUG || _DEBUG

	// Create Device
	ThrowIfFailed(D3D12CreateDevice(0, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));
	// try to create hardware device
	HRESULT hardwareResult = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
	// Fallback to WARP
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
	}
	
	// CreateFence
	ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	//=====================================================================================================





	// Get Discriptor Sizes
	//=====================================================================================================
	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	cbvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//=====================================================================================================





	// Check MSAA 4x quality support
	//=====================================================================================================
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = backBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels)));

	msaa4xQuality = msQualityLevels.NumQualityLevels;
	assert(msaa4xQuality > 0 && "Unexpected MSAA Quality Level.");
	//=====================================================================================================
	




	// Create command objects and return
	//=====================================================================================================
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(device->CreateCommandQueue(
		&queueDesc, 
		IID_PPV_ARGS(&commandQueue)));
	ThrowIfFailed(device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, 
		IID_PPV_ARGS(&commandAllocator)));
	ThrowIfFailed(device->CreateCommandList(
		0, 
		D3D12_COMMAND_LIST_TYPE_DIRECT, 
		commandAllocator.Get(), 
		nullptr, 
		IID_PPV_ARGS(&commandList)));
	commandList->Close();
	//=====================================================================================================



	return true;
}

bool D3D12AppA::OnResize(HWND window, UINT width, UINT height)
{
	// Re-create Swap Chain
	//=====================================================================================================
	swapChain->Release();
	// Describe the swap chain we want to create
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	swapChainDesc.SampleDesc.Count = enableMSAA ? 4 : 1;
	swapChainDesc.SampleDesc.Quality = enableMSAA ? (msaa4xQuality - 1) : 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = swapChainBufferCount;
	swapChainDesc.OutputWindow = window;
	swapChainDesc.Windowed = false;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Create it using the factory created earlier
	ThrowIfFailed(dxgiFactory->CreateSwapChain(
		commandQueue.Get(), 
		&swapChainDesc, 
		swapChain.GetAddressOf()));
	//=====================================================================================================





	// Create Descriptor Heaps for RTV and DSV
	//=====================================================================================================
	// Create the heaps for the two resources
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = swapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap)));
	//=====================================================================================================





	// Create RTV to both buffers in swap Chain
	//=====================================================================================================
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < swapChainBufferCount; i++)
	{
		// get ith buffer in swap chain
		ThrowIfFailed(swapChain->GetBuffer(
			i, 
			IID_PPV_ARGS(&swapChainBuffers[i])));

		device->CreateRenderTargetView(
			swapChainBuffers[i].Get(), 
			nullptr, 
			rtvHeapHandle);

		rtvHeapHandle.Offset(1, rtvDescriptorSize);
	}
	//=====================================================================================================




	// Create Depth Stencil Buffer view
	//=====================================================================================================
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = depthStencilFormat;
	depthStencilDesc.SampleDesc.Count = enableMSAA ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = enableMSAA ? (msaa4xQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = depthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), 
		D3D12_HEAP_FLAG_NONE, 
		&depthStencilDesc, 
		D3D12_RESOURCE_STATE_COMMON, 
		&optClear, 
		IID_PPV_ARGS(depthStencilBuffer.GetAddressOf())));

	device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, DepthStencilView());

	commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(depthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_DEPTH_WRITE));
	//=====================================================================================================





	// Set Viewports
	//=====================================================================================================
	D3D12_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(width);
	vp.Height = static_cast<float>(height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	// needs to be reset when the command list is reset
	commandList->RSSetViewports(1, &vp);
	//=====================================================================================================
	return true;
}

//bool D3D12Renderer::CreateCommandObjects()
//{
//	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
//	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
//	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
//	RetIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
//
//	RetIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
//	RetIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
//
//	commandList->Close();
//
//	return true;
//}

//bool D3D12Renderer::CreateSwapChain(HWND window, UINT width, UINT height)
//{
//	swapChain->Release();
//	// Describe the swap chain we want to create
//	DXGI_SWAP_CHAIN_DESC swapChainDesc;
//	swapChainDesc.BufferDesc.Width = width;
//	swapChainDesc.BufferDesc.Height = height;
//	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
//	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
//	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
//	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
//	swapChainDesc.SampleDesc.Count = enableMSAA ? 4 : 1;
//	swapChainDesc.SampleDesc.Quality = enableMSAA ? msaa4xQuality - 1 : 0;
//	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//	swapChainDesc.BufferCount = swapChainBufferCount;
//	swapChainDesc.OutputWindow = window;
//	swapChainDesc.Windowed = false;
//	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
//	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
//
//	// Create it using the factory created earlier
//	RetIfFailed(dxgiFactory->CreateSwapChain(commandQueue.Get(), &swapChainDesc, swapChain.GetAddressOf()));
//
//	return true;
//}

//bool D3D12Renderer::CreateRtvAndDsvDescriptorHeaps()
//{
//	// Create the heaps for the two resources
//	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
//	rtvHeapDesc.NumDescriptors = swapChainBufferCount;
//	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
//	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//	rtvHeapDesc.NodeMask = 0;
//	RetIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));
//
//	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
//	dsvHeapDesc.NumDescriptors = 1;
//	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
//	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//	dsvHeapDesc.NodeMask = 0;
//	RetIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap)));
//
//	// Create RTV to both buffers in swap Chain
//	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
//	for (UINT i = 0; i < swapChainBufferCount; i++)
//	{
//		// get ith buffer in swap chain
//		RetIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&swapChainBuffers[i])));
//
//		device->CreateRenderTargetView(swapChainBuffers[i].Get(), nullptr, rtvHeapHandle);
//
//		rtvHeapHandle.Offset(1, rtvDescriptorSize);
//	}
//
//	// Create DSBuffer and DSV
//
//	return true;
//}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12AppA::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap->GetCPUDescriptorHandleForHeapStart(), currentBackBuffer, rtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12AppA::DepthStencilView() const
{
	return dsvHeap->GetCPUDescriptorHandleForHeapStart();
}

//ComPtr<const ID3D12Device5> D3D12Renderer::GetDevice()
//{
//	return device;
//}
