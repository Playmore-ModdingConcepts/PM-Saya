#include <SDK/Classes/UDataTable.h>
#include <Unreal/UClass.hpp>
#include <Unreal/FMemory.hpp>
#include <Unreal/UnrealVersion.hpp>
#include <Unreal/VersionedContainer/Container.hpp>
#include <Helpers/Casting.hpp>

using namespace RC;
using namespace RC::Unreal;

namespace UECustom
{
        RC::Unreal::TMap<RC::Unreal::FName, unsigned char*>& UDataTable::GetRowMap()
        {
            return *Helper::Casting::ptr_cast<TMap<FName, unsigned char*>*>(this, 0x30);
        }
        const RC::Unreal::TMap<RC::Unreal::FName, unsigned char*>& UDataTable::GetRowMap() const
        {
            return *Helper::Casting::ptr_cast<const RC::Unreal::TMap<RC::Unreal::FName, unsigned char*>*>(this, 0x30);
        }

        UScriptStruct* UDataTable::GetRowStruct()
        {
            return *Helper::Casting::ptr_cast<UScriptStruct**>(this, 0x28);
        }
        const UScriptStruct* UDataTable::GetRowStruct() const
        {
            return *Helper::Casting::ptr_cast<UScriptStruct**>(this, 0x28);
        }

        UScriptStruct* UDataTable::GetEmptyUsingStruct() const
        {
            const UScriptStruct* RowStructPtr = GetRowStruct();
            if (!RowStructPtr)
            {
                // Use FTableRowBase as fallback
                static UScriptStruct* TableRowBaseStruct = nullptr;
                if (!TableRowBaseStruct)
                {
                    // Try to find FTableRowBase struct in the engine
                    TableRowBaseStruct = static_cast<UScriptStruct*>(UObjectGlobals::StaticFindObject(
                        UScriptStruct::StaticClass(),
                        nullptr,
                        STR("/Script/Engine.TableRowBase")));

                    if (!TableRowBaseStruct)
                    {
                        // If we can't find it, create a minimal struct
                        throw std::runtime_error("FTableRowBase struct not found and RowStruct is null");
                    }
                }
                return TableRowBaseStruct;
            }
            return const_cast<UScriptStruct*>(RowStructPtr);
        }

    void UDataTable::EmptyTable()
    {
        UScriptStruct* EmptyUsingStruct = GetEmptyUsingStruct();

        // Get mutable reference to RowMap
        TMap<RC::Unreal::FName, unsigned char*>& MutableRowMap = GetRowMap();

        // Iterate over all rows in table and free mem
        for (auto& Pair : MutableRowMap)
        {
            uint8* RowData = Pair.Value;
            EmptyUsingStruct->DestroyStruct(RowData);
            FMemory::Free(RowData);
        }

        // Finally empty the map
        MutableRowMap.Empty();
    }

    void UDataTable::RemoveRow(RC::Unreal::FName RowName)
    {
        RemoveRowInternal(RowName);
    }

    void UDataTable::RemoveRowInternal(RC::Unreal::FName RowName)
    {
        UScriptStruct* EmptyUsingStruct = GetEmptyUsingStruct();

        TMap<RC::Unreal::FName, unsigned char*>& MutableRowMap = GetRowMap();
        uint8* RowData = nullptr;

        if (MutableRowMap.RemoveAndCopyValue(RowName, RowData))
        {
            if (RowData)
            {
                EmptyUsingStruct->DestroyStruct(RowData);
                FMemory::Free(RowData);
            }
        }
    }

    void UDataTable::AddRow(RC::Unreal::FName RowName, const FTableRowBase& RowData)
    {
        UScriptStruct* EmptyUsingStruct = GetEmptyUsingStruct();

        // We want to delete the row memory even if it already exists
        RemoveRowInternal(RowName);

        uint8* NewRawRowData = (uint8*)FMemory::Malloc(EmptyUsingStruct->GetSize());

        EmptyUsingStruct->InitializeStruct(NewRawRowData);
        EmptyUsingStruct->CopyScriptStruct(NewRawRowData, &RowData);

        // Add to map
        AddRowInternal(RowName, NewRawRowData);
    }

    void UDataTable::AddRow(RC::Unreal::FName RowName, const uint8* RowData, const UScriptStruct* RowType)
    {
        UScriptStruct* EmptyUsingStruct = GetEmptyUsingStruct();

        if (RowType != EmptyUsingStruct)
        {
            Output::send<LogLevel::Error>(STR("AddRow called with an incompatible row type! Got '{}', but expected '{}'\n"),
                RowType->GetPathName(), EmptyUsingStruct->GetPathName());
            return;
        }

        // We want to delete the row memory even if it already exists
        RemoveRowInternal(RowName);

        uint8* NewRawRowData = (uint8*)FMemory::Malloc(EmptyUsingStruct->GetSize());

        EmptyUsingStruct->InitializeStruct(NewRawRowData);
        EmptyUsingStruct->CopyScriptStruct(NewRawRowData, RowData);

        // Add to map
        AddRowInternal(RowName, NewRawRowData);
    }

    void UDataTable::AddRowInternal(FName RowName, uint8* RowDataPtr)
    {
        TMap<RC::Unreal::FName, unsigned char*>& MutableRowMap = GetRowMap();
        MutableRowMap.Add(RowName, RowDataPtr);
    }

    TArray<FName> UDataTable::GetRowNames() const
    {
        TArray<RC::Unreal::FName> RowNames;
        const TMap<RC::Unreal::FName, unsigned char*>& RowMapRef = GetRowMap();

        RowNames.Reserve(RowMapRef.Num());
        for (auto& Pair : RowMapRef)
        {
            RowNames.Add(Pair.Key);
        }

        return RowNames;
    }
}