#include "Helpers/String.hpp"
#include "Helpers/Casting.hpp"
#include "Unreal/FProperty.hpp"
#include "Unreal/UFunction.hpp"
#include "Unreal/UScriptStruct.hpp"
#include "Unreal/BPMacros.hpp"
#include "SDK/Structs/FTableRowBase.h"
#include "SDK/Classes/UDataTable.h"
#include "SDK/Classes/Custom/UObjectGlobals.h"

using namespace RC;
using namespace RC::Unreal;

namespace UECustom {
	UScriptStruct& UDataTable::GetEmptyUsingStruct() const
	{
		UScriptStruct* EmptyUsingStruct = this->GetRowStruct().Get();

		// ::StaticStruct() appears to be created as part of codegen from Unreal USTRUCT/GENERATED_BODY... 
		// Not sure if we have a good way to replicate this functionality. 

		/*
		if (!EmptyUsingStruct)
		{
			if (!HasAnyFlags(RF_ClassDefaultObject) && GetOutermost() != GetTransientPackage())
			{
				UE_LOG(LogDataTable, Error, TEXT("Missing RowStruct while emptying DataTable '%s'!"), *GetPathName());
			}
			EmptyUsingStruct = FTableRowBase::StaticStruct();
		}
		*/

		if (!EmptyUsingStruct)
		{
			Output::send<LogLevel::Warning>(STR("UDataTable::GetEmptyUsingStruct : Unable to find the underlying row struct for the DataTable. Functionality will likely not work or be undefined behavior."));
		}

		return *EmptyUsingStruct;
	}

	void UDataTable::AddRowInternal(FName RowName, uint8* RowData)
	{
		GetRowMap().Add(RowName, RowData);
	}

	void UDataTable::RemoveRowInternal(FName RowName)
	{
		UScriptStruct& EmptyUsingStruct = GetEmptyUsingStruct();

		uint8* RowData = nullptr;
		GetRowMap().RemoveAndCopyValue(RowName, RowData);

		if (RowData)
		{
			EmptyUsingStruct.DestroyStruct(RowData);
			FMemory::Free(RowData);
		}
	}

    UClass* UDataTable::StaticClass()
    {
        static auto Class = UECustom::UObjectGlobals::StaticFindObject<UClass*>(nullptr, nullptr, STR("/Script/Engine.DataTable"));
        return Class;
    }

	TMap<FName, uint8*>& UDataTable::GetRowMap()
	{
		return *Helper::Casting::ptr_cast<TMap<FName, uint8*>*>(this, 0x30);
	}

	const TMap<FName, uint8*>& UDataTable::GetRowMap() const
	{
		return *Helper::Casting::ptr_cast<TMap<FName, uint8*>*>(this, 0x30);
	}

	TObjectPtr<UScriptStruct>& UDataTable::GetRowStruct()
	{
		return *Helper::Casting::ptr_cast<TObjectPtr<UScriptStruct>*>(this, 0x28);
	}

	const TObjectPtr<UScriptStruct>& UDataTable::GetRowStruct() const
	{
		return *Helper::Casting::ptr_cast<const TObjectPtr<UScriptStruct>*>(this, 0x28);
	}

	void UDataTable::AddRow(RC::Unreal::FName RowName, UECustom::FTableRowBase& RowData)
	{
		UScriptStruct& EmptyUsingStruct = GetEmptyUsingStruct();

		// We want to delete the row memory even for child classes that override remove
		RemoveRowInternal(RowName);

		uint8* NewRawRowData = (uint8*)FMemory::Malloc(EmptyUsingStruct.GetStructureSize());

		EmptyUsingStruct.InitializeStruct(NewRawRowData);
		EmptyUsingStruct.CopyScriptStruct(NewRawRowData, &RowData);

		// Add to map
		AddRowInternal(RowName, NewRawRowData);
	}

	void UDataTable::RemoveRow(RC::Unreal::FName RowName)
	{
		RemoveRowInternal(RowName);
	}

	void UDataTable::GetAllRowNames(TArray<FName>& OutRowNames)
	{
		UE_BEGIN_NATIVE_FUNCTION_BODY("/Script/Engine.DataTableFunctionLibrary:GetDataTableRowNames")

		// Manual expansion of: UE_COPY_PROPERTY(this, UDataTable*)
		auto thisProperty = Function->FindProperty(FName(STR("Table")));

		if (!thisProperty)
		{
			throw std::runtime_error{ "Property not found: '""Table""'" };
		}

		*std::bit_cast<UDataTable**>(&ParamData[thisProperty->GetOffset_Internal()]) = static_cast<UDataTable*>(this);
		// End of manual expansion.

		// Intentionally don't pass in our TArray<FName>&.
		// This allows UE to create and manage its own out array that we copy to our UE4SS TArray.
		// There's some inconsistent behavior when we try and pass in our UE4SS TArray, so this way is more stable
		// at the cost of an additional copy operation/mem allocation.

		UE_CALL_FUNCTION()
		UE_COPY_OUT_PROPERTY(OutRowNames, TArray<FName>)
	}

	auto UDataTable::FindRowUnchecked(FName RowName) const -> uint8*
	{
		if (GetRowStruct().Get() == nullptr)
		{
			return nullptr;
		}
		// Cast away constness due to the const AND non-const version of GetRowMap().
		TMap<FName, unsigned char*>& rowMap = const_cast<TMap<FName, unsigned char*>&>(GetRowMap());

		// If is RowName is none, it won't find anything in the map
		uint8* const* RowDataPtr = rowMap.Find(RowName);

		if (RowDataPtr == nullptr)
		{
			return nullptr;
		}

		return *RowDataPtr;
	}
}
