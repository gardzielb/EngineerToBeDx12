#pragma once

struct __declspec(align(16)) MousePickConstants
{
	DirectX::XMMATRIX transformMatrix;
};

static_assert((sizeof(MousePickConstants) % 16) == 0, "CB size alignment");

class MousePickEffect : DirectX::IEffect, DirectX::IEffectMatrices
{
public:
	MousePickEffect(ID3D12Device* device, DirectX::RenderTargetState rtState);

	void Apply(ID3D12GraphicsCommandList* commandList) override;

	inline void XM_CALLCONV SetWorld(DirectX::FXMMATRIX value) override
	{
		m_modelMatrix = value;
	}

	inline void XM_CALLCONV SetView(DirectX::FXMMATRIX value) override
	{
		m_viewMatrix = value;
	}

	inline void XM_CALLCONV SetProjection(DirectX::FXMMATRIX value) override
	{
		m_projectionMatrix = value;
	}

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

	DirectX::SimpleMath::Matrix m_modelMatrix = DirectX::SimpleMath::Matrix::Identity;
	DirectX::SimpleMath::Matrix m_viewMatrix = DirectX::SimpleMath::Matrix::Identity;
	DirectX::SimpleMath::Matrix m_projectionMatrix = DirectX::SimpleMath::Matrix::Identity;

	DirectX::GraphicsResource m_constantBuffer;
};
