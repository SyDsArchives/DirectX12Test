#include <Windows.h>
#include <iostream>
#include "App.h"

int main()
{
	App app;

	app.Initialize();
	app.Run();
	app.Terminate();

	return 0;
}