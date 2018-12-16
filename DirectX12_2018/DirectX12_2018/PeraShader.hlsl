SamplerState smp:register(s0);
Texture2D<float4> tex:register(t0);
Texture2D<float4> depth:register(t1);

struct Output {
	float4 pos:POSITION;
	float4 svpos:SV_POSITION;
	float2 uv:TEXCOORD;
};

//���_�V�F�[�_
Output PeraVertex(float4 pos:POSITION, float2 uv : TEXCOORD)
{
	Output output;

	output.svpos = pos;

	output.uv = uv;

	return output;
}

//�s�N�Z���V�F�[�_
float4 PeraPixel(Output input) :SV_Target
{
	//�ʏ�`��
	return tex.Sample(smp,input.uv);

	//�K�E�V�A���ڂ��� 
	//float w, h;
	//tex.GetDimensions(w, h);

	//float4 gauss = tex.Sample(smp,input.uv);
	//float dx = 1.0f / w;
	//float dy = 1.0f / h;

	//gauss = gauss * 36 / 256;	//���S
	//dx *= 2;
	//dy *= 2;

	////���̃s�N�Z����24�ߖT�̃s�N�Z���l�����Z
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
	////���S�͊��Ɍv�Z�ς� 									   
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

	//�|�X�^���[�[�V����
	//return float4(tex.Sample(smp, input.uv).rgb - fmod(tex.Sample(smp, input.uv).rgb, 0.25f), 1);

	//�V���h�E�̊m�F
	/*float dep = pow(depth.Sample(smp, input.uv),50);
	return float4(dep, dep, dep, 1);*/
}