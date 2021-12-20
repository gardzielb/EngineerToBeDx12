#include "pch.h"
#include "MousePickEffect.h"

#include "MeshVertex.h"

#include "MousePickPixelShader.h"
#include "MousePickVertexShader.h"


namespace dx = DirectX;

struct __declspec(align(16)) MousePickConstants
{
	DirectX::XMMATRIX transformMatrix;
};

static_assert((sizeof(MousePickConstants) % 16) == 0, "CB size alignment");

MousePickEffect::MousePickEffect(ID3D12Device* device, dx::RenderTargetState rtState)
{
	DX::ThrowIfFailed(
		device->CreateRootSignature(
			0, g_MousePickVS, sizeof(g_MousePickVS),
			IID_PPV_ARGS(m_rootSignature.ReleaseAndGetAddressOf())
		)
	);

	m_constantBuffer = dx::GraphicsMemory::Get(device).AllocateConstant<MousePickConstants>();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = MeshVertex::InputLayout;
	psoDesc.BlendState = {
		FALSE, // AlphaToCoverageEnable
		FALSE, // IndependentBlendEnable
		{
			{
				TRUE, // BlendEnable
				FALSE, // LogicOpEnable
				D3D12_BLEND_ONE, // SrcBlend
				D3D12_BLEND_INV_SRC_ALPHA, // DestBlend
				D3D12_BLEND_OP_ADD, // BlendOp
				D3D12_BLEND_ONE, // SrcBlendAlpha
				D3D12_BLEND_INV_SRC_ALPHA, // DestBlendAlpha
				D3D12_BLEND_OP_ADD, // BlendOpAlpha
				D3D12_LOGIC_OP_NOOP,
				D3D12_COLOR_WRITE_ENABLE_ALL
			}
		}
	};
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
	psoDesc.DepthStencilState = dx::CommonStates::DepthDefault;
	psoDesc.DSVFormat = rtState.dsvFormat;
	psoDesc.NodeMask = rtState.nodeMask;
	psoDesc.NumRenderTargets = rtState.numRenderTargets;
	memcpy(psoDesc.RTVFormats, rtState.rtvFormats, sizeof(DXGI_FORMAT) * D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT);
	psoDesc.SampleDesc = rtState.sampleDesc;
	psoDesc.SampleMask = rtState.sampleMask;
	psoDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	psoDesc.VS = {g_MousePickVS, sizeof(g_MousePickVS)};
	psoDesc.PS = {g_MousePickPS, sizeof(g_MousePickPS)};
	psoDesc.pRootSignature = m_rootSignature.Get();

	DX::ThrowIfFailed(
		device->CreateGraphicsPipelineState(&psoDesc, IID_GRAPHICS_PPV_ARGS(m_pipelineState.ReleaseAndGetAddressOf()))
	);
}

void MousePickEffect::Apply(ID3D12GraphicsCommandList* commandList)
{
	auto transformMatrix = m_modelMatrix * m_viewMatrix * m_projectionMatrix;
	memcpy(m_constantBuffer.Memory(), &transformMatrix, m_constantBuffer.Size());

	commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer.GpuAddress());
	commandList->SetPipelineState(m_pipelineState.Get());
}
