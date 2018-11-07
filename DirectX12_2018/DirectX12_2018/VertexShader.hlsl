SamplerState smp:register(s0);
Texture2D<float4> tex:register(t0);
Texture2D<float4> tex2:register(t1);

cbuffer mat:register(b0)
{
	float4x4 world;
	float4x4 viewproj;
}

cbuffer material:register(b1)
{
	float4 diffuse;
	float4 specular;
	float4 ambient;
}

struct Output {
	float4 svpos:SV_POSITION;
	float4 normal:NORMAL;
	float4 pos:POSITION;
	float2 uv:TEXCOORD;
};



//頂点シェーダ
Output vs( float4 pos:POSITION,float4 normal:NORMAL,float2 uv:TEXCOORD)
{
	Output output;

	output.svpos = pos;
	
	float4 localpos = pos;
	float4 worldpos = mul(world, localpos);
	float4 viewprojpos = mul(viewproj, worldpos);
	output.pos = output.svpos = viewprojpos;

	output.normal = mul(world,normal);

	output.uv = uv;
	
	return output;
}

//ピクセルシェーダ
float4 ps(Output output):SV_Target
{
	//環境光
	float ambientNum = ambient;

	//光源
	float3 light = float3(-1, 1, -1);
	light = normalize(light);

	//明るさ
	float brightness = dot(output.normal.xyz, light) + ambientNum;

	float3 color;
	float alpha;

	color = diffuse.rgb * tex2.Sample(smp, output.uv).rgb;

	alpha = diffuse.a;

	return float4(color.r * brightness, color.g * brightness, color.b * brightness, alpha);
}