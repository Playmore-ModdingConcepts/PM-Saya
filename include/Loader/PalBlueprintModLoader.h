#pragma once

#include "Loader/PalModLoaderBase.h"
#include "Loader/Blueprint/PalBlueprintMod.h"
#include "Unreal/NameTypes.hpp"
#include "Unreal/UObjectArray.hpp"
#include <unordered_map>
#include <safetyhook.hpp>

namespace Palworld {
    class PalBlueprintModLoader : public PalModLoaderBase {
    public:
        PalBlueprintModLoader();

        ~PalBlueprintModLoader();

        virtual void Load(const nlohmann::json& Data) override final;

        void Initialize();

        void OnPostLoadDefaultObject(RC::Unreal::UClass* This, RC::Unreal::UObject* DefaultObject);
    private:
        std::unordered_map<RC::Unreal::FName, std::vector<PalBlueprintMod>> BPModRegistry;

        std::vector<PalBlueprintMod>& GetModsForBlueprint(const RC::Unreal::FName& Name);
        
        void ApplyMod(const PalBlueprintMod& BPMod, RC::Unreal::UObject* Object);

        void ApplyMod(const nlohmann::json& Data, RC::Unreal::UObject* Object);
    };
}