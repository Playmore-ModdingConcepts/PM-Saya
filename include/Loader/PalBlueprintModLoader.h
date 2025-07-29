#pragma once

#include "Loader/PalModLoaderBase.h"
#include "Loader/Blueprint/PalBlueprintMod.h"
#include "Unreal/NameTypes.hpp"
#include "Unreal/UObjectArray.hpp"
#include <unordered_map>
#include <safetyhook.hpp>

namespace UECustom {
    class UBlueprintGeneratedClass;
}

namespace Palworld {
    class PalBlueprintModLoader : public PalModLoaderBase {
    public:
        PalBlueprintModLoader();

        ~PalBlueprintModLoader();

        // Calls UE functions, make sure unreal has done init
        virtual void Load(const nlohmann::json& Data) override final;

        // Does not call UE functions, therefore safe to call whenever
        void LoadSafe(const nlohmann::json& Data);

        void Initialize();

        void OnPostLoadDefaultObject(RC::Unreal::UClass* This, RC::Unreal::UObject* DefaultObject);
    private:
        std::unordered_map<RC::StringType, std::vector<PalBlueprintMod>> BPModRegistry;

        std::vector<PalBlueprintMod>& GetModsForBlueprint(const RC::StringType& Name);
        
        void ApplyMod(const PalBlueprintMod& BPMod, RC::Unreal::UObject* Object);

        void ApplyMod(const nlohmann::json& Data, RC::Unreal::UObject* Object);

        void ModifyBlueprintComponent(UECustom::UBlueprintGeneratedClass* BPClass, const RC::StringType& ComponentName, const nlohmann::json& ComponentData);
    };
}