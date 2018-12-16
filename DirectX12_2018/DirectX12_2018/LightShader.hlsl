Texture2D<float> tex:register(t0);
SamplerState smp : register(s0);

cbuffer mat:register(b0)
{
	float4x4 world;
	float4x4 viewproj;
	float4x4 lvp;
}

struct Output
{
	float4 pos : POSITION;
	float4 svpos : SV_POSITION;	
	float2 uv : TEXCOORD;
};


Output vs( float4 pos:POSITION, float2 uv:TEXCOORD)
{
	Output output;

	output.pos = mul(world, pos);
	output.svpos = mul(world * viewproj, pos);

	output.uv = (float2(1.0f, -1.0f) + uv * float2(0.5f, -0.5f));

	return output;
}

float4 ps(Output input) : SV_Target
{
	return float4(tex.Sample(smp, input.uv), tex.Sample(smp, input.uv), tex.Sample(smp, input.uv), 1.0f);
}