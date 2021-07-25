#include <Windows.h>
#include <clocale>

#include "skin_changer.hpp"

BOOL APIENTRY DllMain(HMODULE moduleHandle, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH) {
		std::setlocale(LC_CTYPE, ".utf8");
		DisableThreadLibraryCalls(moduleHandle);
		skin_changer::my_module = moduleHandle;
		CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(skin_changer::init), NULL, 0, NULL);
	}
	return TRUE;
}

