SamplerState smp:register(s0);
Texture2D<float4> tex:register(t0);
Texture2D<float4> depth:register(t1);

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
	//通常描画
	return tex.Sample(smp,input.uv);

	//ガウシアンぼかし 
	//float w, h;
	//tex.GetDimensions(w, h);

	//float4 gauss = tex.Sample(smp,input.uv);
	//float dx = 1.0f / w;
	//float dy = 1.0f / h;

	//gauss = gauss * 36 / 256;	//中心
	//dx *= 2;
	//dy *= 2;

	////今のピクセルに24近傍のピクセル値を加算
	//gauss += tex.Sample(smp, input.uv + float2(-2 * dx, -2 * dy)) * 1 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(-1 * dx, -2 * dy)) * 4 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(0 * dx, -2 * dy)) * 6 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(1 * dx, -2 * dy)) * 4 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(2 * dx, -2 * dy)) * 1 / 256;

	//gauss += tex.Sample(smp, input.uv + float2(-2 * dx, -1 * dy)) * 4 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(-1 * dx, -1 * dy)) * 16 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(0 * dx, -1 * dy)) * 24 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(1 * dx, -1 * dy)) * 16 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(2 * dx, -1 * dy)) * 4 / 256;

	//gauss += tex.Sample(smp, input.uv + float2(-2 * dx, 0 * dy)) * 6 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(-1 * dx, 0 * dy)) * 24 / 256;
	////中心は既に計算済み 									   
	//gauss += tex.Sample(smp, input.uv + float2(1 * dx, 0 * dy)) * 24 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(2 * dx, 0 * dy)) * 6 / 256;

	//gauss += tex.Sample(smp, input.uv + float2(-2 * dx, 1 * dy)) * 4 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(-1 * dx, 1 * dy)) * 16 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(0 * dx, 1 * dy)) * 24 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(1 * dx, 1 * dy)) * 16 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(2 * dx, 1 * dy)) * 4 / 256;

	//gauss += tex.Sample(smp, input.uv + float2(-2 * dx, 2 * dy)) * 1 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(-1 * dx, 2 * dy)) * 4 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(0 * dx, 2 * dy)) * 6 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(1 * dx, 2 * dy)) * 4 / 256;
	//gauss += tex.Sample(smp, input.uv + float2(2 * dx, 2 * dy)) * 1 / 256;

	//return gauss;

	//ポスタリゼーション
	//return float4(tex.Sample(smp, input.uv).rgb - fmod(tex.Sample(smp, input.uv).rgb, 0.25f), 1);

	//シャドウの確認
	/*float dep = pow(depth.Sample(smp, input.uv),50);
	return float4(dep, dep, dep, 1);*/
}