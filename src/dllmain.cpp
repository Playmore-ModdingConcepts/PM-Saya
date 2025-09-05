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
#include "SDK/Helper/BPGeneratedClassHelper.h"
#include "SDK/Classes/Custom/UBlueprintGeneratedClass.h"
#include "SDK/Classes/Custom/UInheritableComponentHandler.h"
#include "SDK/UnrealOffsets.h"
#include "SDK/Classes/UDataTable.h"
#include "SDK/StaticClassStorage.h"

using namespace RC;
using namespace RC::Unreal;

class PalSchema : public RC::CppUserModBase
{
public:
    PalSchema() : CppUserModBase()
    {
        ModName = STR("PM-Saya");
        ModVersion = STR("0.4.2");
        ModDescription = STR("");
        ModAuthors = STR("Okaetsu");

        PS::PSConfig::Load();

        PS::Log<LogLevel::Verbose>(STR("Initializing SignatureManager...\n"));
        Palworld::SignatureManager::Initialize();

        MainLoader.PreInitialize();

        PS::Log<RC::LogLevel::Normal>(STR("{} v{} by {} loaded.\n"), ModName, ModVersion, ModAuthors);
    }

    ~PalSchema() override
    {
    }

    auto reload_mods() -> void
    {
        // MainLoader.ReloadMods();
    }

    auto on_ui_init() -> void override
    {
        if (UE4SSProgram::settings_manager.Debug.DebugConsoleVisible)
        {
            UE4SS_ENABLE_IMGUI()

            register_tab(ModName, [](CppUserModBase* instance) {
                auto mod = dynamic_cast<PalSchema*>(instance);
                if (!mod)
                {
                    return;
                }

                if (ImGui::Button("Reload Schema Mods"))
                {
                    mod->reload_mods();
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
        Hook::RegisterProcessEventPostCallback([&](UObject* Context, UFunction* Function, void* Parms) {
            if (m_hooked) return;
            m_hooked = true;
            MainLoader.Initialize();
        });
    }
private:
    Palworld::PalMainLoader MainLoader;
    bool m_hooked = false;
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
