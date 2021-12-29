#include "pch.h"
#include "OutlineEffect.h"

#include "MeshVertex.h"

#include "OutlineVertexShader.h"
#include "OutlinePixelShader.h"

namespace dx = DirectX;

struct __declspec(align(16)) MousePickConstants
{
	DirectX::XMMATRIX transformMatrix;
};

static_assert((sizeof(MousePickConstants) % 16) == 0, "CB size alignment");

OutlineEffect::OutlineEffect(ID3D12Device* device, dx::RenderTargetState rtState)
{
	DX::ThrowIfFailed(
		device->CreateRootSignature(
			0, g_OutlineVS, sizeof(g_OutlineVS),
			IID_PPV_ARGS(m_rootSignature.ReleaseAndGetAddressOf())
		)
	);

	m_constantBuffer = dx::GraphicsMemory::Get(device).AllocateConstant<MousePickConstants>();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = MeshVertex::InputLayout;
	psoDesc.BlendState = dx::CommonStates::Opaque;
	psoDesc.RasterizerState = {
		D3D12_FILL_MODE_SOLID,
		D3D12_CULL_MODE_BACK,
		FALSE, // FrontCounterClockwise
		D3D12_DEFAULT_DEPTH_BIAS,
		D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
		D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
		TRUE, // DepthClipEnable
		TRUE, // MultisampleEnable
		FALSE, // AntialiasedLineEnable
		0, // ForcedSampleCount
		D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
	};
	// psoDesc.DepthStencilState = dx::CommonStates::DepthDefault;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
	depthStencilDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	depthStencilDesc.DepthEnable = false;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
	depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xff;
	depthStencilDesc.StencilWriteMask = 0x00;

	psoDesc.DepthStencilState = depthStencilDesc;

	psoDesc.DSVFormat = rtState.dsvFormat;
	psoDesc.NodeMask = rtState.nodeMask;
	psoDesc.NumRenderTargets = rtState.numRenderTargets;
	memcpy(psoDesc.RTVFormats, rtState.rtvFormats, sizeof(DXGI_FORMAT) * D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT);
	psoDesc.SampleDesc = rtState.sampleDesc;
	psoDesc.SampleMask = rtState.sampleMask;
	psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	psoDesc.VS = { g_OutlineVS, sizeof(g_OutlineVS) };
	psoDesc.PS = { g_OutlinePS, sizeof(g_OutlinePS) };
	psoDesc.pRootSignature = m_rootSignature.Get();

	DX::ThrowIfFailed(
		device->CreateGraphicsPipelineState(&psoDesc, IID_GRAPHICS_PPV_ARGS(m_pipelineState.ReleaseAndGetAddressOf()))
	);
}

void OutlineEffect::Apply(ID3D12GraphicsCommandList* commandList)
{
	auto transformMatrix = m_modelMatrix * m_viewMatrix * m_projectionMatrix;
	memcpy(m_constantBuffer.Memory(), &transformMatrix, m_constantBuffer.Size());

	commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer.GpuAddress());
	commandList->SetPipelineState(m_pipelineState.Get());
}