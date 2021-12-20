//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

#include "MeshVertex.h"

extern void ExitGame() noexcept;

using namespace DirectX;

namespace DXMath = DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
	: m_vertexBufferView {}, m_indexBufferView {}
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
void Game::Update(DX::StepTimer const& timer)
{
	PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

	float elapsedTime = float(timer.GetElapsedSeconds());

	// TODO: Add your game logic here.

	auto time = static_cast<float>(m_timer.GetTotalSeconds());
	m_modelMatrix = DXMath::Matrix::CreateRotationY(cosf(time));

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

	auto& currentEffect = m_customEffect;

	currentEffect->SetWorld(m_modelMatrix);
	currentEffect->SetView(m_viewMatrix);
	currentEffect->SetProjection(m_projMatrix);
	currentEffect->Apply(commandList);

	commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	commandList->IASetIndexBuffer(&m_indexBufferView);
	commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawIndexedInstanced(3, 1, 0, 0, 0);

	PIXEndEvent(commandList);

	// Show the new frame.
	PIXBeginEvent(m_deviceResources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present");
	m_deviceResources->Present();

	m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());
	PIXEndEvent(m_deviceResources->GetCommandQueue());
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

	RenderTargetState renderTargetState(
		m_deviceResources->GetBackBufferFormat(), m_deviceResources->GetDepthBufferFormat()
	);

	m_customEffect = std::make_unique<MousePickEffect>(device, renderTargetState);

	CD3DX12_RASTERIZER_DESC rasterizerDesc(
		D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, FALSE,
		D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, TRUE,
		0, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	);

	EffectPipelineStateDescription pipelineDesc(
		&MeshVertex::InputLayout, CommonStates::Opaque, CommonStates::DepthDefault, rasterizerDesc, renderTargetState
	);

	// m_basicEffect = std::make_unique<BasicEffect>(device, EffectFlags::None, pipelineDesc);
	// m_basicEffect->SetDiffuseColor(Colors::White);

	std::vector<MeshVertex> vertices {
		MeshVertex(-0.5f, 0.5f, 0.5f, 1),
		MeshVertex(0.5f, 0.5f, 0.5f, 1),
		MeshVertex(0.0f, -0.5f, 0.5f, 1)
	};

	std::vector<uint16_t> indices {0, 1, 2};

	// Vertex data
	unsigned int vertSizeBytes = vertices.size() * sizeof(MeshVertex);
	m_vertexBuffer = GraphicsMemory::Get(device).Allocate(vertSizeBytes);

	auto verts = reinterpret_cast<const uint8_t*>(vertices.data());
	memcpy(m_vertexBuffer.Memory(), verts, vertSizeBytes);

	// Index data
	unsigned int indSizeBytes = indices.size() * sizeof(uint16_t);
	m_indexBuffer = GraphicsMemory::Get(device).Allocate(indSizeBytes);

	auto ind = reinterpret_cast<const uint8_t*>(indices.data());
	memcpy(m_indexBuffer.Memory(), ind, indSizeBytes);

	m_vertexBufferView.BufferLocation = m_vertexBuffer.GpuAddress();
	m_vertexBufferView.StrideInBytes = static_cast<UINT>(sizeof(MeshVertex));
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

	auto windowSize = m_deviceResources->GetOutputSize();

	m_viewMatrix = DXMath::Matrix::CreateLookAt(
		DXMath::Vector3(2.f, 2.f, 2.f), DXMath::Vector3::Zero, DXMath::Vector3::UnitY
	);

	auto aspectRatio = static_cast<float>(windowSize.right) / static_cast<float>(windowSize.bottom);
	m_projMatrix = DXMath::Matrix::CreatePerspectiveFieldOfView(XM_PI / 4.f, aspectRatio, 0.1f, 10.f);
}

void Game::OnDeviceLost()
{
	// TODO: Add Direct3D resource cleanup here.
	m_graphicsMemory.reset();
	m_customEffect.reset();
	m_basicEffect.reset();
}

void Game::OnDeviceRestored()
{
	CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();
}

#pragma endregion
