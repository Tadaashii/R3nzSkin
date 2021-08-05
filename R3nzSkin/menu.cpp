#include <Windows.h>
#include <fstream>

#include "menu.hpp"
#include "skin_database.hpp"
#include "fnv_hash.hpp"
#include "offsets.hpp"
#include "game_classes.hpp"
#include "imgui/imgui.h"
#include "json.hpp"
#include "R3nzSkin.hpp"

using json = nlohmann::json;

int32_t config::current_combo_skin_index = 0;
int32_t config::current_combo_ward_index = 0;
int32_t config::current_ward_skin_index = -1;

auto config_json = json();

void config::save() noexcept
{
	auto player = *reinterpret_cast<AIBaseCommon**>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::global::Player);
	if (player)
		config_json[std::string(player->get_character_data_stack()->base_skin.model.str) + ".current_combo_skin_index"] = current_combo_skin_index;

	config_json["current_combo_ward_index"] = current_combo_ward_index;
	config_json["current_ward_skin_index"] = current_ward_skin_index;

	auto out = std::ofstream(L"R3nzSkin.json");
	out << config_json.dump();
	out.close();
}

void config::load() noexcept
{
	auto out = std::ifstream(L"R3nzSkin.json");
	if (!out.good())
		return;

	config_json = json::parse(out);

	auto player = *reinterpret_cast<AIBaseCommon**>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::global::Player);
	if (player)
		current_combo_skin_index = config_json.value(std::string(player->get_character_data_stack()->base_skin.model.str) + ".current_combo_skin_index", 0);

	current_combo_ward_index = config_json.value("current_combo_ward_index", 0);
	current_ward_skin_index = config_json.value("current_ward_skin_index", -1);

	out.close();
}

void config::reset() noexcept
{
	current_combo_skin_index = 0;
	current_combo_ward_index = 0;
	current_ward_skin_index = -1;
}

char str_buffer[256];
void menu::draw() noexcept
{
	ImGui::Begin("R3nzSkin", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_AlwaysAutoResize);
	{
		static auto vector_getter_skin = [](void* vec, int idx, const char** out_text) {
			auto& vector = *static_cast<std::vector<skin_database::skin_info>*>(vec);
			if (idx < 0 || idx > static_cast<int>(vector.size())) { return false; }
			*out_text = idx == 0 ? "Default" : vector.at(idx - 1).skin_name.c_str();
			return true;
		};

		static auto vector_getter_ward_skin = [](void* vec, int idx, const char** out_text) {
			auto& vector = *static_cast<std::vector<std::pair<int32_t, std::string>>*>(vec);
			if (idx < 0 || idx > static_cast<int>(vector.size())) { return false; }
			*out_text = idx == 0 ? "Default" : vector.at(idx - 1).second.c_str();
			return true;
		};

		auto player = *reinterpret_cast<AIBaseCommon**>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::global::Player);
		if (player) {
			ImGui::TextColored({ 0.92f, 0.55f, 0.21f, 1.00f }, "Average: %.1f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
			ImGui::TextColored({ 0.92f, 0.55f, 0.21f, 1.00f }, "FPS: %.1f FPS", ImGui::GetIO().Framerate);
			ImGui::TextColored({ 0.13f, 0.92f, 0.13f, 1.00f }, "Coded By R3nzTheCodeGOD");
			if (ImGui::Button("unhook", { 130, 25 })) R3nzSkin::run = false;
			ImGui::Separator();

			auto& values = skin_database::champions_skins[fnv::hash_runtime(player->get_character_data_stack()->base_skin.model.str)];
			ImGui::Text("Localplayer skins settings:");

			if (ImGui::Combo("Current skin", &config::current_combo_skin_index, vector_getter_skin, static_cast<void*>(&values), values.size() + 1))
				if (config::current_combo_skin_index > 0)
					player->change_skin(values[config::current_combo_skin_index - 1].model_name.c_str(), values[config::current_combo_skin_index - 1].skin_id);

			if (ImGui::Combo("Current ward skin", &config::current_combo_ward_index, vector_getter_ward_skin, static_cast<void*>(&skin_database::wards_skins), skin_database::wards_skins.size() + 1))
				config::current_ward_skin_index = config::current_combo_ward_index == 0 ? -1 : skin_database::wards_skins.at(config::current_combo_ward_index - 1).first;

			ImGui::Separator();
		}
	}
	ImGui::End();
}