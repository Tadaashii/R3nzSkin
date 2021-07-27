#pragma once

#include <Windows.h>

namespace R3nzSkin {
	inline bool run;

	void __stdcall update() noexcept;
	void __stdcall init() noexcept;

	extern HMODULE my_module;
};