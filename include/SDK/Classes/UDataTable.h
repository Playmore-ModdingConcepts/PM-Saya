#pragma once

#include "Unreal/UObject.hpp"
#include "Unreal/Core/Containers/Map.hpp"
#include "Unreal/Core/Containers/Array.hpp"
#include "Unreal/NameTypes.hpp"
#include "SDK/Structs/FTableRowBase.h"

namespace RC::Unreal {
    class UScriptStruct;
}

// Copied from UE4SS for now until the UDataTable changes are pushed over to main
namespace UECustom {
    struct FTableRowBase;

	class UDataTable : public RC::Unreal::UObject {
    private:
        /**
         * @brief Gets the internal struct that the UDataTable uses to store row data.
         * @brief This is used internally to correctly set Size and Alignment of rows for UDataTable::AddRow(FName, FTableRowBase&)
         * @return UScriptStruct
        */
        auto GetEmptyUsingStruct() const -> RC::Unreal::UScriptStruct&;
    protected:
        /** Called to add rows to the data table */
        void AddRowInternal(RC::Unreal::FName RowName, RC::Unreal::uint8* RowData);

        /** Deletes the row memory */
        void RemoveRowInternal(RC::Unreal::FName RowName);
    public:
        static RC::Unreal::UClass* StaticClass();

        RC::Unreal::TMap<RC::Unreal::FName, RC::Unreal::uint8*>& GetRowMap();

        const RC::Unreal::TMap<RC::Unreal::FName, RC::Unreal::uint8*>& GetRowMap() const;

        RC::Unreal::TObjectPtr<RC::Unreal::UScriptStruct>& GetRowStruct();

        const RC::Unreal::TObjectPtr<RC::Unreal::UScriptStruct>& GetRowStruct() const;

        /**
         * @brief Add a row to the DataTable. Calls RemoveRow before adding the row.
         * @attention This function automatically assigns the FTableRowBase.Size and FTableRowBase.Alignment.
         * @attention Clients should not have to do any assignment of Size and Alignment.
         * @param RowName FName key
         * @param RowData A struct that derives from FTableRowBase.
        */
        void AddRow(RC::Unreal::FName RowName, UECustom::FTableRowBase& RowData);

        /**
         * @brief Removes a row from the DataTable.
         * @param RowName FName key
        */
        void RemoveRow(RC::Unreal::FName RowName);

        /**
         * @brief Gets all row names from the DataTable.
         * @param OutRowNames Collection of FName keys populated from the DataTable
         * @return void
        */
        void GetAllRowNames(RC::Unreal::TArray<RC::Unreal::FName>& OutRowNames);

        /**
         * @brief Fast lookup of a row without any typechecking.
         * @param RowName FName key
         * @return Pointer to row if found in the UDataTable. Returns nullptr if no row found.
        */
        RC::Unreal::uint8* FindRowUnchecked(RC::Unreal::FName RowName) const;
	};
}