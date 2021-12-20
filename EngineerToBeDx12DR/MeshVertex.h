#pragma once

struct MeshVertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 color;
	uint32_t id;

	MeshVertex(float x, float y, float z, uint32_t id)
		: position(x, y, z), color(1, 1, 1), id(id) {}

	static D3D12_INPUT_LAYOUT_DESC InputLayout;

private:
	static constexpr unsigned int InputElementCount = 2;
	static constexpr D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount] = {
		{
			"SV_Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		// {
			// ""
		// },
		{
			"ID", 0, DXGI_FORMAT_R32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};
};
