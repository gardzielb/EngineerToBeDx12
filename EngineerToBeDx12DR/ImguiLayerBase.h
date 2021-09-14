#pragma once
#include "imgui.h"

class ImguiLayerBase
{
public:
	ImguiLayerBase() = default;
	
	void OnDeviceCreated(HWND window, ID3D12Device* device, int backBufferCount, DXGI_FORMAT rtvFormat);

	void OnRender(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList);

	void OnDeviceLost();

	void OnPresent(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> & commandList);

protected:
	virtual void CreateGUI() = 0;

	virtual ~ImguiLayerBase();

private:
	void Shutdown();

	// members
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvDescriptorHeap;
	ImGuiIO * m_io;
};
