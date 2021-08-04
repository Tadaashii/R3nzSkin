#pragma once

#include <map>
#include <string>
#include <cinttypes>
#include <vector>

namespace skin_database {
	class skin_info {
	public:
		std::string model_name;
		std::string skin_name;
		int32_t skin_id;
	};

	void load() noexcept;

	extern std::map<uint32_t, std::vector<skin_info>> champions_skins;
	extern std::vector<std::pair<uint32_t, std::string>> wards_skins;
};