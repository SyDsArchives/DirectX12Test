struct Output
{
	float4 svpos : SV_POSITION;
	float4 color : COLOR;
};


//頂点シェーダ
float4 vs( float4 pos : POSITION ) : SV_POSITION//三角形が出次第SVPOSを消す
{
	/*Output output;
	output.svpos = output.color = pos;*/
	return pos;
}

//ピクセルシェーダ
float4 ps(float4 pos : SV_POSITION) : SV_Target
{
	return float4(1,1,1,1);
}