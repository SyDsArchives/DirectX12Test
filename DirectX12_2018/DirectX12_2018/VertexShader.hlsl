Texture2D<float4> tex:register(t0);
SamplerState smp:register(s0);

struct Output{
	float4 svpos:SV_POSITION;
	float4 normal:NORMAL;
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
	output.svpos = viewprojpos;

	output.normal = mul(world,normal);
	
	return output;
}

//ピクセルシェーダ
float4 ps(Output output):SV_Target
{
	float3 light = float3(-1, 1, -1);
	light = normalize(light);
	float brightness = dot(output.normal, light);
	brightness = brightness * diffuse;
	return float4(brightness, brightness, brightness, 1);
}