Texture2D<float4> tex:register(t0);
SamplerState smp:register(s0);
matrix mat:register(b0);

struct Output{
	float4 svpos:SV_POSITION;
	//float4 color : COLOR;
	float2 uv:TEXCOORD;
};


//頂点シェーダ
Output vs( float4 pos:POSITION,float2 uv:TEXCOORD )
{
	Output output;
	//pos = mul(mat, pos);
	//output.svpos = output.color = pos;
	output.svpos = pos;
	//output.uv = uv;
	return output;
}

//ピクセルシェーダ
float4 ps(Output output):SV_Target
{
	//float3 color = tex.Sample(smp,output.uv).abg;
	//return float4(output.uv.x,output.uv.y,1,1);
	return tex.Sample(smp,output.uv).rgba;
}