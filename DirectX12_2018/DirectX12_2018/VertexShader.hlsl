
//���_�V�F�[�_
float4 VShader( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}

//�s�N�Z���V�F�[�_
float4 PShader(float4 pos : POSITION) : SV_Target
{
	return float4(1,1,1,1);
}