#include <Windows.h>
#include <cinttypes>
#include <memory>
#include <string>
#include <mutex>
#include <d3d9.h>
#include <d3d11.h>

#include "d3d_hook.hpp"
#include "offsets.hpp"
#include "vmt_smart_hook.hpp"
#include "menu.hpp"
#include "R3nzSkin.hpp"
#include "skin_database.hpp"
#include "imgui_impl_dx9.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imgui_extend.h"
#include "game_classes.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool menu_is_open = false;

LONG_PTR original_wndproc;
LRESULT __stdcall wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, w_param, l_param))
		return true;

	if (msg == WM_KEYDOWN && w_param == VK_INSERT) {
		menu_is_open = !menu_is_open;
		if (!menu_is_open)
			config::save();
	}

	return CallWindowProcW((WNDPROC)original_wndproc, hwnd, msg, w_param, l_param);
}

std::once_flag init_device;
std::unique_ptr<vmt_smart_hook> d3d_device_vmt = nullptr;
std::unique_ptr<vmt_smart_hook> swap_chain_vmt = nullptr;

bool get_system_font_path(const std::string& name, std::string& path) noexcept
{
	char buffer[MAX_PATH];
	HKEY registryKey;

	GetWindowsDirectoryA(buffer, MAX_PATH);
	std::string fontsFolder = buffer + std::string("\\Fonts\\");

	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ, &registryKey)) {
		return false;
	}

	uint32_t valueIndex = 0;
	char valueName[MAX_PATH];
	uint8_t valueData[MAX_PATH];
	std::wstring wsFontFile;

	do {
		uint32_t valueNameSize = MAX_PATH;
		uint32_t valueDataSize = MAX_PATH;
		uint32_t valueType;

		auto error = RegEnumValueA(
			registryKey,
			valueIndex,
			valueName,
			reinterpret_cast<DWORD*>(&valueNameSize),
			0,
			reinterpret_cast<DWORD*>(&valueType),
			valueData,
			reinterpret_cast<DWORD*>(&valueDataSize));

		valueIndex++;

		if (error == ERROR_NO_MORE_ITEMS) {
			RegCloseKey(registryKey);
			return false;
		}

		if (error || valueType != REG_SZ) {
			continue;
		}

		if (_strnicmp(name.data(), valueName, name.size()) == 0) {
			path = fontsFolder + std::string((char*)valueData, valueDataSize);
			RegCloseKey(registryKey);
			return true;
		}
	} while (true);

	return false;
}

static const ImWchar ranges[] =
{
	0x0020, 0x00FF, // Basic Latin + Latin Supplement
	0x0400, 0x044F, // Cyrillic
	0x0100, 0x017F, // Latin Extended-A
	0x0180, 0x024F, // Latin Extended-B
	0x2000, 0x206F, // General Punctuation
	0x3000, 0x30FF, // Punctuations, Hiragana, Katakana
	0x31F0, 0x31FF, // Katakana Phonetic Extensions
	0xFF00, 0xFFEF, // Half-width characters
	0x4e00, 0x9FAF, // CJK Ideograms
	0,
};

namespace d3d_vtable {
	ID3D11Device* d3d11_device = nullptr;
	ID3D11DeviceContext* d3d11_device_context = nullptr;
	ID3D11RenderTargetView* main_render_target_view = nullptr;
	IDXGISwapChain* p_swap_chain = nullptr;
	void create_render_target() noexcept {
		ID3D11Texture2D* back_buffer;
		p_swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
		d3d11_device->CreateRenderTargetView(back_buffer, NULL, &main_render_target_view);
		back_buffer->Release();
	}

	void init_imgui(void* device, bool is_d3d11 = false) noexcept
	{
		skin_database::load();
		ImGui::CreateContext();
		ImGuiStyle& style = ImGui::GetStyle();

		style.WindowPadding = ImVec2(6, 6);
		style.FramePadding = ImVec2(6, 4);
		style.ItemSpacing = ImVec2(6, 4);
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

		style.ScrollbarSize = 10;

		style.WindowBorderSize = 0.5f;
		style.ChildBorderSize = 0.5f;
		style.PopupBorderSize = 0.5f;
		style.FrameBorderSize = 0;

		style.WindowRounding = 0;
		style.ChildRounding = 0;
		style.FrameRounding = 0;
		style.ScrollbarRounding = 0;
		style.GrabRounding = 0;
		style.TabRounding = 0;
		style.PopupRounding = 0;

		ImVec4* colors = style.Colors;

		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.81f, 0.83f, 0.81f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.93f, 0.65f, 0.14f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

		std::string font_path;
		if (get_system_font_path("Arial", font_path))
			ImGui::GetIO().Fonts->AddFontFromFileTTF(font_path.c_str(), 16, 0, ranges);

		ImGui_ImplWin32_Init(*reinterpret_cast<HWND*>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::global::Riot__g_window));

		if (is_d3d11) {
			p_swap_chain = reinterpret_cast<IDXGISwapChain*>(device);
			p_swap_chain->GetDevice(__uuidof(d3d11_device), reinterpret_cast<void**>(&(d3d11_device)));
			d3d11_device->GetImmediateContext(&d3d11_device_context);
			create_render_target();
			ImGui_ImplDX11_Init(d3d11_device, d3d11_device_context);
			ImGui_ImplDX11_CreateDeviceObjects();

		} else
			ImGui_ImplDX9_Init(reinterpret_cast<IDirect3DDevice9*>(device));
		original_wndproc = SetWindowLongPtr(*reinterpret_cast<HWND*>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::global::Riot__g_window), GWLP_WNDPROC, (LONG_PTR)wnd_proc);
	}

	void render(void* device, bool is_d3d11 = false) noexcept
	{
		auto client = *reinterpret_cast<GameClient**>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::global::GameClient);

		if (client && client->game_state == GGameState_s::Running) {
			R3nzSkin::update();

			if (menu_is_open) {
				if (is_d3d11)
					ImGui_ImplDX11_NewFrame();
				else
					ImGui_ImplDX9_NewFrame();
				ImGui_ImplWin32_NewFrame();
				ImGui::NewFrame();
				menu::draw();
				ImGui::EndFrame();
				ImGui::Render();

				if (is_d3d11) {
					d3d11_device_context->OMSetRenderTargets(1, &main_render_target_view, NULL);
					ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
				} else {
					unsigned long colorwrite, srgbwrite;
					reinterpret_cast<IDirect3DDevice9*>(device)->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
					reinterpret_cast<IDirect3DDevice9*>(device)->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgbwrite);
					reinterpret_cast<IDirect3DDevice9*>(device)->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
					reinterpret_cast<IDirect3DDevice9*>(device)->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
					ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
					reinterpret_cast<IDirect3DDevice9*>(device)->SetRenderState(D3DRS_COLORWRITEENABLE, colorwrite);
					reinterpret_cast<IDirect3DDevice9*>(device)->SetRenderState(D3DRS_SRGBWRITEENABLE, srgbwrite);
				}
			}
		}
	}

	struct dxgi_present {
		static long __stdcall hooked(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags)
		{
			std::call_once(init_device, [&]() {
				init_imgui(p_swap_chain, true);
			});
			render(p_swap_chain, true);
			return m_original(p_swap_chain, sync_interval, flags);
		}
		static decltype(&hooked) m_original;
	};
	decltype(dxgi_present::m_original) dxgi_present::m_original;

	struct dxgi_resize_buffers {
		static long __stdcall hooked(IDXGISwapChain* p_swap_chain, UINT buffer_count, UINT width, UINT height, DXGI_FORMAT new_format, UINT swap_chain_flags)
		{
			if (main_render_target_view) { main_render_target_view->Release(); main_render_target_view = nullptr; }
			auto hr = m_original(p_swap_chain, buffer_count, width, height, new_format, swap_chain_flags);
			create_render_target();
			return hr;
		}
		static decltype(&hooked) m_original;
	};
	decltype(dxgi_resize_buffers::m_original) dxgi_resize_buffers::m_original;

	struct end_scene {
		static long __stdcall hooked(IDirect3DDevice9* p_device)
		{
			std::call_once(init_device, [&]() {
				init_imgui(p_device);
			});
			render(p_device);
			return m_original(p_device);
		}
		static decltype(&hooked) m_original;
	};
	decltype(end_scene::m_original) end_scene::m_original;

	struct reset {
		static long __stdcall hooked(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* parametrs)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			auto hr = m_original(device, parametrs);
			if (hr >= 0)
				ImGui_ImplDX9_CreateDeviceObjects();
			return hr;
		}
		static decltype(&hooked) m_original;
	};
	decltype(reset::m_original) reset::m_original;
};
using namespace d3d_vtable;

void __stdcall d3d_hook::hook() noexcept
{
	auto material_registry = reinterpret_cast<uintptr_t(__stdcall*)()>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::functions::Riot__Renderer__MaterialRegistry__GetSingletonPtr)();
	auto d3d_device = *reinterpret_cast<IDirect3DDevice9**>(material_registry + offsets::MaterialRegistry::D3DDevice);
	auto swap_chain = *reinterpret_cast<IDXGISwapChain**>(material_registry + offsets::MaterialRegistry::SwapChain);

	if (d3d_device) {
		d3d_device_vmt = std::make_unique<::vmt_smart_hook>(d3d_device);
		d3d_device_vmt->apply_hook<end_scene>(42);
		d3d_device_vmt->apply_hook<reset>(16);
	} else if (swap_chain) {
		swap_chain_vmt = std::make_unique<::vmt_smart_hook>(swap_chain);
		swap_chain_vmt->apply_hook<dxgi_present>(8);
		swap_chain_vmt->apply_hook<dxgi_resize_buffers>(13);
	}
}

void __stdcall d3d_hook::unhook() noexcept
{
	SetWindowLongPtr(*reinterpret_cast<HWND*>(std::uintptr_t(GetModuleHandle(nullptr)) + offsets::global::Riot__g_window), GWLP_WNDPROC, original_wndproc);

	if (d3d_device_vmt)
		d3d_device_vmt->unhook();
	if (swap_chain_vmt)
		swap_chain_vmt->unhook();
}