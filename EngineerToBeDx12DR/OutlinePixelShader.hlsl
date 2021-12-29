#include "MousePickRS.hlsli"

[RootSignature(MousePickRS)]
float4 main(float alpha : ALPHA, float4 position : SV_Position) : SV_Target0
{
	return float4(1, 1, 0, 1);
}
