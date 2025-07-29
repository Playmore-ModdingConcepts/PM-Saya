#pragma once

#include "Unreal/NameTypes.hpp"
#include "Loader/PalModLoaderBase.h"
#include "nlohmann/json.hpp"

namespace UECustom {
    class UDataAsset;
}

namespace RC::Unreal {
    class UEnum;
}

namespace Palworld {
    class PalEnumLoader : public PalModLoaderBase {
    public:
        PalEnumLoader();

        ~PalEnumLoader();

        void Initialize();

        virtual void Load(const nlohmann::json& Data) override final;
    private:
        std::unordered_map<RC::StringType, RC::Unreal::UEnum*> EnumNameToObjectMap;

        RC::Unreal::UEnum* GetEnumByName(const RC::StringType& Name);
    };
}