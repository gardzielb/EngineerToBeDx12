#include "pch.h"
#include "ImguiLayerBase.h"

#include "imgui.h"
#include "backends/imgui_impl_dx12.h"
#include "backends/imgui_impl_win32.h"


ImguiLayerBase::~ImguiLayerBase()
{
	Shutdown();
}

void ImguiLayerBase::OnRender(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	// Start the Dear ImGui frame
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	CreateGUI();

	// Rendering
	ImGui::Render();

	// Render Dear ImGui graphics
	commandList->SetDescriptorHeaps(1, m_srvDescriptorHeap.GetAddressOf());
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
}

void ImguiLayerBase::OnDeviceLost()
{
	m_srvDescriptorHeap.Reset();
}

void ImguiLayerBase::OnPresent(const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	// Update and Render additional Platform Windows
	if (m_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault(nullptr, (void*)commandList.Get());
	}
}

void ImguiLayerBase::OnDeviceCreated(HWND window, ID3D12Device* device, int backBufferCount, DXGI_FORMAT rtvFormat)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DX::ThrowIfFailed(
		device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_srvDescriptorHeap.ReleaseAndGetAddressOf()))
	);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	m_io = &ImGui::GetIO();
	m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	//m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
	m_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
	m_io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (m_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX12_Init(
		device, backBufferCount, rtvFormat, m_srvDescriptorHeap.Get(),
		m_srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		m_srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
	);
}

void ImguiLayerBase::Shutdown()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}