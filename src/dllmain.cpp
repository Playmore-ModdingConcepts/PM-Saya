#include "Mod/CppUserModBase.hpp"
#include "UE4SSProgram.hpp"
#include "Unreal/AActor.hpp"
#include "Unreal/UClass.hpp"
#include "Unreal/UFunction.hpp"
#include "Loader/PalMainLoader.h"
#include "Tools/EnumSchemaDefinitionGenerator.h"
#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "SDK/PalSignatures.h"
#include "SDK/Classes/Async.h"

using namespace RC;
using namespace RC::Unreal;

class PalSchema : public RC::CppUserModBase
{
public:
    PalSchema() : CppUserModBase()
    {
        ModName = STR("PalSchema");
        ModVersion = STR("0.3.3");
        ModDescription = STR("Allows modifying of Palworld's DataTables and DataAssets dynamically.");
        ModAuthors = STR("Okaetsu");

        PS::PSConfig::Load();
        Palworld::SignatureManager::Initialize();
        MainLoader.PreInitialize();

        PS::Log<RC::LogLevel::Verbose>(STR("{} v{} by {} loaded.\n"), ModName, ModVersion, ModAuthors);
    }

    ~PalSchema() override
    {
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
