#include <Windows.h>

#include "game_classes.hpp"
#include "encryption.hpp"
#include "fnv_hash.hpp"

std::uintptr_t SummonerEmoteUserComponent::rtti = 0;

SummonerEmote* SummonerEmoteUserComponent::get_summoner_emote_data(std::int32_t id) noexcept
{
	static const auto GetSummonerEmoteData = reinterpret_cast<SummonerEmote*(__cdecl*)(std::int32_t)>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::functions::SummonerEmoteUserComponent__GetSummonerEmoteData);
	return GetSummonerEmoteData(id);
}

void SummonerEmoteUserComponent::set_emote_id_for_slot(SummonerEmoteSlot slot, std::int32_t emote_id) noexcept
{
	static const auto SetEmoteIdForSlot = reinterpret_cast<void(__thiscall*)(void*, std::int32_t, SummonerEmoteSlot)>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::functions::SummonerEmoteUserComponent__SetEmoteIdForSlot);
	SetEmoteIdForSlot(this, emote_id, slot);
}

void CharacterDataStack::push(const char* model, std::int32_t skin) noexcept
{
	static const auto Push = reinterpret_cast<int(__thiscall*)(void*, const char* model, std::int32_t skinid, std::int32_t, bool update_spells, bool dont_update_hud, bool, bool, bool change_particle, bool, char, const char*, std::int32_t, const char*, std::int32_t, bool, std::int32_t)>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::functions::CharacterDataStack__Push);
	Push(this, model, skin, 0, false, false, false, false, true, false, -1, "\x00", 0, "\x00", 0, false, 1);
}

void CharacterDataStack::update(bool change) noexcept
{
	static const auto Update = reinterpret_cast<void(__thiscall*)(void*, bool)>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::functions::CharacterDataStack__Update);
	Update(this, change);
}

void AIBaseCommon::change_skin(const char* model, std::int32_t skin) noexcept
{
	reinterpret_cast<xor_value<std::int32_t>*>(std::uintptr_t(this) + offsets::AIBaseCommon::SkinId)->encrypt(skin);
	this->get_character_data_stack()->base_skin.skin = skin;

	if (fnv::hash_runtime(this->get_character_data_stack()->base_skin.model.str) == FNV("Lux")) {
		if (skin == 7) {
			this->get_character_data_stack()->stack.clear();
			this->get_character_data_stack()->push(model, skin);
			return;
		} else {
			this->get_character_data_stack()->stack.clear();
		}
	}

	this->get_character_data_stack()->update(true);
}

AIBaseCommon* AIMinionClient::get_gold_redirect_target() noexcept
{
	static const auto GetGoldRedirectTarget = reinterpret_cast<AIBaseCommon*(__thiscall*)(void*)>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::functions::GetGoldRedirectTarget);
	return GetGoldRedirectTarget(this);
}

bool AIMinionClient::is_lane_minion() noexcept
{
	return reinterpret_cast<xor_value<bool>*>(std::uintptr_t(this) + offsets::AIMinionClient::IsLaneMinion)->decrypt();
}