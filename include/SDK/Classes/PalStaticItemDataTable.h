#pragma once

#include "Unreal/UObject.hpp"

namespace Palworld {
    class UPalStaticItemDataAsset;

    class UPalStaticItemDataTable : public RC::Unreal::UObject {
    public:
        UPalStaticItemDataAsset* GetDataAsset();
    };
}