#pragma once

#include <vector>
#include <functional>
#include "Loader/PalRawTableLoader.h"
#include "Loader/PalBlueprintModLoader.h"
#include "Loader/PalEnumLoader.h"
#include "FileWatch.hpp"

namespace RC::Unreal {
    class AGameModeBase;
    class UDataTable;
}

namespace UECustom {
    class UCompositeDataTable;
    class UWorldPartitionRuntimeLevelStreamingCell;
}

namespace Palworld {
    class UPalStaticItemDataTable;

	class PalMainLoader {
	public:
        PalMainLoader();

        ~PalMainLoader();

		void PreInitialize();

		void Initialize();

        // Should be called in Game Thread
        void ReloadMods();
	private:
        PalEnumLoader EnumLoader;

        std::unique_ptr<filewatch::FileWatch<std::wstring>> m_fileWatch;

        void SetupAutoReload();

        void InitCore();

        void InitLoaders();

		void LoadLanguageMods(const std::filesystem::path& path);

        void LoadCustomEnums();

        void IterateModsFolder(const std::function<void(const std::filesystem::path&, const std::filesystem::path::string_type&)>& callback);

        void ParseJsonFileInPath(const std::filesystem::path& path, const std::function<void(const nlohmann::json&)>& callback);

        void ParseJsonFilesInPath(const std::filesystem::path& path, const std::function<void(const nlohmann::json&)>& callback);
    private:
        static void PostLoad(RC::Unreal::UClass* This);

        static void GetPakFolders(const RC::Unreal::TCHAR* CmdLine, RC::Unreal::TArray<RC::Unreal::FString>* OutPakFolders);

        static void OnDataTableSerialized(RC::Unreal::UDataTable* This, RC::Unreal::FArchive* Archive);

        static void OnGameInstanceInit(RC::Unreal::UObject* This);

        static RC::Unreal::UObject* StaticItemDataTable_Get(UPalStaticItemDataTable* This, RC::Unreal::FName ItemId);

        static void UWorld_CleanupWorld(RC::Unreal::UWorld* This, bool bSessionEnded, bool bCleanupResources, RC::Unreal::UWorld* NewWorld);

        bool m_hasInit = false;
	};
}