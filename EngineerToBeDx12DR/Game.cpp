//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx12.h"

extern void ExitGame() noexcept;

using namespace DirectX;

namespace DXMath = DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
	m_deviceResources = std::make_unique<DX::DeviceResources>();
	m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
	// IMGUI
	// --------------------------
	// Cleanup
	// ImGui_ImplDX12_Shutdown();
	// ImGui_ImplWin32_Shutdown();
	// ImGui::DestroyContext();
	// --------------------------

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

	// IMGUI
	// -----------------------------------
	// Setup Dear ImGui context
	// IMGUI_CHECKVERSION();
	// ImGui::CreateContext();
	// ImGuiIO & io = ImGui::GetIO();
	// (void)io;
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	// io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	// ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	// ImGuiStyle & style = ImGui::GetStyle();
	// if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	// {
	// 	style.WindowRounding = 0.0f;
	// 	style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	// }
	//
	// // Setup Platform/Renderer backends
	// auto srvDescHeap = m_deviceResources->GetSrvDescriptorHeap();
	// ImGui_ImplWin32_Init(window);
	// ImGui_ImplDX12_Init(
	// 	m_deviceResources->GetD3DDevice(), m_deviceResources->GetBackBufferCount(),
	// 	DXGI_FORMAT_R8G8B8A8_UNORM, srvDescHeap.Get(),
	// 	srvDescHeap->GetCPUDescriptorHandleForHeapStart(),
	// 	srvDescHeap->GetGPUDescriptorHandleForHeapStart()
	// );
	// -----------------------------------

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

	auto time = static_cast<float>(m_timer.GetTotalSeconds());

	float yaw = time * 0.4f;
	float pitch = time * 0.7f;
	float roll = time * 1.1f;

	// TODO: seems some interesting fucking magic
	auto quaternion = DXMath::Quaternion::CreateFromYawPitchRoll(pitch, yaw, roll);
	auto light = XMVector3Rotate(g_XMOne, quaternion);
	m_shapeEffect->SetLightDirection(0, light);

	// m_triangleEffect->SetLightDirection(0, light);

	m_gridModelMatrix = DXMath::Matrix::CreateRotationY(cosf(time));

	m_shapeModelMatrix = DXMath::Matrix::CreateRotationY(-cosf(time));

	// m_cupModelMatrix = DXMath::Matrix::CreateRotationZ(cosf(time) * 2.f);
	m_cupModelMatrix = DXMath::Matrix::CreateTranslation(-1.f, 0.f, 0.f);

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

	// IMGUI
	// -----------------------------------
	// PrepareImguiFrame();
	// -----------------------------------

	// Prepare the command list to render a new frame.
	m_deviceResources->Prepare();
	Clear();

	auto commandList = m_deviceResources->GetCommandList();
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

	// TODO: Add your rendering code here.

	// IMGUI
	// -----------------------------------
	// Render Dear ImGui graphics
	// commandList->SetDescriptorHeaps(1, m_deviceResources->GetSrvDescriptorHeap().GetAddressOf());
	// ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
	// -----------------------------------

	// RENDER 3D
	{
		ID3D12DescriptorHeap * heaps[] = {m_resourceDescHeap->Heap(), m_states->Heap()};
		commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

		m_shapeEffect->SetWorld(m_shapeModelMatrix);
		m_shapeEffect->Apply(commandList);
		m_shape->Draw(commandList);
	}

	// RENDER TRIANGLE
	// {
	// ID3D12DescriptorHeap * heaps[] = {m_resourceDescHeap->Heap(), m_states->Heap()};
	// commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

	// m_triangleEffect->Apply(commandList);
	// m_triangleBatch->Begin(commandList);

	// TriangleVertex v1(DXMath::Vector3(0.0f, 0.5f, 0.5f), -DXMath::Vector3::UnitZ, DXMath::Vector2(0.5f, 0));
	// TriangleVertex v2(DXMath::Vector3(0.5f, -0.5f, 0.5f), -DXMath::Vector3::UnitZ, DXMath::Vector2(1, 1));
	// TriangleVertex v3(DXMath::Vector3(-0.5f, -0.5f, 0.5f), -DXMath::Vector3::UnitZ, DXMath::Vector2(0, 1));

	// m_triangleBatch->DrawTriangle(v1, v2, v3);
	// m_triangleBatch->End();
	// }

	// RENDER CUP MODEL
	{
		ID3D12DescriptorHeap * heaps[] = {m_cupModelResources->Heap(), m_states->Heap()};
		commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);

		Model::UpdateEffectMatrices(m_cupModelEffects, m_cupModelMatrix, m_viewMatrix, m_projMatrix);
		m_cupModel->Draw(commandList, m_cupModelEffects.cbegin());
	}

	// RENDER GRID
	{
		m_gridEffect->SetWorld(m_gridModelMatrix);
		m_gridEffect->Apply(commandList);
		m_gridBatch->Begin(commandList);

		DXMath::Vector3 xAxis(2.f, 0.f, 0.f);
		DXMath::Vector3 yAxis(0.f, 0.f, 2.f);
		DXMath::Vector3 origin = DXMath::Vector3::Zero;

		constexpr size_t divisions = 20;

		for (size_t i = 0; i < divisions; i++)
		{
			float gridPercent = (float)i / (float)divisions * 2.f - 1.f;;
			auto cellX = xAxis * gridPercent + origin;
			auto cellY = yAxis * gridPercent + origin;

			GridVertex v1(cellX - yAxis, Colors::White);
			GridVertex v2(cellX + yAxis, Colors::White);
			m_gridBatch->DrawLine(v1, v2);

			GridVertex v3(cellY - xAxis, Colors::White);
			GridVertex v4(cellY + xAxis, Colors::White);
			m_gridBatch->DrawLine(v3, v4);
		}

		m_gridBatch->End();
	}

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

	// const float clear_color_with_alpha[4] = {
	// m_clearColor.x * m_clearColor.w, m_clearColor.y * m_clearColor.w, m_clearColor.z * m_clearColor.w,
	// m_clearColor.w
	// };
	// commandList->ClearRenderTargetView(rtvDescriptor, clear_color_with_alpha, 0, NULL);
	// commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);

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

	RenderTargetState renderTargetState(
		m_deviceResources->GetBackBufferFormat(), m_deviceResources->GetDepthBufferFormat()
	);

	// GRID

	m_gridBatch = std::make_unique<PrimitiveBatch<GridVertex>>(device);

	CD3DX12_RASTERIZER_DESC rasterizerDesc(
		D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE, FALSE,
		D3D12_DEFAULT_DEPTH_BIAS, D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, TRUE, FALSE, TRUE,
		0, D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	);

	EffectPipelineStateDescription gridPipelineDesc(
		&GridVertex::InputLayout, CommonStates::Opaque, CommonStates::DepthDefault,
		rasterizerDesc, renderTargetState, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE
	);

	m_gridEffect = std::make_unique<BasicEffect>(device, EffectFlags::VertexColor, gridPipelineDesc);
	m_gridModelMatrix = DXMath::Matrix::Identity;

	// 3D SHAPE

	LoadTexture(device);

	EffectPipelineStateDescription shapePipelineDesc(
		&GeometricPrimitive::VertexType::InputLayout, CommonStates::Opaque,
		CommonStates::DepthDefault, CommonStates::CullNone, renderTargetState
	);

	m_shapeEffect = std::make_unique<NormalMapEffect>(device, EffectFlags::None, shapePipelineDesc);
	m_shapeEffect->EnableDefaultLighting();

	m_states = std::make_unique<CommonStates>(device);

	auto textureGpuHandle = m_resourceDescHeap->GetGpuHandle(ResourceDescriptors::Rocks);
	m_shapeEffect->SetTexture(textureGpuHandle, m_states->AnisotropicWrap());

	auto normalMapGpuHandle = m_resourceDescHeap->GetGpuHandle(ResourceDescriptors::NormalMap);
	m_shapeEffect->SetNormalTexture(normalMapGpuHandle);

	m_shape = GeometricPrimitive::CreateTeapot();
	m_shapeModelMatrix = DXMath::Matrix::Identity;

	// CUP MODEL

	m_cupModel = Model::CreateFromSDKMESH(device, L"cup.sdkmesh");
	LoadModel(device);

	EffectPipelineStateDescription cupPipelineDesc(
		nullptr, CommonStates::Opaque, CommonStates::DepthDefault,
		CommonStates::CullClockwise, renderTargetState
	);

	m_cupModelEffects = m_cupModel->CreateEffects(*m_effectFactory, cupPipelineDesc, cupPipelineDesc);
	m_cupModelMatrix = DXMath::Matrix::Identity;

	// TRIANGLE

	// LoadTexture(device);

	// m_triangleBatch = std::make_unique<PrimitiveBatch<TriangleVertex>>(device);

	// EffectPipelineStateDescription trianglePipelineDesc(
	// &TriangleVertex::InputLayout, CommonStates::Opaque, CommonStates::DepthDefault,
	// CommonStates::CullCounterClockwise, renderTargetState
	// );

	// according to the tutorial this line is not here,
	// but the tutorial is obviously wrong
	// m_states = std::make_unique<CommonStates>(device);

	// m_triangleEffect = std::make_unique<NormalMapEffect>(device, EffectFlags::None, trianglePipelineDesc);

	// auto textureGpuHandle = m_resourceDescHeap->GetGpuHandle(ResourceDescriptors::Rocks);
	// m_triangleEffect->SetTexture(textureGpuHandle, m_states->LinearClamp());

	// auto normalMapGpuHandle = m_resourceDescHeap->GetGpuHandle(ResourceDescriptors::NormalMap);
	// m_triangleEffect->SetNormalTexture(normalMapGpuHandle);

	// m_triangleEffect->EnableDefaultLighting();
	// m_triangleEffect->SetLightDiffuseColor(0, Colors::Gray);

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

	m_gridEffect->SetView(m_viewMatrix);
	m_gridEffect->SetProjection(m_projMatrix);

	m_shapeEffect->SetView(m_viewMatrix);
	m_shapeEffect->SetProjection(m_projMatrix);
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

	DX::ThrowIfFailed(
		CreateDDSTextureFromFile(
			device, resourceUpload, L"rocks_normalmap.dds", m_normalMap.ReleaseAndGetAddressOf()
		)
	);

	CreateShaderResourceView(
		device, m_normalMap.Get(), m_resourceDescHeap->GetCpuHandle(ResourceDescriptors::NormalMap)
	);

	auto commandQueue = m_deviceResources->GetCommandQueue();
	auto uploadResourceTask = resourceUpload.End(commandQueue);
	uploadResourceTask.wait();
}

void Game::LoadModel(ID3D12Device * device)
{
	ResourceUploadBatch resourceUpload(device);
	resourceUpload.Begin();

	m_cupModelResources = m_cupModel->LoadTextures(device, resourceUpload);
	m_effectFactory = std::make_unique<EffectFactory>(m_cupModelResources->Heap(), m_states->Heap());

	auto uploadFinished = resourceUpload.End(m_deviceResources->GetCommandQueue());
	uploadFinished.wait();
}

void Game::OnDeviceLost()
{
	// TODO: Add Direct3D resource cleanup here.
	m_graphicsMemory.reset();

	m_gridEffect.reset();
	m_gridBatch.reset();

	// m_triangleEffect.reset();
	// m_triangleBatch.reset();

	m_texture.Reset();
	m_normalMap.Reset();
	m_resourceDescHeap.reset();

	m_shapeEffect.reset();
	m_shape.reset();

	m_states.reset();
	m_effectFactory.reset();
	m_cupModelResources.reset();
	m_cupModel.reset();
	m_cupModelEffects.clear();
}

void Game::OnDeviceRestored()
{
	CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();
}
#pragma endregion


// IMGUI
// -----------------------
// void Game::PrepareImguiFrame()
// {
// 	// Start the Dear ImGui frame
// 	ImGui_ImplDX12_NewFrame();
// 	ImGui_ImplWin32_NewFrame();
// 	ImGui::NewFrame();
//
// 	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
// 	if (m_showDemoWindow)
// 		ImGui::ShowDemoWindow(&m_showDemoWindow);
//
// 	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
// 	{
// 		static float f = 0.0f;
// 		static int counter = 0;
//
// 		ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.
//
// 		ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
// 		ImGui::Checkbox("Demo Window", &m_showDemoWindow); // Edit bools storing our window open/close state
// 		ImGui::Checkbox("Another Window", &m_showAnotherWindow);
//
// 		ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
// 		ImGui::ColorEdit3("clear color", (float*)&m_clearColor); // Edit 3 floats representing a color
//
// 		if (ImGui::Button("Button"))
// 			// Buttons return true when clicked (most widgets return true when edited/activated)
// 			counter++;
// 		ImGui::SameLine();
// 		ImGui::Text("counter = %d", counter);
//
// 		ImGui::Text(
// 			"Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate
// 		);
// 		ImGui::End();
// 	}
//
// 	// 3. Show another simple window.
// 	if (m_showAnotherWindow)
// 	{
// 		ImGui::Begin("Another Window", &m_showAnotherWindow);
// 		// Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
// 		ImGui::Text("Hello from another window!");
// 		if (ImGui::Button("Close Me"))
// 			m_showAnotherWindow = false;
// 		ImGui::End();
// 	}
//
// 	// Rendering
// 	ImGui::Render();
// }

// -----------------------
