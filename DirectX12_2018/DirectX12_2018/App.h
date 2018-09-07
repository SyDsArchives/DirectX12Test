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

	HWND GetHWND();

	void Initialize();
	void Run();
	void Terminate();
};

