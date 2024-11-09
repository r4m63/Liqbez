// DLL ---------------------------------------
/*
make *.def and write func names
and their import names:
		LIBRARY
		EXPORTS
		Sum=Sum
		Sub=Sub
		Mult=Mult
		Div=Div
		Part=Part
*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define EXPORT __declspec(dllexport)

int EXPORT Sum(int i, int j);
int EXPORT Sub(int i, int j);
int EXPORT Mult(int i, int j);
int EXPORT Div(int i, int j);
int EXPORT Part(int i, int j);


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}


int EXPORT Sum(int i, int j)
{
	return i + j;
}
int EXPORT Sub(int i, int j)
{
	return i - j;
}
int EXPORT Mult(int i, int j)
{
	return i * j;
}
int EXPORT Div(int i, int j)
{
	if (!j)
		return j;
	return i / j;
}
int EXPORT Part(int i, int j)
{
	if (!j)
		return j;
	return i % j;
}




// EXE ---------------------------------------
#include <Windows.h>
#include <iostream>


typedef int(*SumF)(int, int);
typedef int(*SubF)(int, int);
typedef int(*MultF)(int, int);
typedef int(*DivF)(int, int);
typedef int(*PartF)(int, int);

/*
extern "C" {
		__declspec(dllimport) int TestF(int, int);
}
int main() {
		HMODULE hModule = LoadLibrary(L"mydll.dll");
		if (hModule == NULL) {
				return 1;
		}
		int(*)(int, int) lpfnAddNumbers = reinterpret_cast<int(*)(int, int)>(GetProcAddress(hModule, "AddNumbers"));
		if (lpfnAddNumbers == NULL) {
				FreeLibrary(hModule);
				return 1;
		}
		int result = lpfnAddNumbers(2, 3);
		FreeLibrary(hModule);
		return 0;
}
*/

int main()
{

	HMODULE hDLL = LoadLibrary(L"dll.dll");
	if (!hDLL)
		return 1;


	SumF	Sum = (SumF)GetProcAddress(hDLL, "Sum");
	SubF	Sub = (SubF)GetProcAddress(hDLL, "Sub");
	MultF Mult = (MultF)GetProcAddress(hDLL, "Mult");
	DivF	Div = (DivF)GetProcAddress(hDLL, "Div");
	PartF Part = (PartF)GetProcAddress(hDLL, "Part");



	std::cout << Sum(1, 2) << std::endl;
	std::cout << Sub(100, 63) << std::endl;
	std::cout << Mult(2, 7) << std::endl;
	std::cout << Div(36, 4) << std::endl;
	std::cout << Part(2018, 4) << std::endl;

	int a;
	std::cin >> a;

	FreeLibrary(hDLL);


	return 0;

}