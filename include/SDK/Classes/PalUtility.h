#pragma once

#include "Unreal/UObject.hpp"
#include "Unreal/FText.hpp"

namespace Palworld {
    class UPalMasterDataTables;
    class UPalItemIDManager;

    class UPalUtility : public RC::Unreal::UObject {
    public:
        static UPalMasterDataTables* GetMasterDataTables(RC::Unreal::UObject* WorldContextObject);

        static void Alert(RC::Unreal::UObject* WorldContextObject, const RC::Unreal::FText& Message);

        static UPalItemIDManager* GetItemIDManager(RC::Unreal::UObject* WorldContextObject);
    private:
        static UPalUtility* GetDefaultObject();
    };
}