#pragma once

struct MeshVertex
{
	DirectX::XMFLOAT3 position;
	float alpha;

	MeshVertex(float x, float y, float z, float alpha)
		: position(x, y, z), alpha(alpha) {}

	static D3D12_INPUT_LAYOUT_DESC InputLayout;

private:
	static constexpr unsigned int InputElementCount = 2;
	static constexpr D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount] = {
		{
			"SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"ALPHA", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};
};
