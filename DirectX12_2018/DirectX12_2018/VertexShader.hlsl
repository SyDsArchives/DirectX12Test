
//���_�V�F�[�_
float4 vs( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
}

//�s�N�Z���V�F�[�_
float4 ps(float4 pos : SV_POSITION) : SV_Target
{
	return float4(1,1,1,1);
}