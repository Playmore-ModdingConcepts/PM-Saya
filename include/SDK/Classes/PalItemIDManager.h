#pragma once

#include "Unreal/UObject.hpp"

namespace Palworld {
    class UPalStaticItemDataTable;

    class UPalItemIDManager : public RC::Unreal::UObject {
    public:
        UPalStaticItemDataTable* GetStaticItemDataTable();

        RC::Unreal::UObject* GetStaticItemData(const RC::Unreal::FName& StaticItemId);
    };
}