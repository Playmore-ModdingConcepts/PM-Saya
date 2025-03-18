#include <regex>
#include "Unreal/UClass.hpp"
#include "Unreal/UObject.hpp"
#include "Unreal/AActor.hpp"
#include "Unreal/Property/FObjectProperty.hpp"
#include "Helpers/String.hpp"
#include "Utility/DataTableHelper.h"
#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "Loader/PalBlueprintModLoader.h"
#include "SDK/Classes/KismetSystemLibrary.h"

using namespace RC;
using namespace RC::Unreal;

namespace Palworld {
    PalBlueprintModLoader::PalBlueprintModLoader() : PalModLoaderBase("blueprints")
    {
    }

    PalBlueprintModLoader::~PalBlueprintModLoader()
    {
        BPModRegistry.clear();
    }

    void PalBlueprintModLoader::Load(const nlohmann::json& Data)
    {
        for (auto& [BlueprintName, BlueprintData] : Data.items())
        {
            auto BlueprintName_Conv = RC::to_generic_string(BlueprintName);
            if (BlueprintName_Conv.starts_with(STR("/Game/")))
            {
                static const std::wregex Pattern(LR"(^(.*/)([^/.]+)$)");
                BlueprintName_Conv = std::regex_replace(BlueprintName_Conv, Pattern, STR("$1$2.$2_C"));

                auto SoftObjectPtr = UECustom::TSoftObjectPtr<UObject>(UECustom::FSoftObjectPath(BlueprintName_Conv));
                auto Asset = UECustom::UKismetSystemLibrary::LoadAsset_Blocking(SoftObjectPtr);
                if (!Asset)
                {
                    throw std::runtime_error(RC::fmt("Failed to apply blueprint changes, asset '%S' was invalid", BlueprintName_Conv.c_str()));
                }

                Asset->SetRootSet();

                auto DefaultObject = static_cast<UClass*>(Asset)->GetClassDefaultObject();
                ApplyMod(BlueprintData, DefaultObject);
            }
            else
            {
                auto BlueprintFName = FName(BlueprintName_Conv, FNAME_Add);
                auto NewBlueprintMod = PalBlueprintMod(BlueprintName, BlueprintData);
                auto ModsIt = BPModRegistry.find(BlueprintFName);
                if (ModsIt != BPModRegistry.end())
                {
                    BPModRegistry.at(BlueprintFName).push_back(NewBlueprintMod);
                }
                else
                {
                    auto NewModContainer = std::vector<PalBlueprintMod>{
                        NewBlueprintMod
                    };
                    BPModRegistry.emplace(BlueprintFName, NewModContainer);
                }
            }
        }
    }

    void PalBlueprintModLoader::Initialize()
    {
        
    }

    void PalBlueprintModLoader::OnPostLoadDefaultObject(RC::Unreal::UClass* This, RC::Unreal::UObject* DefaultObject)
    {
        if (DefaultObject)
        {
            if (Palworld::PalBlueprintModLoader::BPModRegistry.contains(This->GetNamePrivate()))
            {
                auto& Mods = PalBlueprintModLoader::GetModsForBlueprint(This->GetNamePrivate());
                for (auto& Mod : Mods)
                {
                    try
                    {
                        PalBlueprintModLoader::ApplyMod(Mod, DefaultObject);
                        PS::Log<RC::LogLevel::Normal>(STR("Applied modifications to {}\n"), Mod.GetBlueprintName().ToString());
                    }
                    catch (const std::exception& e)
                    {
                        PS::Log<RC::LogLevel::Error>(STR("Failed modifying blueprint '{}', {}\n"), Mod.GetBlueprintName().ToString(), RC::to_generic_string(e.what()));
                    }
                }
            }
        }
    }

    std::vector<PalBlueprintMod>& PalBlueprintModLoader::GetModsForBlueprint(const RC::Unreal::FName& Name)
    {
        auto Iterator = BPModRegistry.find(Name);
        if (Iterator != BPModRegistry.end())
        {
            return Iterator->second;
        }

        throw std::runtime_error(RC::fmt("Failed to get mods for this blueprint from BPModRegistry. Affected mod name: %S", Name.ToString().c_str()));
    }

    void PalBlueprintModLoader::ApplyMod(const PalBlueprintMod& BPMod, UObject* Object)
    {
        auto& Data = BPMod.GetData();
        ApplyMod(Data, Object);
    }

    void PalBlueprintModLoader::ApplyMod(const nlohmann::json& Data, RC::Unreal::UObject* Object)
    {
        UClass* Class = Object->GetClassPrivate();
        for (auto& [Key, Value] : Data.items())
        {
            auto PropertyName = RC::to_generic_string(Key);
            auto Property = Class->GetPropertyByNameInChain(PropertyName.c_str());
            if (Property)
            {
                if (auto ObjectProperty = CastField<FObjectProperty>(Property))
                {
                    auto ObjectValue = *Property->ContainerPtrToValuePtr<UObject*>(static_cast<void*>(Object));
                    if (!ObjectValue)
                    {
                        auto GEN_VAR_PATH = std::format(STR("{}:{}_GEN_VARIABLE"), Class->GetPathName(), PropertyName);
                        auto GEN_VAR_OBJECT = UObjectGlobals::StaticFindObject<UObject*>(nullptr, nullptr, GEN_VAR_PATH);
                        if (GEN_VAR_OBJECT)
                        {
                            if (Value.is_object())
                            {
                                for (auto& [InnerKey, InnerValue] : Value.items())
                                {
                                    auto GEN_VAR_OBJECT_PROPERTY_NAME = RC::to_generic_string(InnerKey);
                                    auto GEN_VAR_OBJECT_PROPERTY = GEN_VAR_OBJECT->GetPropertyByNameInChain(GEN_VAR_OBJECT_PROPERTY_NAME.c_str());
                                    if (GEN_VAR_OBJECT_PROPERTY)
                                    {
                                        Palworld::DataTableHelper::CopyJsonValueToTableRow(static_cast<void*>(GEN_VAR_OBJECT), GEN_VAR_OBJECT_PROPERTY, InnerValue);
                                    }
                                    else
                                    {
                                        PS::Log<LogLevel::Warning>(STR("Failed to modify {} because property {} doesn't exist\n"), Object->GetNamePrivate().ToString(), GEN_VAR_OBJECT_PROPERTY_NAME);
                                    }
                                }
                            }
                            else
                            {
                                PS::Log<LogLevel::Warning>(STR("{} failed to apply correctly, JSON value wasn't an object\n"), Object->GetNamePrivate().ToString());
                            }
                        }
                        else
                        {
                            Palworld::DataTableHelper::CopyJsonValueToTableRow(static_cast<void*>(Object), Property, Value);
                        }
                    }
                    else
                    {
                        Palworld::DataTableHelper::CopyJsonValueToTableRow(static_cast<void*>(Object), Property, Value);
                    }
                }
                else
                {
                    Palworld::DataTableHelper::CopyJsonValueToTableRow(static_cast<void*>(Object), Property, Value);
                }
            }
            else
            {
                PS::Log<RC::LogLevel::Warning>(STR("Property '{}' does not exist in {}\n"), PropertyName, Object->GetNamePrivate().ToString());
            }
        }
    }
}