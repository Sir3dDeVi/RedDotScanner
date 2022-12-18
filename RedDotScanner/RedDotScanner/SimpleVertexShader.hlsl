cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
}

float4 main(float3 vPos : POSITION) : SV_POSITION
{
	float4 pos = float4(vPos, 1.0f);

	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);

	return pos;
}