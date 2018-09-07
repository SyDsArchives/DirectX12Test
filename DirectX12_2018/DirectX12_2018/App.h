#pragma once
#include <memory>
class DirectX12;
class App
{
private:
	WNDCLASSEX w = {};
	HWND hwnd = {};
	MSG msg;

	std::shared_ptr<DirectX12> dx12;
public:
	App();
	~App();

	HWND GetHWND();

	void Initialize();
	void Run();
	void Terminate();
};

