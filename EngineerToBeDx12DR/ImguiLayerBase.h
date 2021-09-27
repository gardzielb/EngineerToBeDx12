#pragma once
#include "imgui.h"

class ImguiLayerBase
{
public:
	ImguiLayerBase();

	void OnDeviceCreated(HWND window, ID3D12Device * device, int backBufferCount, DXGI_FORMAT rtvFormat);

	void OnRender(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> & commandList);

	void OnDeviceLost();

	void OnPresent(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> & commandList) const;

protected:
	// override this method to create UI
	virtual void CreateGUI() = 0;

	virtual ~ImguiLayerBase();

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvDescriptorHeap;
	ImGuiIO * m_io;
};
