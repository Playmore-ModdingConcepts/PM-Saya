#include "Mod/CppUserModBase.hpp"
#include "UE4SSProgram.hpp"
#include "Loader/PalMainLoader.h"
#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "SDK/PalSignatures.h"
#include "SDK/Classes/Async.h"
#include "SDK/UnrealOffsets.h"

using namespace RC;
using namespace RC::Unreal;

class PalSchema : public RC::CppUserModBase
{
public:
    PalSchema() : CppUserModBase()
    {
        ModName = STR("PalSchema");
        ModVersion = STR("0.4.3");
        ModDescription = STR("Allows modifying of Palworld's assets dynamically.");
        ModAuthors = STR("Okaetsu");

        if (!has_member_variable_layout())
        {
            PS::Log<LogLevel::Error>(STR("MemberVariableLayout.ini is missing, unable to start PalSchema. Please ensure you are using UE4SS from https://github.com/Okaetsu/RE-UE4SS/releases/tag/experimental-palworld which comes with MemberVariableLayout.ini\n"));
            return;
        }

        PS::PSConfig::Load();

        PS::Log<LogLevel::Verbose>(STR("Initializing SignatureManager...\n"));
        Palworld::SignatureManager::Initialize();

        PS::Log<LogLevel::Verbose>(STR("Initializing UnrealOffsets...\n"));
        Palworld::UnrealOffsets::Initialize();

        MainLoader.PreInitialize();

        PS::Log<RC::LogLevel::Normal>(STR("{} v{} by {} loaded.\n"), ModName, ModVersion, ModAuthors);
    }

    ~PalSchema() override
    {
    }

    auto has_member_variable_layout() -> bool
    {
        namespace fs = std::filesystem;
        auto MemberVariableLayoutFile = fs::path(UE4SSProgram::get_program().get_working_directory()) / "MemberVariableLayout.ini";
        return fs::exists(MemberVariableLayoutFile);
    }

    auto reload_mods() -> void
    {
        MainLoader.ReloadMods();
    }

    auto on_ui_init() -> void override
    {
        if (UE4SSProgram::settings_manager.Debug.DebugConsoleVisible)
        {
            UE4SS_ENABLE_IMGUI()

            register_tab(STR("Pal Schema"), [](CppUserModBase* instance) {
                auto mod = dynamic_cast<PalSchema*>(instance);
                if (!mod)
                {
                    return;
                }

                if (ImGui::Button("Reload Schema Mods"))
                {
                    UECustom::AsyncTask(UECustom::ENamedThreads::GameThread, [mod]() {
                        mod->reload_mods();
                    });
                }
            });
        }
    }

    auto on_update() -> void override
    {
    }

    auto on_program_start() -> void override
    {
    }

    auto on_unreal_init() -> void override
    {
        MainLoader.Initialize();
    }
private:
    Palworld::PalMainLoader MainLoader;
};


#define PALSCHEMA_API __declspec(dllexport)
extern "C"
{
    PALSCHEMA_API RC::CppUserModBase* start_mod()
    {
        return new PalSchema();
    }

    PALSCHEMA_API void uninstall_mod(RC::CppUserModBase* mod)
    {
        delete mod;
    }
}
