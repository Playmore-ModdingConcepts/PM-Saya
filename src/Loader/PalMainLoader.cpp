#include <fstream>
#include <filesystem>
#include "Unreal/CoreUObject/UObject/Class.hpp"
#include "Unreal/UFunction.hpp"
#include "Unreal/Hooks.hpp"
#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "SDK/Classes/Async.h"
#include "SDK/Classes/Custom/UDataTableStore.h"
#include "SDK/Classes/Custom/UObjectGlobals.h"
#include "SDK/Classes/UDataTable.h"
#include "SDK/Classes/UWorldPartitionRuntimeLevelStreamingCell.h"
#include "SDK/StaticClassStorage.h"
#include "SDK/UnrealOffsets.h"
#include "UE4SSProgram.hpp"
#include "Loader/PalMainLoader.h"

using namespace RC;
using namespace RC::Unreal;

namespace fs = std::filesystem;

namespace constants {
    // FOLDERS //
    constexpr std::string enumsFolder           = "enums";
}

namespace Palworld {
    PalMainLoader::PalMainLoader() {}

    PalMainLoader::~PalMainLoader()
    {
    }

    void PalMainLoader::PreInitialize()
    {
    }

    void PalMainLoader::Initialize()
	{
        LoadCustomEnums();
	}

    void PalMainLoader::ReloadMods()
    {
        PS::Log<LogLevel::Normal>(STR("Reloading mods...\n"));

        auto loadErrorCallback = [](const fs::path::string_type& modName, const std::exception& e) {
            PS::Log<LogLevel::Error>(STR("Failed to reload mod {} - {}\n"), modName, RC::to_generic_string(e.what()));
        };

        PS::Log<LogLevel::Normal>(STR("Finished reloading mods.\n"));
    }

    void PalMainLoader::InitCore()
    {
        if (m_hasInit) return;
        m_hasInit = true;

        PS::Log<LogLevel::Verbose>(STR("Initializing Static Class Storage...\n"));
        Palworld::StaticClassStorage::Initialize();

        LoadCustomEnums();

        // Should in theory be more consistent than finding a signature for BlueprintGeneratedClass::PostLoad
        auto BlueprintGeneratedClass = UECustom::UObjectGlobals::StaticFindObject<UClass*>(nullptr, nullptr, STR("/Script/Engine.BlueprintGeneratedClass"), false);
        if (!BlueprintGeneratedClass)
        {
            PS::Log<LogLevel::Error>(STR("Failed to find BlueprintGeneratedClass. Cannot hook PostLoad.\n"));
            return;
        }

        PS::Log<LogLevel::Verbose>(STR("Fetching default object for UBlueprintGeneratedClass...\n"));
        uintptr_t* BGCVTablePtr = *(uintptr_t**)BlueprintGeneratedClass->GetClassDefaultObject();
        void* PostLoadPtr = (void*)BGCVTablePtr[20];
        PS::Log<LogLevel::Verbose>(STR("Found UBlueprintGeneratedClass::PostLoad: {}\n"), PostLoadPtr);

        auto PalGameInstanceClass = UECustom::UObjectGlobals::StaticFindObject<UClass*>(nullptr, nullptr, STR("/Script/Pal.PalGameInstance"));
        if (!PalGameInstanceClass)
        {
            PS::Log<LogLevel::Error>(STR("Failed to find PalGameInstance. Cannot hook OnGameInstanceInit.\n"));
            return;
        }

        PS::Log<LogLevel::Verbose>(STR("Fetching default object for UPalGameInstance...\n"));
        uintptr_t** PGIVTablePtr = *(uintptr_t***)PalGameInstanceClass->GetClassDefaultObject();
        void* GameInstanceInitPtr = (void*)PGIVTablePtr[90];
        PS::Log<LogLevel::Verbose>(STR("Found UPalGameInstance::Init: {}\n"), GameInstanceInitPtr);

        PS::Log<LogLevel::Verbose>(STR("Initialized Core\n"));
    }

    void PalMainLoader::InitLoaders()
    {
        PS::Log<LogLevel::Verbose>(STR("Initializing UDataTable storage...\n"));
        UECustom::UDataTableStore::Initialize();

        PS::Log<LogLevel::Verbose>(STR("Initializing Loaders...\n"));
        PS::Log<LogLevel::Verbose>(STR("Initialized Loaders\n"));
        PS::Log<LogLevel::Normal>(STR("Loading mods...\n"));

        auto loadErrorCallback = [](const fs::path::string_type& modName, const std::exception& e) {
            PS::Log<LogLevel::Error>(STR("Failed to load mod {} - {}\n"), modName, RC::to_generic_string(e.what()));
        };

        PS::Log<LogLevel::Normal>(STR("Finished loading mods.\n"));
    }

    void PalMainLoader::LoadCustomEnums()
    {
        EnumLoader.Initialize();

        PS::Log<LogLevel::Verbose>(STR("Loading custom enums...\n"));
        IterateModsFolder([&](const fs::path& modPath, const fs::path::string_type& modName) {
            auto enumsFolder = modPath / constants::enumsFolder;
            ParseJsonFilesInPath(enumsFolder, [&](nlohmann::json data) {
                try
                {
                    EnumLoader.Load(data);
                }
                catch (const std::exception& e)
                {
                    PS::Log<LogLevel::Error>(STR("Failed to add custom enums from mod {} - {}\n"), modName, RC::to_generic_string(e.what()));
                }
            });
        });

        PS::Log<LogLevel::Verbose>(STR("Finished loading custom enums.\n"));
    }

    void PalMainLoader::IterateModsFolder(const std::function<void(const std::filesystem::path&, const std::filesystem::path::string_type&)>& callback)
    {
        auto cwd = fs::path(UE4SSProgram::get_program().get_game_executable_directory()) / "Game" / "Content" / "Paks" / "~DefaultMods" / "SAYA-R1";
        if (fs::exists(cwd))
        {
            for (const auto& entry : fs::directory_iterator(cwd)) {
                if (entry.is_directory())
                {
                    auto& path = entry.path();
                    auto folderName = path.stem().native();
                    callback(entry.path(), folderName);
                }
            }
        }
    }

    void PalMainLoader::ParseJsonFileInPath(const std::filesystem::path& path, const std::function<void(const nlohmann::json&)>& callback)
    {
        if (!fs::exists(path))
        {
            PS::Log<LogLevel::Error>(STR("Failed parsing mod file {} - file doesn't exist.\n"), path.native());
            return;
        }

        auto ignoreComments = path.extension() == ".jsonc";
        std::ifstream f(path);

        nlohmann::json data = nlohmann::json::parse(f, nullptr, true, ignoreComments);
        callback(data);
    }

    void PalMainLoader::ParseJsonFilesInPath(const std::filesystem::path& path, const std::function<void(const nlohmann::json&)>& callback)
    {
        if (!fs::is_directory(path))
        {
            return;
        }

        for (const auto& file : fs::directory_iterator(path)) {
            try
            {
                auto filePath = file.path();
                if (filePath.has_extension())
                {
                    if (filePath.extension() == ".json" || filePath.extension() == ".jsonc")
                    {
                        ParseJsonFileInPath(filePath, callback);
                    }
                }
            }
            catch (const std::exception& e)
            {
                PS::Log<RC::LogLevel::Error>(STR("Failed parsing mod file {} - {}.\n"), RC::to_generic_string(file.path().native()), RC::to_generic_string(e.what()));
                throw std::runtime_error(e.what());
            }
        }
    }

    void PalMainLoader::PostLoad(UClass* This)
    {
    }
}
