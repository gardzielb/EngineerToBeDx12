#include "MousePickRS.hlsli"

[RootSignature(MousePickRS)]
uint4 main(uint id : ID, float4 position : SV_Position) : SV_Target0
{
	return uint4(255, 255, 255, 255);
	// return uint4(id & 0xFF000000, id & 0xFF0000, id & 0xFF00, id & 0xFF);
}
