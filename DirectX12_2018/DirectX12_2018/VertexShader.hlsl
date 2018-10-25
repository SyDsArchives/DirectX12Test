Texture2D<float4> tex:register(t0);
SamplerState smp:register(s0);

struct Output{
	float4 svpos:SV_POSITION;
	float4 normal:NORMAL;
	float4 pos:POSITION;
	//float2 uv:TEXCOORD;
};

cbuffer mat:register(b0)
{
	float4x4 world;
	float4x4 viewproj;
}

cbuffer material:register(b1)
{
	float3 diffuse;
	/*float alpha;
	float specularity;
	float3 specularityColor;*/
}


//頂点シェーダ
Output vs( float4 pos:POSITION,float4 normal:NORMAL)
{
	Output output;

	output.svpos = pos;
	
	float4 localpos = pos;
	float4 worldpos = mul(world, localpos);
	float4 viewprojpos = mul(viewproj, worldpos);
	output.pos = output.svpos = viewprojpos;

	output.normal = mul(world,normal);
	
	return output;
}

//ピクセルシェーダ
float4 ps(Output output):SV_Target
{
	//環境光
	float ambient = 0.5;

	//光源
	float3 light = float3(-1, 1, -1);
	light = normalize(light);

	//明るさ
	float brightness = dot(output.normal, light) + ambient;

	//RGB
	float Red = brightness * diffuse.r;
	float Green = brightness * diffuse.g;
	float Blue = brightness * diffuse.b;

	return float4(Red, Green, Blue, 1);
}