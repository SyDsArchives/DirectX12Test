#pragma once
class App
{
private:
	WNDCLASSEX w = {};
	HWND hwnd = {};
	MSG msg;
public:
	App();
	~App();

	void Initialize();
	void Run();
	void Terminate();
};

