#include "SDK/Classes/PalStaticItemDataTable.h"
#include "SDK/Classes/PalStaticItemDataAsset.h"

namespace Palworld {
    UPalStaticItemDataAsset* UPalStaticItemDataTable::GetDataAsset()
    {
        auto DataAsset = this->GetValuePtrByPropertyNameInChain<UPalStaticItemDataAsset*>(STR("DataAsset"));
        if (DataAsset)
        {
            return *DataAsset;
        }
        return nullptr;
    }
}