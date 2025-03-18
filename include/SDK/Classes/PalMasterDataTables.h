#pragma once

#include "Unreal/UObject.hpp"

namespace UECustom {
    class UDataTable;
}

namespace Palworld {
    class UPalMasterDataTables : public RC::Unreal::UObject {
    public:
        UECustom::UDataTable* GetItemRecipeDataTable();
    };
}