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

using namespace RC;
using namespace RC::Unreal;

class PalSchema : public RC::CppUserModBase
{
public:
    PalSchema() : CppUserModBase()
    {
        ModName = STR("PalSchema");
        ModVersion = STR("0.3.0-beta");
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
