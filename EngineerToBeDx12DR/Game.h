//
// Game.h
//

#pragma once

#include <imgui.h>

#include "DeviceResources.h"
#include "StepTimer.h"


// A basic game implementation that creates a D3D12 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:
	Game() noexcept(false);
	~Game();

	Game(Game &&) = default;
	Game & operator=(Game &&) = default;

	Game(Game const &) = delete;
	Game & operator=(Game const &) = delete;

	// Initialization and management
	void Initialize(HWND window, int width, int height);

	// Basic game loop
	void Tick();

	// IDeviceNotify
	void OnDeviceLost() override;
	void OnDeviceRestored() override;

	// Messages
	void OnActivated();
	void OnDeactivated();
	void OnSuspending();
	void OnResuming();
	void OnWindowMoved();
	void OnWindowSizeChanged(int width, int height);

	// Properties
	void GetDefaultSize(int & width, int & height) const noexcept;

private:
	void Update(DX::StepTimer const & timer);
	void Render();

	void Clear();

	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();

	void LoadTexture(ID3D12Device* device);
	void LoadModel(ID3D12Device * device);

	// IMGUI
	// -----------------------------------------------
	// void PrepareImguiFrame();
	// -----------------------------------------------

	// Device resources.
	std::unique_ptr<DX::DeviceResources> m_deviceResources;

	// Rendering loop timer.
	DX::StepTimer m_timer;

	// DX TK
	std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;

	// using TriangleVertex = DirectX::VertexPositionNormalTexture;

	// std::unique_ptr<DirectX::NormalMapEffect> m_triangleEffect;
	// std::unique_ptr<DirectX::PrimitiveBatch<TriangleVertex>> m_triangleBatch;

	std::unique_ptr<DirectX::CommonStates> m_states;

	std::unique_ptr<DirectX::DescriptorHeap> m_resourceDescHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_normalMap;

	enum ResourceDescriptors
	{
		Rocks,
		NormalMap,
		Count
	};

	using GridVertex = DirectX::VertexPositionColor;
	
	std::unique_ptr<DirectX::BasicEffect> m_gridEffect;
	std::unique_ptr<DirectX::PrimitiveBatch<GridVertex>> m_gridBatch;
	DirectX::SimpleMath::Matrix m_gridModelMatrix;

	DirectX::SimpleMath::Matrix m_shapeModelMatrix;
	std::unique_ptr<DirectX::GeometricPrimitive> m_shape;
	std::unique_ptr<DirectX::NormalMapEffect> m_shapeEffect;

	DirectX::SimpleMath::Matrix m_cupModelMatrix;
	std::unique_ptr<DirectX::EffectFactory> m_effectFactory;
	std::unique_ptr<DirectX::Model> m_cupModel;
	std::unique_ptr<DirectX::EffectTextureFactory> m_cupModelResources;
	std::vector<std::shared_ptr<DirectX::IEffect>> m_cupModelEffects;

	DirectX::SimpleMath::Matrix m_viewMatrix;
	DirectX::SimpleMath::Matrix m_projMatrix;

	// IMGUI
	// -----------------------------------------------
	// bool m_showDemoWindow = true;
	// bool m_showAnotherWindow = false;
	// ImVec4 m_clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	// -----------------------------------------------
};
