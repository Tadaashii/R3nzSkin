#include <algorithm>

#include "skin_database.hpp"
#include "R3nzSkin.hpp"
#include "offsets.hpp"
#include "fnv_hash.hpp"
#include "game_classes.hpp"

void skin_database::load() noexcept
{
	static const auto translateString_UNSAFE_DONOTUSE = reinterpret_cast<const char* (__cdecl*)(const char*)>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::functions::translateString_UNSAFE_DONOTUSE);
	const auto g_championg_manager = *reinterpret_cast<ChampionManager**>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::global::ChampionManager);

	for (const auto& champion : g_championg_manager->champions) {
		std::vector<int32_t> skins_ids;
		for (const auto& skin : champion->skins)
			skins_ids.push_back(skin.skin_id);
		std::sort(skins_ids.begin(), skins_ids.end());

		std::map<std::string, int32_t> temp_skin_list;
		for (auto& i : skins_ids) {
			auto skin_display_name = std::string("game_character_skin_displayname_");
			skin_display_name.append(champion->champion_name.str);
			skin_display_name.append("_");
			skin_display_name.append(std::to_string(i));

			auto skin_display_name_translated = i > 0 ? std::string(translateString_UNSAFE_DONOTUSE(skin_display_name.c_str())) : std::string(champion->champion_name.str);
			const auto it = temp_skin_list.find(skin_display_name_translated);

			if (it == temp_skin_list.end())
				temp_skin_list[skin_display_name_translated] = 1;
			else {
				skin_display_name_translated.append(" Chroma ");
				skin_display_name_translated.append(std::to_string(it->second));
				it->second = it->second + 1;
			}

			const auto champ_name = fnv::hash_runtime(champion->champion_name.str);
			champions_skins[champ_name].push_back(skin_info{ std::string(champion->champion_name.str),skin_display_name_translated,i });

			if (i == 7 && champ_name == FNV("Lux")) {
				champions_skins[champ_name].push_back(skin_info{ "LuxAir", "Elementalist Air Lux", i });
				champions_skins[champ_name].push_back(skin_info{ "LuxDark", "Elementalist Dark Lux", i });
				champions_skins[champ_name].push_back(skin_info{ "LuxFire", "Elementalist Fire Lux", i });
				champions_skins[champ_name].push_back(skin_info{ "LuxIce", "Elementalist Ice Lux", i });
				champions_skins[champ_name].push_back(skin_info{ "LuxMagma", "Elementalist Magma Lux", i });
				champions_skins[champ_name].push_back(skin_info{ "LuxMystic", "Elementalist Mystic Lux", i });
				champions_skins[champ_name].push_back(skin_info{ "LuxNature", "Elementalist Nature Lux", i });
				champions_skins[champ_name].push_back(skin_info{ "LuxStorm", "Elementalist Storm Lux", i });
				champions_skins[champ_name].push_back(skin_info{ "LuxWater", "Elementalist Water Lux", i });
			}
		}
	}

	for (auto ward_skin_id = 1;; ward_skin_id++) {
		const auto ward_display_name = std::string("game_character_skin_displayname_SightWard_" + std::to_string(ward_skin_id));
		const auto ward_display_name_translated = std::string(translateString_UNSAFE_DONOTUSE(ward_display_name.c_str()));
		
		if (ward_display_name == ward_display_name_translated)
			break;

		wards_skins.push_back({ ward_skin_id, ward_display_name_translated });
	}

	static const auto get_skins_len_for_model = [](std::string model) -> uint32_t {
		static const auto CharacterData__GetCharacterPackage = reinterpret_cast<uintptr_t(__cdecl*)(std::string&, int32_t)>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::functions::CharacterData__GetCharacterPackage);
		const auto default_skin_data = *reinterpret_cast<uintptr_t*>(CharacterData__GetCharacterPackage(model, 0) + 0x3C);
		
		for (auto skins_len = 1;; skins_len++) {
			if (*reinterpret_cast<uintptr_t*>(CharacterData__GetCharacterPackage(model, skins_len) + 0x3C) == default_skin_data)
				return skins_len;
		}
	};
}

std::map<uint32_t, std::vector<skin_database::skin_info>> skin_database::champions_skins;
std::vector<std::pair<uint32_t, std::string>> skin_database::wards_skins;