//
// Game.h
//

#pragma once

#include "DemoImguiLayer.h"
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

	// Device resources.
	std::unique_ptr<DX::DeviceResources> m_deviceResources;

	// Rendering loop timer.
	DX::StepTimer m_timer;

	// DX TK
	std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;

	DirectX::SimpleMath::Matrix m_world;
	DirectX::SimpleMath::Matrix m_view;
	DirectX::SimpleMath::Matrix m_projection;

	std::unique_ptr<DirectX::CommonStates> m_states;
	std::unique_ptr<DirectX::GeometricPrimitive> m_shape;
	std::unique_ptr<DirectX::BasicEffect> m_effect;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_background;

	std::unique_ptr<DirectX::DescriptorHeap> m_resourceDescriptors;

	enum Descriptors
	{
		SceneTex,
		Background,
		BlurTex1,
		BlurTex2,
		Count
	};

	RECT m_fullscreenRect;
};
