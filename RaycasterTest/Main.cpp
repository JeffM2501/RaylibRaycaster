#include "framework.h"
#include "RaycasterTest.h"

#ifdef _WINDOWS
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
#else
int main()
#endif
{
    DoMain();
}
