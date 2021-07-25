#include <Windows.h>

#include "skin_changer.hpp"

BOOL APIENTRY DllMain(HMODULE h_module, DWORD reason, LPVOID lp_reserved)
{
	if (reason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(h_module);
		skin_changer::my_module = h_module;
		CreateThread(NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(skin_changer::init), NULL, 0, NULL);
	}
	return TRUE;
}

