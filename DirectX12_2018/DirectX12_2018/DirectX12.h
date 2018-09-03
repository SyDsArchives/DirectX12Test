#pragma once
class DirectX12
{
private:
	DirectX12();
	virtual ~DirectX12();
	void operator=(const DirectX12& dx12) {}//‘ã“ü‹Ö~
	DirectX12(const DirectX12 &dx12) {}//ƒRƒs[‹Ö~
public:
	static DirectX12& Dx12Instance()
	{
		static DirectX12 instance;
		return instance;
	}
};

