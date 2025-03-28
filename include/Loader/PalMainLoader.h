#pragma once

#include <vector>
#include <functional>
#include "Loader/PalMonsterModLoader.h"
#include "Loader/PalHumanModLoader.h"
#include "Loader/PalLanguageModLoader.h"
#include "Loader/PalItemModLoader.h"
#include "Loader/PalSkinModLoader.h"
#include "Loader/PalAppearanceModLoader.h"
#include "Loader/PalBuildingModLoader.h"
#include "Loader/PalRawTableLoader.h"
#include "Loader/PalBlueprintModLoader.h"
#include "FileWatch.hpp"

namespace UECustom {
    class UDataTable;
}

namespace Palworld {
	class PalMainLoader {
	public:
        PalMainLoader();

        ~PalMainLoader();

		void PreInitialize();

		void Initialize();

        // Should be called in Game Thread
        void ReloadMods();
	private:
		PalLanguageModLoader LanguageModLoader;
		PalMonsterModLoader MonsterModLoader;
		PalHumanModLoader HumanModLoader;
		PalItemModLoader ItemModLoader;
		PalSkinModLoader SkinModLoader;
		PalAppearanceModLoader AppearanceModLoader;
		PalBuildingModLoader BuildingModLoader;
		PalRawTableLoader RawTableLoader;
		PalBlueprintModLoader BlueprintModLoader;

        int m_errorCount = 0;
        RC::Unreal::UFunction* m_loadBrowserFunction{};
        RC::Unreal::CallbackId m_loadBrowserFunctionCallbackId{};
        std::pair<int, int> m_titleCallbackIds{};

        std::unique_ptr<filewatch::FileWatch<std::wstring>> m_fileWatch;

        void SetupAutoReload();

		void Load();

		void LoadLanguageMods(const std::filesystem::path& path);

		void LoadPalMods(const std::filesystem::path& path);

		void LoadBuildingMods(const std::filesystem::path& path);

		void LoadRawTables(const std::filesystem::path& path);

		void LoadBlueprintMods(const std::filesystem::path& path);

		void LoadBlueprintModsSafe(const std::filesystem::path& path);

		void LoadAppearanceMods(const std::filesystem::path& path);

        void LoadItemMods(const std::filesystem::path& path);

        void LoadSkinMods(const std::filesystem::path& path);

        void IterateModsFolder(const std::function<void(const std::filesystem::directory_entry&)>& callback);

        void ParseJsonFileInPath(const std::filesystem::path& path, const std::function<void(const nlohmann::json&)>& callback);

        void ParseJsonFilesInPath(const std::filesystem::path& path, const std::function<void(const nlohmann::json&)>& callback);

        void OnPostLoadDefaultObject(RC::Unreal::UClass* This, RC::Unreal::UObject* DefaultObject);

        void DisplayErrorPopup();
    private:
        static void HandleDataTableChanged(UECustom::UDataTable* This, RC::Unreal::FName param_1);

        static void PostLoadDefaultObject(RC::Unreal::UClass* This, RC::Unreal::UObject* DefaultObject);

        static void InitGameState(RC::Unreal::AGameModeBase* This);

        static inline std::vector<std::function<void(UECustom::UDataTable*)>> HandleDataTableChangedCallbacks;
        static inline std::vector<std::function<void(RC::Unreal::UClass*, RC::Unreal::UObject*)>> PostLoadDefaultObjectCallbacks;
        static inline std::vector<std::function<void(RC::Unreal::AGameModeBase*)>> InitGameStateCallbacks;
        static inline SafetyHookInline HandleDataTableChanged_Hook;
        static inline SafetyHookInline PostLoadDefaultObject_Hook;
        static inline SafetyHookInline InitGameState_Hook;
	};
}