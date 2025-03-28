#include <fstream>
#include <filesystem>
#include "Unreal/UClass.hpp"
#include "Unreal/UFunction.hpp"
#include "Unreal/Hooks.hpp"
#include "Helpers/String.hpp"
#include "Loader/PalMainLoader.h"
#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "SDK/Classes/UDataTable.h"
#include "SDK/Classes/PalUtility.h"
#include "SDK/PalSignatures.h"
#include "UE4SSProgram.hpp"
#include <FileWatch.hpp>
#include <SDK/Classes/Async.h>

using namespace RC;
using namespace RC::Unreal;

namespace fs = std::filesystem;

namespace Palworld {
    PalMainLoader::PalMainLoader() {}

    PalMainLoader::~PalMainLoader()
    {
        auto expected1 = HandleDataTableChanged_Hook.disable();
        HandleDataTableChanged_Hook = {};

        auto expected2 = PostLoadDefaultObject_Hook.disable();
        PostLoadDefaultObject_Hook = {};

        auto expected3 = InitGameState_Hook.disable();
        InitGameState_Hook = {};

        HandleDataTableChangedCallbacks.clear();
        PostLoadDefaultObjectCallbacks.clear();
        InitGameStateCallbacks.clear();

        if (m_loadBrowserFunction)
        {
            m_loadBrowserFunction->UnregisterHook(m_loadBrowserFunctionCallbackId);
            m_loadBrowserFunction = nullptr;
        }
    }

    void PalMainLoader::PreInitialize()
    {
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

        auto PostLoadDefaultObjectAddress = Palworld::SignatureManager::GetSignature("UBlueprintGeneratedClass::PostLoadDefaultObject");
        if (PostLoadDefaultObjectAddress)
        {
            PostLoadDefaultObject_Hook = safetyhook::create_inline(PostLoadDefaultObjectAddress,
                reinterpret_cast<void*>(PostLoadDefaultObject));

            PostLoadDefaultObjectCallbacks.push_back([&](UClass* BPGeneratedClass, UObject* DefaultObject) {
                OnPostLoadDefaultObject(BPGeneratedClass, DefaultObject);
                BlueprintModLoader.OnPostLoadDefaultObject(BPGeneratedClass, DefaultObject);
            });
        }

        auto InitGameState_Address = Palworld::SignatureManager::GetSignature("AGameModeBase::InitGameState");
        if (InitGameState_Address)
        {
            InitGameState_Hook = safetyhook::create_inline(InitGameState_Address,
                reinterpret_cast<void*>(InitGameState));

            InitGameStateCallbacks.push_back([&](AGameModeBase* Instance) {
                LanguageModLoader.Initialize();
                MonsterModLoader.Initialize();
                HumanModLoader.Initialize();
                AppearanceModLoader.Initialize();
                BuildingModLoader.Initialize();
                ItemModLoader.Initialize();
                SkinModLoader.Initialize();
                RawTableLoader.Initialize();

                Load();
            });
        }

        auto HandleDataTableChanged_Address = Palworld::SignatureManager::GetSignature("UDataTable::HandleDataTableChanged");
        if (HandleDataTableChanged_Address)
        {
            HandleDataTableChanged_Hook = safetyhook::create_inline(reinterpret_cast<void*>(HandleDataTableChanged_Address),
                HandleDataTableChanged);

            HandleDataTableChangedCallbacks.push_back([&](UECustom::UDataTable* Table) {
                RawTableLoader.Apply(Table);
            });
        }
    }

    void PalMainLoader::Initialize()
	{
        m_loadBrowserFunction = UObjectGlobals::StaticFindObject<UFunction*>(nullptr, nullptr, STR("/Script/WebBrowserWidget.WebBrowser:LoadURL"));
        if (m_loadBrowserFunction)
        {
            m_loadBrowserFunctionCallbackId = m_loadBrowserFunction->RegisterPostHook([this](UnrealScriptFunctionCallableContext& Context, void* CustomData) {
                if (m_loadBrowserFunction)
                {
                    m_loadBrowserFunction->UnregisterHook(m_loadBrowserFunctionCallbackId);
                    m_loadBrowserFunction = nullptr;
                }

                DisplayErrorPopup();
            });
        }

        if (PS::PSConfig::IsAutoReloadEnabled())
        {
            PS::Log<LogLevel::Normal>(STR("Auto-reload is enabled.\n"));
            SetupAutoReload();
        }
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
        m_fileWatch = std::make_unique<filewatch::FileWatch<std::wstring>>(
            fs::path(UE4SSProgram::get_program().get_working_directory()) / "Mods" / "PalSchema" / "mods",
            std::wregex(L".*\\.json"),
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

    void PalMainLoader::OnPostLoadDefaultObject(UClass* This, UObject* DefaultObject)
    {
        static auto NAME_WBP_Title = FName(STR("WBP_Title_C"), FNAME_Add);
        if (This->GetNamePrivate() == NAME_WBP_Title)
        {
            static bool HasHookedOnce = false;
            if (HasHookedOnce) return;

            HasHookedOnce = true;

            m_titleCallbackIds = UObjectGlobals::RegisterHook(STR("/Game/Pal/Blueprint/UI/Title/WBP_TItle.WBP_TItle_C:OnSetup"),
            [](UnrealScriptFunctionCallableContext& Context, void* CustomData) {},
            [this](UnrealScriptFunctionCallableContext& Context, void* CustomData) {
                DisplayErrorPopup();
                UObjectGlobals::UnregisterHook(STR("/Game/Pal/Blueprint/UI/Title/WBP_TItle.WBP_TItle_C:OnSetup"), m_titleCallbackIds);
            },
            nullptr);
        }
    }

    void PalMainLoader::DisplayErrorPopup()
    {
        if (m_errorCount > 0)
        {
            auto WorldContextObject = UObjectGlobals::FindFirstOf(STR("Actor"));
            if (WorldContextObject)
            {
                auto ErrorMessage = std::format(STR("<Yellow_20B>Pal Schema</>\r\n{} mod{} had errors.\r\n\r\nPlease check <NumBlue_12>UE4SS.log</> for detailed information."),
                    m_errorCount, m_errorCount > 1 ? STR("s") : STR(""));
                UPalUtility::Alert(WorldContextObject, FText(ErrorMessage));
            }
        }
    }

    void PalMainLoader::HandleDataTableChanged(UECustom::UDataTable* This, FName param_1)
    {
        HandleDataTableChanged_Hook.call(This, param_1);

        for (auto& Callback : HandleDataTableChangedCallbacks)
        {
            Callback(This);
        }
    }

    void PalMainLoader::PostLoadDefaultObject(UClass* This, UObject* DefaultObject)
    {
        PostLoadDefaultObject_Hook.call(This, DefaultObject);

        for (auto& Callback : PostLoadDefaultObjectCallbacks)
        {
            Callback(This, DefaultObject);
        }
    }

    void PalMainLoader::InitGameState(AGameModeBase* This)
    {
        InitGameState_Hook.call(This);

        for (auto& Callback : InitGameStateCallbacks)
        {
            Callback(This);
        }

        auto expected = InitGameState_Hook.disable();
        InitGameState_Hook = {};
    }
}
