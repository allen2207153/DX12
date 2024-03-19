#include "BasicShaderHeader.hlsli"


float4 BasicPS(Output input) : SV_TARGET
{
    return float4(input.uv, 1, 1); //真ん中の四角形の色
}