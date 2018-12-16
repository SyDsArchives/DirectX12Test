SamplerState smp:register(s0);
Texture2D<float4> tex:register(t0);
Texture2D<float4> tex2:register(t1);
Texture2D<float4> tex3:register(t2);
Texture2D<float4> clut:register(t3);
Texture2D<float4> depth:register(t4);

cbuffer mat:register(b0)
{
	matrix world;
	matrix viewproj;
	matrix lvp;
}

cbuffer material:register(b1)
{
	float4 diffuse;
	float4 specular;
	float4 ambient;
	float spec;
}

cbuffer bones : register(b2) {
	matrix boneMats[512];
}

struct Output {
	float4 pos:POSITION;
	float4 svpos:SV_POSITION;
	float4 normal:NORMAL;
	float2 uv:TEXCOORD;
	float weight:WEIGHT;
};

struct PrimOutput {
	float4 pos:POSITION;
	float4 svpos:SV_POSITION;
	float4 normal:NORMAL;
	float2 uv:TEXCOORD;
};


//頂点シェーダ
Output vs( float4 pos:POSITION,float4 normal:NORMAL,float2 uv:TEXCOORD, min16uint2 boneno:BONENO, min16uint weight:WEIGHT)
{
	Output output;

	output.svpos = pos;

	float w = weight / 100.f;
	matrix m = boneMats[boneno.x] * w + boneMats[boneno.y] * (1 - w);

	//すべての頂点にボーン行列を乗算する
	float4 localpos = mul(m, pos);

	float4 worldpos = mul(world, localpos);
	float4 viewprojpos = mul(viewproj, worldpos);
	output.pos = output.svpos = viewprojpos;

	output.normal = mul(world,normal);

	output.uv = uv;

	output.weight = w;
	
	return output;
}

//ピクセルシェーダ
float4 ps(Output output):SV_Target
{
	/*float dep = pow(depth.Sample(smp, output.uv),50);
	return float4(dep, dep, dep, 1);*/

	//環境光
	float ambientNum = ambient;

	//光源
	float3 light = float3(-1, 1, -1);
	//light = normalize(light);

	////明るさ
	//float brightness = dot(output.normal.xyz, light) + ambientNum;

	//float3 color;
	//float alpha;

	//color = diffuse.rgb * tex2.Sample(smp, output.uv).rgb;
	//alpha = diffuse.a;

	//return float4(color.r * brightness, color.g * brightness, color.b * brightness, alpha);

	//明るさ
	float brightness = dot(light,mul(output.normal.xyz, output.pos));
	brightness = brightness * 0.5f + 0.5f;
	
	//トゥーン
	float4 toon = clut.Sample(smp, float2(0.f, brightness));
	float3 matcol = float3(saturate(toon.rgb + specular.rgb * spec + ambient.rgb));

	float3 color = diffuse.rgb;
	color = color * tex2.Sample(smp, output.uv).rgb;
	color = color * matcol;

	return float4(color.r, color.g, color.b, diffuse.a);
}

PrimOutput PrimitiveVS(float4 pos:POSITION, float3 normal:NORMAL, float2 uv:TEXCOORD)
{
	PrimOutput o;
	o.svpos = mul(mul(viewproj, world), pos);
	/*o.uv = (float2(1.0f, -1.0f) + uv * float2(0.5f, -0.5f));*/
	o.uv = (float2(1.0f, -1.0f) - pos.xy * float2(0.5f, -0.5f));
	return o;
}

float4 PrimitivePS(PrimOutput input) :SV_Target
{
	//return depth.Sample(smp, input.uv);
	float3 color = float3(1, 1, 1);
	color = color * depth.Sample(smp, input.uv).rgb;
	return float4(color, 1);
	//return float4(1,1,1,1);
}
