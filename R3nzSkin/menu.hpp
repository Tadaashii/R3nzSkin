#pragma once

#include <cinttypes>
#include <map>

namespace config {
	extern int32_t current_combo_skin_index;
	extern int32_t current_combo_ward_index;
	extern int32_t current_ward_skin_index;

	void save() noexcept;
	void load() noexcept;
	void reset() noexcept;
};

namespace menu {
	void draw() noexcept;
};

