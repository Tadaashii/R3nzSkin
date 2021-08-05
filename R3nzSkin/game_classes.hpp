#pragma once

#include <vector>
#include <cinttypes>
#include <string>
#include <memory>
#include <map>
#include <unordered_map>

#include "offsets.hpp"

#define CONCAT_IMPL(x, y) x##y
#define MACRO_CONCAT(x, y) CONCAT_IMPL(x, y)
#define PAD(SIZE) uint8_t MACRO_CONCAT(_pad, __COUNTER__)[SIZE];

template <size_t Index, typename ReturnType, typename... Args>
ReturnType call_virtual(void* instance, Args... args) noexcept
{
	using Fn = ReturnType(__thiscall*)(void*, Args...);
	auto function = (*reinterpret_cast<Fn**>(instance))[Index];
	return function(instance, args...);
}

enum class GGameState_s : std::int32_t {
	LoadingScreen = 0,
	Connecting = 1,
	Running = 2,
	Poaused = 3,
	Finished = 4,
	Exiting = 5
};

class GameClient {
	PAD(0x8);
public:
	GGameState_s game_state;
};

class AString {
public:
	const char* str;
	std::size_t length;
	std::size_t capacity;
};

class Champion {
public:
	class Skin
	{
	public:
		std::int32_t skin_id;
		AString skin_name;
	};
private:
	PAD(0x4);
public:
	AString champion_name;
	PAD(0x48);
	std::vector<Skin> skins;
	PAD(0x8);
};

class ChampionManager {
	PAD(0xC);
public:
	std::vector<Champion*> champions;
};

class CharacterStackData {
public:
	AString model;
	std::int32_t skin;
	PAD(0x20);
	bool update_spells;
	bool dont_update_hud;
	bool change_particle;
	PAD(0x1);
	PAD(0xC);
};

class CharacterDataStack {
public:
	std::vector<CharacterStackData> stack;
	CharacterStackData base_skin;

	void update(const bool change) noexcept;
	void push(const char* model, const std::int32_t skin) noexcept;
};

class ComponentHost {
private:
	std::map<std::uintptr_t, void*>& components() { return *reinterpret_cast<std::map<std::uintptr_t, void*>*>(std::uintptr_t(this) + offsets::ComponentHost::Components); }
public:
	template<typename T>
	T* get_component()
	{
		auto compoment = this->components().find(T::rtti);
		return compoment != this->components().end() ? *reinterpret_cast<T**>(*reinterpret_cast<std::uintptr_t*>(compoment->second) + 0x0008) : nullptr;
	}
};

class GameObject {
public:
	std::string& name() { return *reinterpret_cast<std::string*>(std::uintptr_t(this) + offsets::GameObject::Name); }

	ComponentHost* get_component_host() { return call_virtual<1, ComponentHost*>(this); }
};

class AIBaseCommon : public GameObject {
public:
	CharacterDataStack* get_character_data_stack() { return reinterpret_cast<CharacterDataStack*>(std::uintptr_t(this) + offsets::AIBaseCommon::CharacterDataStack); }

	bool skin_model_push(const char* model, const std::int32_t skin) noexcept;
	void change_skin(const char* model, const std::int32_t skin) noexcept;
};

class AIHero : public AIBaseCommon {
public:
};

class AIMinionClient : public AIBaseCommon {
public:
	AIBaseCommon* get_gold_redirect_target() noexcept;
};

template <class T>
class ManagerTemplate {
	PAD(0x4);
public:
	T** list;
	std::size_t length;
	std::size_t capacity;
};