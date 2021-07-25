#include <Windows.h>
#include <clocale>

#include "R3nzSkin.hpp"

BOOL APIENTRY DllMain(HMODULE moduleHandle, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH) {
		std::setlocale(LC_CTYPE, ".utf8");
		DisableThreadLibraryCalls(moduleHandle);
		R3nzSkin::my_module = moduleHandle;
		CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(R3nzSkin::init), NULL, 0, NULL);
	}
	return TRUE;
}

