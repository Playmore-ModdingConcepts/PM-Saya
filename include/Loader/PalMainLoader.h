#pragma once

#include <vector>
#include <functional>
#include "Loader/PalRawTableLoader.h"
#include "Loader/PalBlueprintModLoader.h"
#include "Loader/PalEnumLoader.h"
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
		PalRawTableLoader RawTableLoader;
		PalBlueprintModLoader BlueprintModLoader;
        PalEnumLoader EnumLoader;

        int m_errorCount = 0;

        std::unique_ptr<filewatch::FileWatch<std::wstring>> m_fileWatch;

        void SetupAutoReload();

        // Makes PM-Saya read paks from the 'PM-Saya/mods' folder. Although the paks can be anywhere, prefer for them to be put inside 'YourModName/paks'.
        // This is intended for custom assets like models or textures that are part of a schema mod which makes it easier for modders to package their mod.
        // Requires a signature for FPakPlatformFile::GetPakFolders, otherwise this feature will not be available.
        void SetupAlternativePakPathReader();

		void Load();

		void LoadRawTables(const std::filesystem::path& path);

		void LoadBlueprintMods(const std::filesystem::path& path);

		void LoadBlueprintModsSafe(const std::filesystem::path& path);

        void LoadCustomEnums();

        void IterateModsFolder(const std::function<void(const std::filesystem::directory_entry&)>& callback);

        void ParseJsonFileInPath(const std::filesystem::path& path, const std::function<void(const nlohmann::json&)>& callback);

        void ParseJsonFilesInPath(const std::filesystem::path& path, const std::function<void(const nlohmann::json&)>& callback);
    private:
        static void GetPakFolders(const RC::Unreal::TCHAR* CmdLine, RC::Unreal::TArray<RC::Unreal::FString>* OutPakFolders);

        static inline SafetyHookInline GetPakFolders_Hook;
	};
}