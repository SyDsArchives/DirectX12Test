struct Output{
	float4 svpos:SV_POSITION;
	float4 pos:POSITION;
	//float4 color : COLOR;
};


//���_�V�F�[�_
Output vs( float4 pos:POSITION )
{
	Output output;
	output.svpos = pos;
	output.pos = pos;
	//output.color = pos;
	return output;
}

//�s�N�Z���V�F�[�_
float4 ps(Output output):SV_Target
{
	return float4((output.pos.xy + float2(1,1)) / 2,1,1);
}