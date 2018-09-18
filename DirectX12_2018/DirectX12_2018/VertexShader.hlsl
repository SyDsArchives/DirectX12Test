Texture2D<float4> tex:register(t0);
SamplerState smp:register(s0);

struct Output{
	float4 svpos:SV_POSITION;
	float4 pos:POSITION;
	float2 uv:TEXCOORD;
	//float4 color : COLOR;
};


//頂点シェーダ
Output vs( float4 pos:POSITION,float2 uv:TEXCOORD )
{
	Output output;
	output.svpos = pos;
	output.pos = pos;
	output.uv = uv;
	//output.color = pos;
	return output;
}

//ピクセルシェーダ
float4 ps(Output output):SV_Target
{
	//float3 color = tex.Sample(smp,output.uv).abg;
	//return float4(output.uv.x,output.uv.y,1,1);
	return tex.Sample(smp,output.uv);
}