SamplerState smp:register(s0);
Texture2D<float4> tex:register(t0);

struct Output {
	float4 pos:POSITION;
	float4 svpos:SV_POSITION;
	float2 uv:TEXCOORD;
};

//頂点シェーダ
Output PeraVertex(float4 pos:POSITION, float2 uv : TEXCOORD)
{
	Output output;

	output.svpos = pos;

	output.uv = uv;

	return output;
}

//ピクセルシェーダ
float4 PeraPixel(Output input) :SV_Target
{
	return tex.Sample(smp,input.uv);
	//return float4(tex.Sample(smp, input.uv).rgb - fmod(tex.Sample(smp, input.uv).rgb, 0.25f), 1);
	//return float4(test.r - 0.1f, test.g, test.b - 0.1f, test.a);
	//return float4(1,0,0,1);
}