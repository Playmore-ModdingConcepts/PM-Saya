#pragma once

#include "SDK/Classes/UDataTable.h"
#include "Unreal/Core/Containers/Array.hpp"

namespace UECustom {
    class UCompositeDataTable : public UECustom::UDataTable {
    public:
        static RC::Unreal::UClass* StaticClass();
    public:
        RC::Unreal::TArray<RC::Unreal::TObjectPtr<UECustom::UDataTable>> GetParentTables();
    };
}