#pragma once

#include <Windows.h>

namespace skin_changer {
	void update() noexcept;
	void init() noexcept;

	extern HMODULE my_module;
};