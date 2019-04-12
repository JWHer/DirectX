#include "BOCEngine.h"

//프로그램 시작점
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR IpCmdLine, int nCmdShow) {

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(IpCmdLine);

	BOCEngine Application(hInstance);

	if (!Application.init())
		return 0;

	return Application.Run();
}