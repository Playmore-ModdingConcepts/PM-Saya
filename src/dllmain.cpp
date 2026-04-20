#include "Mod/CppUserModBase.hpp"
#include "UE4SSProgram.hpp"
#include "Unreal/AActor.hpp"
#include "Unreal/UClass.hpp"
#include "Unreal/UFunction.hpp"
#include "Loader/PalMainLoader.h"
#include "Tools/EnumSchemaDefinitionGenerator.h"
#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "SDK/Classes/Async.h"
#include "SDK/UnrealOffsets.h"
#include "SDK/StaticClassStorage.h"

using namespace RC;
using namespace RC::Unreal;

class PalSchema : public RC::CppUserModBase
{
public:
    PalSchema() : CppUserModBase()
    {
        ModName = STR("SAYA - [R1]");
        ModVersion = STR("0.5.2");
        ModDescription = STR("Playmore SAYA Module Revision 1 -  based on PalSchema");
        ModAuthors = STR("- Joint collaboration by Okaetsu and Rythus");

        auto config = PS::PSConfig::Get();
        config->Load();

        PS::Log<RC::LogLevel::Normal>(STR("{} {} {} loaded.\n"), ModDescription, ModVersion, ModAuthors);
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
