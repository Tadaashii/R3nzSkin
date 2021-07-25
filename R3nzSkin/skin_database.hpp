#pragma once

#include <map>
#include <string>
#include <cinttypes>
#include <vector>

class SummonerEmote;

namespace skin_database {
	class skin_info {
	public:
		std::string model_name;
		std::string skin_name;
		int32_t skin_id;
	};

	class jungle_mob_skin_info {
	public:
		std::string name;
		std::string model_name;
		std::vector<uint32_t> name_hashes;
		std::vector<std::string> skins;
	};

	void load();

	extern std::map<uint32_t, std::vector<skin_info>> champions_skins;
	extern std::vector<std::pair<uint32_t, std::string>> wards_skins;
	extern std::vector<jungle_mob_skin_info> jungle_mobs_skins;
	extern std::vector<std::string> minions_skins;
	extern std::vector<std::pair<SummonerEmote*, std::string>> summoner_emotes;
};