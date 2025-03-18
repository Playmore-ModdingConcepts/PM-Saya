#include "SDK/Classes/PalMasterDataTables.h"

namespace Palworld {
    UECustom::UDataTable* UPalMasterDataTables::GetItemRecipeDataTable()
    {
        auto DataTable = this->GetValuePtrByPropertyNameInChain<UECustom::UDataTable*>(STR("ItemRecipeDataTable"));
        if (DataTable)
        {
            return *DataTable;
        }
        return nullptr;
    }
}