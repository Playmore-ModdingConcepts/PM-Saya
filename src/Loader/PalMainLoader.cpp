#include <fstream>
#include <filesystem>
#include "Unreal/UClass.hpp"
#include "Unreal/Hooks.hpp"
#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "SDK/Classes/Async.h"
#include "SDK/Classes/Custom/UDataTableStore.h"
#include "SDK/Classes/Custom/UObjectGlobals.h"
#include "SDK/Classes/UDataTable.h"
#include "SDK/Classes/PalUtility.h"
#include "SDK/PalSignatures.h"
#include "SDK/StaticClassStorage.h"
#include "SDK/UnrealOffsets.h"
#include "UE4SSProgram.hpp"
#include "Loader/PalMainLoader.h"

using namespace RC;
using namespace RC::Unreal;

namespace fs = std::filesystem;

namespace Palworld {
    PalMainLoader::PalMainLoader() {}

    PalMainLoader::~PalMainLoader()
    {
        auto expected1 = HandleDataTableChanged_Hook.disable();
        HandleDataTableChanged_Hook = {};

        auto expected2 = InitGameState_Hook.disable();
        InitGameState_Hook = {};

        auto expected3 = PostLoad_Hook.disable();
        PostLoad_Hook = {};

        HandleDataTableChangedCallbacks.clear();
        InitGameStateCallbacks.clear();
        PostLoadCallbacks.clear();
    }

    void PalMainLoader::PreInitialize()
    {
        auto InitGameState_Address = Palworld::SignatureManager::GetSignature("AGameModeBase::InitGameState");
        if (InitGameState_Address)
        {
            InitGameState_Hook = safetyhook::create_inline(InitGameState_Address,
                reinterpret_cast<void*>(InitGameState));

            InitGameStateCallbacks.push_back([&](AGameModeBase* Instance) {
                InitLoaders();
            });
        }

        auto HandleDataTableChanged_Address = Palworld::SignatureManager::GetSignature("UDataTable::HandleDataTableChanged");
        if (HandleDataTableChanged_Address)
        {
            HandleDataTableChanged_Hook = safetyhook::create_inline(reinterpret_cast<void*>(HandleDataTableChanged_Address),
                HandleDataTableChanged);

            HandleDataTableChangedCallbacks.push_back([&](UECustom::UDataTable* Table) {
                InitCore();
                RawTableLoader.OnDataTableChanged(Table);
                UECustom::UDataTableStore::Store(Table);
            });
        }

        SetupAlternativePakPathReader();
    }

    void PalMainLoader::Initialize()
	{
        SetupAutoReload();
	}

    void PalMainLoader::ReloadMods()
    {
        IterateModsFolder([&](const fs::directory_entry& modFolder) {
            auto& modsPath = modFolder.path();
            auto modName = modsPath.stem().native();
            try
            {
                PS::Log<RC::LogLevel::Normal>(STR("Reloading mod: {}\n"), modName);

                auto palFolder = modsPath / "pals";
                LoadPalMods(palFolder);

                auto appearanceFolder = modsPath / "appearance";
                LoadAppearanceMods(appearanceFolder);

                auto buildingsFolder = modsPath / "buildings";
                LoadBuildingMods(buildingsFolder);

                auto itemsFolder = modsPath / "items";
                LoadItemMods(itemsFolder);

                auto skinsFolder = modsPath / "skins";
                LoadSkinMods(skinsFolder);

                auto translationsFolder = modsPath / "translations";
                LoadLanguageMods(translationsFolder);

                auto blueprintFolder = modFolder.path() / "blueprints";
                LoadBlueprintMods(blueprintFolder);
            }
            catch (const std::exception& e)
            {
                PS::Log<LogLevel::Error>(STR("Failed to reload mod {} - {}\n"), modName, RC::to_generic_string(e.what()));
            }
        });
    }

    void PalMainLoader::SetupAutoReload()
    {
        if (!PS::PSConfig::IsAutoReloadEnabled()) return;

        PS::Log<LogLevel::Normal>(STR("Auto-reload is enabled.\n"));

        m_fileWatch = std::make_unique<filewatch::FileWatch<std::wstring>>(
            fs::path(UE4SSProgram::get_program().get_working_directory()) / "Mods" / "PalSchema" / "mods",
            std::wregex(L".*\\.(json|jsonc)"),
            [&](const std::wstring& path, const filewatch::Event change_type) {
                if (change_type == filewatch::Event::modified)
                {
                    auto ue4ssPath = fs::path(UE4SSProgram::get_program().get_working_directory());
                    auto modFilePath = ue4ssPath / "Mods" / "PalSchema" / "mods" / path;

                    std::ifstream f(modFilePath);
                    if (f.peek() == std::ifstream::traits_type::eof()) {
                        return;
                    }
                    f.close();

                    UECustom::AsyncTask(UECustom::ENamedThreads::GameThread, [this, path, modFilePath]() {
                        auto subPath = fs::path(path);
                        auto it = subPath.begin();
                        if (std::distance(subPath.begin(), subPath.end()) >= 2) {
                            auto modName = it->native();

                            std::advance(it, 1);
                            auto folderType = it->string();
                            try
                            {
                                ParseJsonFileInPath(modFilePath, [&](const nlohmann::json& data) {
                                    if (folderType == "pals")
                                    {
                                        MonsterModLoader.Load(data);
                                    }
                                    else if (folderType == "appearance")
                                    {
                                        AppearanceModLoader.Load(data);
                                    }
                                    else if (folderType == "buildings")
                                    {
                                        BuildingModLoader.Load(data);
                                    }
                                    else if (folderType == "items")
                                    {
                                        ItemModLoader.Load(data);
                                    }
                                    else if (folderType == "skins")
                                    {
                                        SkinModLoader.Load(data);
                                    }
                                    else if (folderType == "translations")
                                    {
                                        LanguageModLoader.Load(data);
                                    }
                                    else if (folderType == "blueprints")
                                    {
                                        BlueprintModLoader.Load(data);
                                    }
                                    else if (folderType == "raw")
                                    {
                                        RawTableLoader.Reload(data);
                                    }
                                });

                                PS::Log<LogLevel::Normal>(STR("Reloaded mod {}\n"), modName);
                            }
                            catch (const std::exception& e)
                            {
                                PS::Log<LogLevel::Error>(STR("Failed to reload mod {} - {}\n"), modName, RC::to_generic_string(e.what()));
                            }
                        }
                    });
                }
            }
        );
    }

    void PalMainLoader::SetupAlternativePakPathReader()
    {
        auto GetPakFolders_Address = Palworld::SignatureManager::GetSignature("FPakPlatformFile::GetPakFolders");
        if (GetPakFolders_Address)
        {
            GetPakFolders_Hook = safetyhook::create_inline(reinterpret_cast<void*>(GetPakFolders_Address),
                GetPakFolders);
        }
    }

    void PalMainLoader::InitCore()
    {
        if (m_hasInit) return;
        m_hasInit = true;

        Palworld::StaticClassStorage::Initialize();

        LoadCustomEnums();

        std::vector<fs::path::string_type> listOfModsWithErrors;

        IterateModsFolder([&](const fs::directory_entry& modFolder) {
            auto modName = modFolder.path().stem().native();
            try
            {
                PS::Log<RC::LogLevel::Normal>(STR("Loading mod: {}\n"), modName);

                auto rawFolder = modFolder.path() / "raw";
                LoadRawTables(rawFolder);

                auto blueprintFolder = modFolder.path() / "blueprints";
                LoadBlueprintModsSafe(blueprintFolder);
            }
            catch (const std::exception&)
            {
                listOfModsWithErrors.push_back(modName);
            }
        });

        auto errorCount = listOfModsWithErrors.size();
        m_errorCount += errorCount;

        // Should in theory be more consistent than finding a signature for BlueprintGeneratedClass::PostLoad
        auto BlueprintGeneratedClass = UECustom::UObjectGlobals::StaticFindObject<UClass*>(nullptr, nullptr, STR("/Script/Engine.BlueprintGeneratedClass"), false);
        uintptr_t* VTablePtr = *(uintptr_t**)BlueprintGeneratedClass->GetClassDefaultObject();
        void* PostLoadPtr = (void*)VTablePtr[20];

        PostLoadCallbacks.push_back([&](UClass* BPGeneratedClass) {
            BlueprintModLoader.OnPostLoadDefaultObject(BPGeneratedClass, BPGeneratedClass->GetClassDefaultObject());
        });

        PostLoad_Hook = safetyhook::create_inline(PostLoadPtr,
            reinterpret_cast<void*>(PostLoad));

        PS::Log<LogLevel::Verbose>(STR("Initialized Core\n"));
    }

    void PalMainLoader::InitLoaders()
    {
        UECustom::UDataTableStore::Initialize();

        LanguageModLoader.Initialize();
        MonsterModLoader.Initialize();
        HumanModLoader.Initialize();
        AppearanceModLoader.Initialize();
        BuildingModLoader.Initialize();
        ItemModLoader.Initialize();
        SkinModLoader.Initialize();

        Load();

        PS::Log<LogLevel::Verbose>(STR("Initialized Loaders\n"));
    }

    void PalMainLoader::Load()
	{
        std::vector<fs::path::string_type> listOfModsWithErrors;

        IterateModsFolder([&](const fs::directory_entry& modFolder) {
            auto& modsPath = modFolder.path();
            auto modName = modsPath.stem().native();

            try
            {
                PS::Log<RC::LogLevel::Normal>(STR("Loading mod: {}\n"), modName);
                
                auto palFolder = modsPath / "pals";
                LoadPalMods(palFolder);

                auto appearanceFolder = modsPath / "appearance";
                LoadAppearanceMods(appearanceFolder);

                auto buildingsFolder = modsPath / "buildings";
                LoadBuildingMods(buildingsFolder);

                auto itemsFolder = modsPath / "items";
                LoadItemMods(itemsFolder);

                auto skinsFolder = modsPath / "skins";
                LoadSkinMods(skinsFolder);

                auto translationsFolder = modsPath / "translations";
                LoadLanguageMods(translationsFolder);

                auto blueprintFolder = modFolder.path() / "blueprints";
                LoadBlueprintMods(blueprintFolder);
            }
            catch (const std::exception&)
            {
                listOfModsWithErrors.push_back(modName);
            }
        });

        auto errorCount = listOfModsWithErrors.size();
        m_errorCount += errorCount;
	}

	void PalMainLoader::LoadLanguageMods(const std::filesystem::path& path)
	{
		const auto& currentLanguage = LanguageModLoader.GetCurrentLanguage();

        auto globalLanguageFolder = path / "global";
        if (fs::exists(globalLanguageFolder))
        {
            ParseJsonFilesInPath(globalLanguageFolder, [&](nlohmann::json data) {
                LanguageModLoader.Load(data);
            });
        }

        auto translationLanguageFolder = path / currentLanguage;
		if (fs::exists(translationLanguageFolder))
		{
            ParseJsonFilesInPath(translationLanguageFolder, [&](nlohmann::json data) {
                LanguageModLoader.Load(data);
            });
		}
	}

	void PalMainLoader::LoadPalMods(const std::filesystem::path& path)
	{
        ParseJsonFilesInPath(path, [&](nlohmann::json data) {
            MonsterModLoader.Load(data);
        });
	}

	void PalMainLoader::LoadBuildingMods(const std::filesystem::path& path)
	{
        ParseJsonFilesInPath(path, [&](nlohmann::json data) {
            BuildingModLoader.Load(data);
        });
	}

	void PalMainLoader::LoadAppearanceMods(const std::filesystem::path& path)
	{
        ParseJsonFilesInPath(path, [&](nlohmann::json data) {
            AppearanceModLoader.Load(data);
        });
	}

    void PalMainLoader::LoadRawTables(const std::filesystem::path& path)
    {
        ParseJsonFilesInPath(path, [&](nlohmann::json data) {
            RawTableLoader.Load(data);
        });
    }

    void PalMainLoader::LoadBlueprintMods(const std::filesystem::path& path)
    {
        ParseJsonFilesInPath(path, [&](nlohmann::json data) {
            BlueprintModLoader.Load(data);
        });
    }

    void PalMainLoader::LoadBlueprintModsSafe(const std::filesystem::path& path)
    {
        ParseJsonFilesInPath(path, [&](nlohmann::json data) {
            BlueprintModLoader.LoadSafe(data);
        });
    }

    void PalMainLoader::LoadItemMods(const std::filesystem::path& path)
    {
        ParseJsonFilesInPath(path, [&](nlohmann::json data) {
            ItemModLoader.Load(data);
        });
    }

    void PalMainLoader::LoadSkinMods(const std::filesystem::path& path)
    {
        ParseJsonFilesInPath(path, [&](nlohmann::json data) {
            SkinModLoader.Load(data);
        });
    }

    void PalMainLoader::LoadCustomEnums()
    {
        EnumLoader.Initialize();

        IterateModsFolder([&](const fs::directory_entry& modFolder) {
            auto& modsPath = modFolder.path();
            auto modName = modsPath.stem().native();
            auto enumsFolder = modFolder.path() / "enums";
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
    }

    void PalMainLoader::IterateModsFolder(const std::function<void(const std::filesystem::directory_entry&)>& callback)
    {
        auto cwd = fs::path(UE4SSProgram::get_program().get_working_directory()) / "Mods" / "PalSchema" / "mods";
        if (fs::exists(cwd))
        {
            for (const auto& entry : fs::directory_iterator(cwd)) {
                if (entry.is_directory())
                {
                    callback(entry);
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

    void PalMainLoader::HandleDataTableChanged(UECustom::UDataTable* This, FName param_1)
    {
        for (auto& Callback : HandleDataTableChangedCallbacks)
        {
            Callback(This);
        }

        HandleDataTableChanged_Hook.call(This, param_1);
    }

    void PalMainLoader::PostLoad(UClass* This)
    {
        PostLoad_Hook.call(This);

        for (auto& Callback : PostLoadCallbacks)
        {
            Callback(This);
        }
    }

    void PalMainLoader::InitGameState(AGameModeBase* This)
    {
        PS::Log<LogLevel::Verbose>(STR("Calling Original AGameModeBase::InitGameState\n"));
        InitGameState_Hook.call(This);

        PS::Log<LogLevel::Verbose>(STR("Firing AGameModeBase::InitGameState Callbacks...\n"));
        for (auto& Callback : InitGameStateCallbacks)
        {
            Callback(This);
        }

        auto expected = InitGameState_Hook.disable();
        InitGameState_Hook = {};

        PS::Log<LogLevel::Verbose>(STR("Returning from AGameModeBase::InitGameState...\n"));
    }

    void PalMainLoader::GetPakFolders(const TCHAR* CmdLine, TArray<FString>* OutPakFolders)
    {
        GetPakFolders_Hook.call(CmdLine, OutPakFolders);

        try
        {
            // Calling this here, because we want GMalloc to be available ASAP inside this hook so we can make our changes to the TArray.
            // Once UE4SS starts running things on Game Thread, this could be moved to PreInitialize.
            UnrealOffsets::InitializeGMalloc();
        }
        catch (const std::exception& e)
        {
            PS::Log<LogLevel::Error>(STR("Failed to initialize GMalloc early: {}\n"), RC::to_generic_string(e.what()));
            PS::Log<LogLevel::Error>(STR("PalSchema won't be able to load paks from the PalSchema/mods folder.\n"));
            return;
        }

        auto ModsFolderPath = fs::path(UE4SSProgram::get_program().get_working_directory()) / "Mods" / "PalSchema" / "mods";
        auto AbsolutePath = ModsFolderPath.native();
        auto AbsolutePathWithSuffix = std::format(STR("{}/"), RC::to_generic_string(AbsolutePath));

        // If GMalloc isn't properly initialized, accessing the TArray will crash.
        OutPakFolders->Add(FString(AbsolutePathWithSuffix.c_str()));
    }
}
