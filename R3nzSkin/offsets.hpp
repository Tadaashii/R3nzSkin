#pragma once

#include <cinttypes>

namespace offsets
{
	namespace GameObject
	{
		enum
		{
			Name = 0x6C,
			Team = 0x4C
		};
	};

	namespace ComponentHost
	{
		enum
		{
			Components = 0x00
		};
	}

	namespace SummonerEmoteUserComponent
	{
		enum
		{
			Emotes = 0x000C
		};
	}

	namespace global
	{
		extern std::uint32_t Player;
		extern std::uint32_t ChampionManager;
		extern std::uint32_t Riot__g_window;
		extern std::uint32_t ManagerTemplate_AIMinionClient_;
		extern std::uint32_t ManagerTemplate_AIHero_;
		extern std::uint32_t GameClient;
	};

	namespace AIBaseCommon
	{
		extern std::uint32_t CharacterDataStack;
		extern std::uint32_t SkinId;
	};

	namespace MaterialRegistry
	{
		extern std::uint32_t D3DDevice;
		extern std::uint32_t SwapChain;
	};

	namespace AIMinionClient
	{
		extern std::uint32_t IsLaneMinion;
	};

	namespace functions
	{
		extern std::uint32_t Riot__Renderer__MaterialRegistry__GetSingletonPtr;
		extern std::uint32_t translateString_UNSAFE_DONOTUSE;
		extern std::uint32_t CharacterDataStack__Push;
		extern std::uint32_t CharacterDataStack__Update;
		extern std::uint32_t GetGoldRedirectTarget;
		extern std::uint32_t CharacterData__GetCharacterPackage;
		extern std::uint32_t SummonerEmoteUserComponent__GetSummonerEmoteData;
		extern std::uint32_t SummonerEmoteUserComponent__SetEmoteIdForSlot;
	};
};