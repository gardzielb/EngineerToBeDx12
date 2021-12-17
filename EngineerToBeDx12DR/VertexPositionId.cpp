#include "pch.h"
#include "VertexPositionId.h"

D3D12_INPUT_LAYOUT_DESC VertexPositionId::InputLayout = {
	VertexPositionId::InputElements,
	VertexPositionId::InputElementCount
};
