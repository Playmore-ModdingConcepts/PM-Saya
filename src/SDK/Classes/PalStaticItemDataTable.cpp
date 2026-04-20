#include "SDK/Classes/PalStaticItemDataTable.h"
#include "SDK/Classes/PalStaticItemDataAsset.h"
#include "Helpers/Casting.hpp"

using namespace RC;

namespace Palworld {
    UPalStaticItemDataAsset* UPalStaticItemDataTable::GetDataAsset()
    {
        auto DataAsset = *Helper::Casting::ptr_cast<UPalStaticItemDataAsset**>(this, 0x28);
        return DataAsset;
    }
}