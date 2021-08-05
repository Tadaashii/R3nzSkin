#include <Windows.h>
#include <cinttypes>
#include <thread>
#include <mutex>

#include "R3nzSkin.hpp"
#include "offsets.hpp"
#include "hook.hpp"
#include "skin_database.hpp"
#include "menu.hpp"
#include "fnv_hash.hpp"
#include "game_classes.hpp"
#include "memory.hpp"

std::once_flag change_skins;

void R3nzSkin::update() noexcept
{
	const auto league_module = std::uintptr_t(GetModuleHandle(nullptr));
	const auto player = *reinterpret_cast<AIBaseCommon**>(league_module + offsets::global::Player);
	const auto minions = *reinterpret_cast<ManagerTemplate<AIMinionClient>**>(league_module + offsets::global::ManagerTemplate_AIMinionClient_);

	std::call_once(change_skins, [&]()
	{
		if (player) {
			if (config::current_combo_skin_index > 0) {
				const auto& values = skin_database::champions_skins[fnv::hash_runtime(player->get_character_data_stack()->base_skin.model.str)];
				player->change_skin(values[config::current_combo_skin_index - 1].model_name.c_str(), values[config::current_combo_skin_index - 1].skin_id);
			}
		}
	});

	static const auto change_skin_for_object = [](AIBaseCommon* obj, const std::int32_t skin) -> void
	{
		if (skin == -1)
			return;

		if (obj->get_character_data_stack()->base_skin.skin != skin) {
			obj->get_character_data_stack()->base_skin.skin = skin;
			obj->get_character_data_stack()->update(true);
		}
	};

	for (std::size_t i = 0; i < minions->length; i++) {
		const auto minion = minions->list[i];
		const auto owner = minion->get_gold_redirect_target();

		if (owner) {
			const auto hash = fnv::hash_runtime(minion->get_character_data_stack()->base_skin.model.str);
			if (hash == FNV("JammerDevice") || hash == FNV("SightWard") || hash == FNV("YellowTrinket") || hash == FNV("VisionWard") || hash == FNV("TestCubeRender10Vision")) {
				if (!player || owner == player) {
					if (hash == FNV("TestCubeRender10Vision"))
						change_skin_for_object(minion, 0);
					else
						change_skin_for_object(minion, config::current_ward_skin_index);
				}
				continue;
			}
			change_skin_for_object(minion, owner->get_character_data_stack()->base_skin.skin);
		}
	}
}

void R3nzSkin::init() noexcept
{
	using namespace std::chrono_literals;
	memory::start(true);
	auto client = *reinterpret_cast<GameClient**>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::global::GameClient);
	while (!client || client->game_state != GGameState_s::Running) {
		std::this_thread::sleep_for(500ms);
		client = *reinterpret_cast<GameClient**>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::global::GameClient);
	}

	memory::start(false);
	config::load();
	d3d_hook::hook();

	R3nzSkin::run = true;
	while (true) {
		if (!run) break;
		std::this_thread::sleep_for(200ms);
	}

	d3d_hook::unhook();
	FreeLibraryAndExitThread(my_module, 0);
}

HMODULE R3nzSkin::my_module;
