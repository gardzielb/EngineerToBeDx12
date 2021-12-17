//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

#include "VertexPositionId.h"
#include "ReadData.h"
#include "CopyTexture.h"

extern void ExitGame() noexcept;

using namespace DirectX;

namespace DXMath = DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
	m_deviceResources = std::make_unique<DX::DeviceResources>();
	m_deviceResources->RegisterDeviceNotify(this);
	m_offscreenTexture = std::make_unique<DX::RenderTexture>(DXGI_FORMAT_B8G8R8A8_UNORM);
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

	m_deviceResources->CreateWindowSizeDependentResources();
	CreateWindowSizeDependentResources();

	auto device = m_deviceResources->GetD3DDevice();

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
void Game::Update(DX::StepTimer const& timer)
{
	PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

	float elapsedTime = float(timer.GetElapsedSeconds());

	// TODO: Add your game logic here.

	// TODO: seems to be an interesting fucking magic

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
	Clear(true);

	auto commandList = m_deviceResources->GetCommandList();

	// // TODO: Add your rendering code here.

	m_offscreenTexture->BeginScene(commandList);

	// RENDER TRIANGLE
	// m_mousePickEffect->SetWorld(DXMath::Matrix::CreateScale(2, 2, 2));
	m_mousePickEffect->Apply(commandList);

	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);
	commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->DrawIndexedInstanced(3, 1, 0, 0, 0);

	m_offscreenTexture->EndScene(commandList);

	m_deviceResources->Present();

	uint8_t* texData = nullptr;
	uint32_t dataSize = m_offscreenTexture->CopyData(m_deviceResources->GetCommandQueue(), &texData);
	if (dataSize > 0)
	{
		OutputDebugStringA("Copied texture\n");
		delete[] texData;
	}

	m_deviceResources->Prepare();
	Clear(false);
	
	// RENDER TRIANGLE
	// m_effect->SetWorld(DXMath::Matrix::CreateScale(2, 2, 2));
	// m_effect->Apply(commandList);
	m_mousePickEffect->Apply(commandList);

	m_batch->Begin(commandList);

	VertexPositionColor v1(XMFLOAT3(0.f, 1.f, 1.f), XMFLOAT4(1.f, 0.f, 0.f, 1.f));
	VertexPositionColor v2(XMFLOAT3(1.f, -1.f, 1.f), XMFLOAT4(0.f, 1.f, 0.f, 1.f));
	VertexPositionColor v3(XMFLOAT3(-1.f, -1.f, 1.f), XMFLOAT4(0.f, 0.f, 1.f, 1.f));

	m_batch->DrawTriangle(v1, v2, v3);
	m_batch->End();
	
	// Show the new frame.
	m_deviceResources->Present();

	m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());
}

// Helper method to clear the back buffers.
void Game::Clear(bool offscreen)
{
	auto commandList = m_deviceResources->GetCommandList();
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

	// Clear the views.
	auto rtvDescriptor = offscreen ? m_renderDescriptors->GetCpuHandle(OffscreenRT) : m_deviceResources->GetRenderTargetView();
	auto dsvDescriptor = m_deviceResources->GetDepthStencilView();

	commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
	const float clearColor[4] = {0, 0, 0, 0};
	commandList->ClearRenderTargetView(rtvDescriptor, clearColor, 0, nullptr);
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
void Game::GetDefaultSize(int& width, int& height) const noexcept
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

	m_renderDescriptors = std::make_unique<DescriptorHeap>(
		device,
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		RTDescriptors::RTCount
	);

	m_offscreenTexture->SetDevice(device, m_renderDescriptors->GetCpuHandle(OffscreenRT));

	RenderTargetState rtState(m_offscreenTexture->GetFormat(), m_deviceResources->GetDepthBufferFormat());
	m_mousePickEffect = std::make_unique<MousePickEffect>(device, rtState);

	RenderTargetState rtScreenState(m_deviceResources->GetBackBufferFormat(), m_deviceResources->GetDepthBufferFormat());

	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(device);

	CD3DX12_RASTERIZER_DESC rasterizerDesc(
		D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, FALSE,
		D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, TRUE,
		0, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	);

	EffectPipelineStateDescription gridPipelineDesc(
		&VertexPositionColor::InputLayout, CommonStates::Opaque, CommonStates::DepthDefault,
		rasterizerDesc, rtScreenState, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE
	);

	m_effect = std::make_unique<BasicEffect>(device, EffectFlags::VertexColor, gridPipelineDesc);

	std::vector<VertexPositionId> vertices {
		VertexPositionId(0.f, 1.f, 1.f, 120),
		VertexPositionId(1.f, -1.f, 1.f, 120),
		VertexPositionId(-1.f, -1.f, 1.f, 120)
	};

	std::vector<uint16_t> indices {0, 1, 2};

	unsigned int vertSizeBytes = 3 * sizeof(VertexPositionId);
	m_vertexBuffer = GraphicsMemory::Get(device).Allocate(vertSizeBytes);

	auto verts = reinterpret_cast<const uint8_t*>(vertices.data());
	memcpy(m_vertexBuffer.Memory(), verts, vertSizeBytes);

	unsigned int indSizeBytes = indices.size() * sizeof(uint16_t);
	m_indexBuffer = GraphicsMemory::Get(device).Allocate(indSizeBytes);

	auto ind = reinterpret_cast<const uint8_t*>(indices.data());
	memcpy(m_indexBuffer.Memory(), ind, indSizeBytes);

	m_vertexBufferView.BufferLocation = m_vertexBuffer.GpuAddress();
	m_vertexBufferView.StrideInBytes = static_cast<UINT>(sizeof(VertexPositionId));
	m_vertexBufferView.SizeInBytes = static_cast<UINT>(m_vertexBuffer.Size());

	m_indexBufferView.BufferLocation = m_indexBuffer.GpuAddress();
	m_indexBufferView.SizeInBytes = static_cast<UINT>(m_indexBuffer.Size());
	m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;

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

	m_offscreenTexture->SetWindow(m_deviceResources->GetOutputSize());
}

void Game::OnDeviceLost()
{
	// TODO: Add Direct3D resource cleanup here.
	m_graphicsMemory.reset();
}

void Game::OnDeviceRestored()
{
	CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();
}
#pragma endregion
