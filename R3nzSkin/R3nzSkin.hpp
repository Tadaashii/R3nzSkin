#pragma once

#include <Windows.h>

namespace R3nzSkin {
	inline bool run;

	void update() noexcept;
	void init() noexcept;

	extern HMODULE my_module;
};