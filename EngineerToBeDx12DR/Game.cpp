//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

// #include "imgui.h"
// #include "backends/imgui_impl_win32.h"
// #include "backends/imgui_impl_dx12.h"

extern void ExitGame() noexcept;

using namespace DirectX;

namespace dxmath = DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
	: m_imguiLayer(true, true, ImVec4(0.45f, 0.55f, 0.60f, 1.00f))
{
	m_deviceResources = std::make_unique<DX::DeviceResources>();
	m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
	if (m_deviceResources)
	{
		m_deviceResources->WaitForGpu();
	}
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
	m_deviceResources->SetWindow(window, width, height);

	m_deviceResources->CreateDeviceResources();
	CreateDeviceDependentResources();

	auto device = m_deviceResources->GetD3DDevice();
	m_imguiLayer.OnDeviceCreated(
		window, device, m_deviceResources->GetBackBufferCount(), m_deviceResources->GetBackBufferFormat()
	);

	m_deviceResources->CreateWindowSizeDependentResources();
	CreateWindowSizeDependentResources();

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
	m_timer.Tick(
		[&]()
		{
			Update(m_timer);
		}
	);

	Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const & timer)
{
	PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

	float elapsedTime = float(timer.GetElapsedSeconds());

	// TODO: Add your game logic here.
	elapsedTime;

	PIXEndEvent();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return;
	}

	// Prepare the command list to render a new frame.
	m_deviceResources->Prepare();
	Clear();

	auto commandList = m_deviceResources->GetCommandList();
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

	// TODO: Add your rendering code here.

	ID3D12DescriptorHeap * heaps[] = {m_resourceDescHeap->Heap(), m_states->Heap()};
	commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

	m_effect->Apply(commandList);
	m_primitiveBatch->Begin(commandList);

	Vertex v1(dxmath::Vector3(400.f, 150.f, 0.f), dxmath::Vector2(0.5f, 0));
	Vertex v2(dxmath::Vector3(600.f, 450.f, 0.f), dxmath::Vector2(1, 1));
	Vertex v3(dxmath::Vector3(200.f, 450.f, 0.f), dxmath::Vector2(0, 1));

	m_primitiveBatch->DrawTriangle(v1, v2, v3);
	m_primitiveBatch->End();

	m_imguiLayer.OnRender(commandList);

	PIXEndEvent(commandList);

	// Show the new frame.
	PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");

	m_imguiLayer.OnPresent(commandList);
	
	m_deviceResources->Present();
	m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());

	PIXEndEvent();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
	auto commandList = m_deviceResources->GetCommandList();
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

	// Clear the views.
	auto rtvDescriptor = m_deviceResources->GetRenderTargetView();
	auto dsvDescriptor = m_deviceResources->GetDepthStencilView();

	commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
	commandList->ClearRenderTargetView(rtvDescriptor, Colors::CornflowerBlue, 0, nullptr);
	commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// Set the viewport and scissor rect.
	auto viewport = m_deviceResources->GetScreenViewport();
	auto scissorRect = m_deviceResources->GetScissorRect();
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	PIXEndEvent(commandList);
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
	// TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
	// TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
	// TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
	m_timer.ResetElapsedTime();

	// TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
	auto r = m_deviceResources->GetOutputSize();
	m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
	// spdlog::debug("Window resized: {} x {}", height, width);
	std::cout << "Window resized\n";

	if (!m_deviceResources->WindowSizeChanged(width, height))
		return;

	CreateWindowSizeDependentResources();

	// TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int & width, int & height) const noexcept
{
	// TODO: Change to desired default window size (note minimum size is 320x200).
	width = 800;
	height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
	auto device = m_deviceResources->GetD3DDevice();

	m_graphicsMemory = std::make_unique<GraphicsMemory>(device);

	m_primitiveBatch = std::make_unique<PrimitiveBatch<Vertex>>(device);

	RenderTargetState renderTargetState(
		m_deviceResources->GetBackBufferFormat(),
		m_deviceResources->GetDepthBufferFormat()
	);

	EffectPipelineStateDescription pipelineDesc(
		&Vertex::InputLayout, CommonStates::Opaque, CommonStates::DepthDefault,
		CommonStates::CullCounterClockwise, renderTargetState
	);

	LoadTexture(device);

	// according to the tutorial this line is not here,
	// but the tutorial is obviously wrong
	m_states = std::make_unique<CommonStates>(device);

	m_effect = std::make_unique<BasicEffect>(device, EffectFlags::Texture, pipelineDesc);
	auto gpuHandle = m_resourceDescHeap->GetGpuHandle(ResourceDescriptors::Rocks);
	m_effect->SetTexture(gpuHandle, m_states->LinearClamp());

	// Check Shader Model 6 support
	D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {D3D_SHADER_MODEL_6_0};
	if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
		|| (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0))
	{
#ifdef _DEBUG
		OutputDebugStringA("ERROR: Shader Model 6.0 is not supported!\n");
#endif
		throw std::runtime_error("Shader Model 6.0 is not supported!");
	}

	// TODO: Initialize device dependent objects here (independent of window size).
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
	// TODO: Initialize windows-size dependent objects here.		
	auto size = m_deviceResources->GetOutputSize();

	dxmath::Matrix proj = dxmath::Matrix::CreateScale(
			2.f / float(size.right),
			-2.f / float(size.bottom), 1.f
		)
		* dxmath::Matrix::CreateTranslation(-1.f, 1.f, 0.f);
	m_effect->SetProjection(proj);
}

void Game::LoadTexture(ID3D12Device * device)
{
	m_resourceDescHeap = std::make_unique<DescriptorHeap>(device, ResourceDescriptors::Count);

	ResourceUploadBatch resourceUpload(device);
	resourceUpload.Begin();

	DX::ThrowIfFailed(
		CreateWICTextureFromFile(
			device, resourceUpload, L"kiti.jpg", m_texture.ReleaseAndGetAddressOf()
		)
	);

	CreateShaderResourceView(
		device, m_texture.Get(), m_resourceDescHeap->GetCpuHandle(ResourceDescriptors::Rocks)
	);

	auto commandQueue = m_deviceResources->GetCommandQueue();
	auto uploadResourceTask = resourceUpload.End(commandQueue);
	uploadResourceTask.wait();
}

void Game::OnDeviceLost()
{
	// TODO: Add Direct3D resource cleanup here.
	m_graphicsMemory.reset();

	m_effect.reset();
	m_primitiveBatch.reset();

	m_texture.Reset();
	m_resourceDescHeap.reset();

	m_imguiLayer.OnDeviceLost();
}

void Game::OnDeviceRestored()
{
	CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();
}
#pragma endregion
