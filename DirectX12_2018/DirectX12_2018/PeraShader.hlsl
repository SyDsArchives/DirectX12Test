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
	//return float4(1,1,1,1);
}