#pragma once

#include "Effects.h"

class OutlineEffect : public DirectX::IEffect, DirectX::IEffectMatrices
{
public:
	OutlineEffect(ID3D12Device* device, DirectX::RenderTargetState rtState);

	void Apply(ID3D12GraphicsCommandList* commandList) override;

	inline void XM_CALLCONV SetProjection(DirectX::FXMMATRIX value) override
	{
		m_projectionMatrix = value;
	}

	inline void XM_CALLCONV SetView(DirectX::FXMMATRIX value) override
	{
		m_viewMatrix = value;
	}

	inline void XM_CALLCONV SetWorld(DirectX::FXMMATRIX value) override
	{
		m_modelMatrix = value;
	}

private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

	DirectX::GraphicsResource m_constantBuffer;
	DirectX::SimpleMath::Matrix m_modelMatrix = DirectX::SimpleMath::Matrix::Identity;
	DirectX::SimpleMath::Matrix m_viewMatrix = DirectX::SimpleMath::Matrix::Identity;
	DirectX::SimpleMath::Matrix m_projectionMatrix = DirectX::SimpleMath::Matrix::Identity;
};
