#include "MousePickRS.hlsli"

cbuffer Parameters : register(b0)
{
    row_major float4x4 MatrixTransform;
};

[RootSignature(MousePickRS)]
void main(inout float alpha : ALPHA, inout float4 position : SV_Position)
{
    position = mul(position, MatrixTransform);
}