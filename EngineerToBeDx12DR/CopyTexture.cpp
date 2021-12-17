#include "pch.h"
// #include "CopyTexture.h"
//
// using namespace Microsoft::WRL;
// using namespace DirectX;
//
//
// HRESULT CaptureTexture(_In_ ID3D12Device* device,
//                        _In_ ID3D12CommandQueue* pCommandQ,
//                        _In_ ID3D12Resource* pSource,
//                        UINT64 srcPitch,
//                        const D3D12_RESOURCE_DESC& desc,
//                        ComPtr<ID3D12Resource>& pStaging,
//                        D3D12_RESOURCE_STATES beforeState,
//                        D3D12_RESOURCE_STATES afterState) noexcept
// {
// 	if (!pCommandQ || !pSource)
// 		return E_INVALIDARG;
//
// 	if (desc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D)
// 	{
// 		return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
// 	}
//
// 	if (srcPitch > UINT32_MAX)
// 		return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
//
// 	UINT numberOfPlanes = D3D12GetFormatPlaneCount(device, desc.Format);
// 	if (numberOfPlanes != 1)
// 		return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
//
// 	D3D12_HEAP_PROPERTIES sourceHeapProperties;
// 	HRESULT hr = pSource->GetHeapProperties(&sourceHeapProperties, nullptr);
// 	if (SUCCEEDED(hr) && sourceHeapProperties.Type == D3D12_HEAP_TYPE_READBACK)
// 	{
// 		// Handle case where the source is already a staging texture we can use directly
// 		pStaging = pSource;
// 		return S_OK;
// 	}
//
// 	// Create a command allocator
// 	ComPtr<ID3D12CommandAllocator> commandAlloc;
// 	hr = device->CreateCommandAllocator(
// 		D3D12_COMMAND_LIST_TYPE_DIRECT, IID_GRAPHICS_PPV_ARGS(commandAlloc.GetAddressOf())
// 	);
// 	if (FAILED(hr))
// 		return hr;
//
// 	SetDebugObjectName(commandAlloc.Get(), L"ScreenGrab");
//
// 	// Spin up a new command list
// 	ComPtr<ID3D12GraphicsCommandList> commandList;
// 	hr = device->CreateCommandList(
// 		0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAlloc.Get(), nullptr,
// 		IID_GRAPHICS_PPV_ARGS(commandList.GetAddressOf())
// 	);
// 	if (FAILED(hr))
// 		return hr;
//
// 	SetDebugObjectName(commandList.Get(), L"ScreenGrab");
//
// 	// Create a fence
// 	ComPtr<ID3D12Fence> fence;
// 	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_GRAPHICS_PPV_ARGS(fence.GetAddressOf()));
// 	if (FAILED(hr))
// 		return hr;
//
// 	SetDebugObjectName(fence.Get(), L"ScreenGrab");
//
// 	assert((srcPitch & 0xFF) == 0);
//
// 	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
// 	CD3DX12_HEAP_PROPERTIES readBackHeapProperties(D3D12_HEAP_TYPE_READBACK);
//
// 	// Readback resources must be buffers
// 	D3D12_RESOURCE_DESC bufferDesc = {};
// 	bufferDesc.DepthOrArraySize = 1;
// 	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
// 	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
// 	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
// 	bufferDesc.Height = 1;
// 	bufferDesc.Width = srcPitch * desc.Height;
// 	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
// 	bufferDesc.MipLevels = 1;
// 	bufferDesc.SampleDesc.Count = 1;
//
// 	ComPtr<ID3D12Resource> copySource(pSource);
// 	if (desc.SampleDesc.Count > 1)
// 	{
// 		// MSAA content must be resolved before being copied to a staging texture
// 		auto descCopy = desc;
// 		descCopy.SampleDesc.Count = 1;
// 		descCopy.SampleDesc.Quality = 0;
// 		descCopy.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
//
// 		ComPtr<ID3D12Resource> pTemp;
// 		hr = device->CreateCommittedResource(
// 			&defaultHeapProperties,
// 			D3D12_HEAP_FLAG_NONE,
// 			&descCopy,
// 			D3D12_RESOURCE_STATE_COPY_DEST,
// 			nullptr,
// 			IID_GRAPHICS_PPV_ARGS(pTemp.GetAddressOf())
// 		);
// 		if (FAILED(hr))
// 			return hr;
//
// 		assert(pTemp);
//
// 		SetDebugObjectName(pTemp.Get(), L"ScreenGrab temporary");
//
// 		DXGI_FORMAT fmt = desc.Format;
//
// 		D3D12_FEATURE_DATA_FORMAT_SUPPORT formatInfo = {fmt, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE};
// 		hr = device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatInfo, sizeof(formatInfo));
// 		if (FAILED(hr))
// 			return hr;
//
// 		if (!(formatInfo.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE2D))
// 			return E_FAIL;
//
// 		for (UINT item = 0; item < desc.DepthOrArraySize; ++item)
// 		{
// 			for (UINT level = 0; level < desc.MipLevels; ++level)
// 			{
// 				UINT index = D3D12CalcSubresource(level, item, 0, desc.MipLevels, desc.DepthOrArraySize);
// 				commandList->ResolveSubresource(pTemp.Get(), index, pSource, index, fmt);
// 			}
// 		}
//
// 		copySource = pTemp;
// 	}
//
// 	// Create a staging texture
// 	hr = device->CreateCommittedResource(
// 		&readBackHeapProperties,
// 		D3D12_HEAP_FLAG_NONE,
// 		&bufferDesc,
// 		D3D12_RESOURCE_STATE_COPY_DEST,
// 		nullptr,
// 		IID_GRAPHICS_PPV_ARGS(pStaging.ReleaseAndGetAddressOf())
// 	);
// 	if (FAILED(hr))
// 		return hr;
//
// 	SetDebugObjectName(pStaging.Get(), L"ScreenGrab staging");
//
// 	assert(pStaging);
//
// 	// Transition the resource if necessary
// 	TransitionResource(commandList.Get(), pSource, beforeState, D3D12_RESOURCE_STATE_COPY_SOURCE);
//
// 	// Get the copy target location
// 	D3D12_PLACED_SUBRESOURCE_FOOTPRINT bufferFootprint = {};
// 	bufferFootprint.Footprint.Width = static_cast<UINT>(desc.Width);
// 	bufferFootprint.Footprint.Height = desc.Height;
// 	bufferFootprint.Footprint.Depth = 1;
// 	bufferFootprint.Footprint.RowPitch = static_cast<UINT>(srcPitch);
// 	bufferFootprint.Footprint.Format = desc.Format;
//
// 	CD3DX12_TEXTURE_COPY_LOCATION copyDest(pStaging.Get(), bufferFootprint);
// 	CD3DX12_TEXTURE_COPY_LOCATION copySrc(copySource.Get(), 0);
//
// 	// Copy the texture
// 	commandList->CopyTextureRegion(&copyDest, 0, 0, 0, &copySrc, nullptr);
//
// 	// Transition the resource to the next state
// 	TransitionResource(commandList.Get(), pSource, D3D12_RESOURCE_STATE_COPY_SOURCE, afterState);
//
// 	hr = commandList->Close();
// 	if (FAILED(hr))
// 		return hr;
//
// 	// Execute the command list
// 	pCommandQ->ExecuteCommandLists(1, CommandListCast(commandList.GetAddressOf()));
//
// 	// Signal the fence
// 	hr = pCommandQ->Signal(fence.Get(), 1);
// 	if (FAILED(hr))
// 		return hr;
//
// 	// Block until the copy is complete
// 	while (fence->GetCompletedValue() < 1)
// 		SwitchToThread();
//
// 	return S_OK;
// }
//
// HRESULT CopyTexture(
// 	ID3D12CommandQueue* pCommandQ,
// 	ID3D12Resource* pSource,
// 	D3D12_RESOURCE_STATES beforeState,
// 	D3D12_RESOURCE_STATES afterState) noexcept
// {
// 	ComPtr<ID3D12Device> device;
// 	pCommandQ->GetDevice(IID_GRAPHICS_PPV_ARGS(device.GetAddressOf()));
//
// 	// Get the size of the image
// 	const auto desc = pSource->GetDesc();
//
// 	if (desc.Width > UINT32_MAX)
// 		return E_INVALIDARG;
//
// 	UINT64 totalResourceSize = 0;
// 	UINT64 fpRowPitch = 0;
// 	UINT fpRowCount = 0;
// 	// Get the rowcount, pitch and size of the top mip
// 	device->GetCopyableFootprints(
// 		&desc,
// 		0,
// 		1,
// 		0,
// 		nullptr,
// 		&fpRowCount,
// 		&fpRowPitch,
// 		&totalResourceSize
// 	);
//
// #if (defined(_XBOX_ONE) && defined(_TITLE)) || defined(_GAMING_XBOX)
//     // Round up the srcPitch to multiples of 1024
//     UINT64 dstRowPitch = (fpRowPitch + static_cast<uint64_t>(D3D12XBOX_TEXTURE_DATA_PITCH_ALIGNMENT) - 1u) & ~(static_cast<uint64_t>(D3D12XBOX_TEXTURE_DATA_PITCH_ALIGNMENT) - 1u);
// #else
// 	// Round up the srcPitch to multiples of 256
// 	UINT64 dstRowPitch = (fpRowPitch + 255) & ~0xFFu;
// #endif
//
// 	if (dstRowPitch > UINT32_MAX)
// 		return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
//
// 	ComPtr<ID3D12Resource> pStaging;
// 	HRESULT hr = CaptureTexture(device.Get(), pCommandQ, pSource, dstRowPitch, desc, pStaging, beforeState, afterState);
// 	if (FAILED(hr))
// 		return hr;
//
// 	// size_t rowPitch, slicePitch, rowCount;
// 	// hr = GetSurfaceInfo(static_cast<size_t>(desc.Width), desc.Height, desc.Format, &slicePitch, &rowPitch, &rowCount);
// 	// if (FAILED(hr))
// 	// return hr;
//
// 	// if (rowPitch > UINT32_MAX || slicePitch > UINT32_MAX)
// 	// return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
//
// 	// Setup pixels
// 	UINT64 imageSize = desc.Width * desc.Height * 4;
// 	uint8_t* pixels = new uint8_t[imageSize];
//
// 	// assert(fpRowCount == rowCount);
// 	// assert(fpRowPitch == rowPitch);
//
// 	// UINT64 imageSize = dstRowPitch * UINT64(rowCount);
// 	// if (imageSize > UINT32_MAX)
// 	// return HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW);
//
// 	void* pMappedMemory = nullptr;
// 	D3D12_RANGE readRange = {0, static_cast<SIZE_T>(imageSize)};
// 	D3D12_RANGE writeRange = {0, 0};
// 	hr = pStaging->Map(0, &readRange, &pMappedMemory);
// 	if (FAILED(hr))
// 		return hr;
//
// 	auto sptr = static_cast<const uint8_t*>(pMappedMemory);
// 	if (!sptr)
// 	{
// 		pStaging->Unmap(0, &writeRange);
// 		return E_POINTER;
// 	}
//
// 	memcpy(pixels, sptr, imageSize);
//
// 	// uint8_t* dptr = pixels;
//
// 	// size_t msize = std::min<size_t>(rowPitch, size_t(dstRowPitch));
// 	// for (size_t h = 0; h < rowCount; ++h)
// 	// {
// 	// memcpy(dptr, sptr, msize);
// 	// sptr += dstRowPitch;
// 	// dptr += rowPitch;
// 	// }
//
// 	pStaging->Unmap(0, &writeRange);
//
// 	return S_OK;
// }
