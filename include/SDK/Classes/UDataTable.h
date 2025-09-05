#pragma once

#include <Unreal/UObject.hpp>
#include <Unreal/UScriptStruct.hpp>
#include <Unreal/FString.hpp>
#include <Unreal/NameTypes.hpp>
#include <Unreal/Core/Containers/Map.hpp>
#include <Unreal/Core/Templates/Function.hpp>
#include <DynamicOutput/DynamicOutput.hpp>
#include <Unreal/TypeChecker.hpp>

namespace UECustom
{
    // Base class for all table row structs to inherit from
    struct FTableRowBase
    {
        FTableRowBase() {}
        virtual ~FTableRowBase() {}
    };

    class UDataTable : public RC::Unreal::UObject
    {
    public:
        RC::Unreal::TMap<RC::Unreal::FName, unsigned char*>& GetRowMap();
        const RC::Unreal::TMap<RC::Unreal::FName, unsigned char*>& GetRowMap() const;

        RC::Unreal::UScriptStruct* GetRowStruct();
        const RC::Unreal::UScriptStruct* GetRowStruct() const;
    public:
        // High-level methods that we implement directly

        /**
         * Function to find the row of a table given its name.
         * @param RowName The name of the row to find
         * @param ContextString String for error reporting
         * @param bWarnIfRowMissing Whether to log a warning if the row is missing
         * @return Pointer to the row data if found, nullptr otherwise
         */
        template <class T>
        T* FindRow(RC::Unreal::FName RowName, const RC::Unreal::TCHAR* ContextString, bool bWarnIfRowMissing = true) const
        {
            if (!GetRowStruct())
            {
                if (bWarnIfRowMissing)
                {
                    RC::Output::send<RC::LogLevel::Error>(STR("UDataTable::FindRow : '{}' specified no RowStruct for DataTable '{}'.\n"),
                        ContextString, GetPathName());
                }
                return nullptr;
            }

            if (!GetRowStruct()->IsChildOf(T::StaticStruct()))
            {
                if (bWarnIfRowMissing)
                {
                    RC::Output::send<RC::LogLevel::Error>(STR("UDataTable::FindRow : '{}' specified incorrect type for DataTable '{}'.\n"),
                        ContextString, GetPathName());
                }
                return nullptr;
            }

            if (RowName == RC::Unreal::NAME_None)
            {
                if (bWarnIfRowMissing)
                {
                    RC::Output::send<RC::LogLevel::Warning>(STR("UDataTable::FindRow : '{}' requested invalid row 'None' from DataTable '{}'.\n"),
                        ContextString, GetPathName());
                }
                return nullptr;
            }

            const RC::Unreal::TMap<RC::Unreal::FName, unsigned char*>& RowMapRef = GetRowMap();
            unsigned char* const* RowDataPtr = RowMapRef.Find(RowName);
            if (RowDataPtr == nullptr)
            {
                if (bWarnIfRowMissing)
                {
                    RC::Output::send<RC::LogLevel::Warning>(STR("UDataTable::FindRow : '{}' requested row '{}' not in DataTable '{}'.\n"),
                        ContextString, RowName.ToString(), GetPathName());
                }
                return nullptr;
            }

            unsigned char* RowData = *RowDataPtr;
            return reinterpret_cast<T*>(RowData);
        }

        template <class T>
        T* FindRow(RC::Unreal::FName RowName, const RC::StringType& ContextString, bool bWarnIfRowMissing = true) const
        {
            return FindRow<T>(RowName, ContextString, bWarnIfRowMissing);
        }

        /**
         * High performance version with no type safety
         */
        RC::Unreal::uint8* FindRowUnchecked(RC::Unreal::FName RowName) const
        {
            if (!GetRowStruct())
            {
                return nullptr;
            }

            const RC::Unreal::TMap<RC::Unreal::FName, unsigned char*>& RowMapRef = GetRowMap();
            unsigned char* const* RowDataPtr = RowMapRef.Find(RowName);

            if (RowDataPtr == nullptr)
            {
                return nullptr;
            }

            return *RowDataPtr;
        }

        /**
         * Get all of the rows in the table, regardless of name
         */
        template <class T>
        void GetAllRows(const RC::Unreal::TCHAR* ContextString, RC::Unreal::TArray<T*>& OutRowArray) const
        {
            if (!GetRowStruct())
            {
                RC::Output::send<RC::LogLevel::Error>(STR("UDataTable::GetAllRows : DataTable '{}' has no RowStruct specified ({}).\n"),
                    GetPathName(), ContextString);
                return;
            }

            if (!GetRowStruct()->IsChildOf(T::StaticStruct()))
            {
                RC::Output::send<RC::LogLevel::Error>(STR("UDataTable::GetAllRows : Incorrect type specified for DataTable '{}' ({}).\n"),
                    GetPathName(), ContextString);
                return;
            }

            const RC::Unreal::TMap<RC::Unreal::FName, unsigned char*>& RowMapRef = GetRowMap();
            OutRowArray.Reserve(OutRowArray.Num() + RowMapRef.Num());

            for (auto& Pair : RowMapRef)
            {
                OutRowArray.Add(reinterpret_cast<T*>(Pair.Value));
            }
        }

        template <class T>
        void GetAllRows(const RC::StringType& ContextString, RC::Unreal::TArray<T*>& OutRowArray) const
        {
            GetAllRows<T>(ContextString, OutRowArray);
        }

        /**
         * Perform some operation for every row.
         */
        template <class T>
        void ForeachRow(const RC::Unreal::TCHAR* ContextString, RC::Unreal::TFunctionRef<void(const RC::Unreal::FName& Key, const T& Value)> Predicate) const
        {
            if (!GetRowStruct())
            {
                RC::Output::send<RC::LogLevel::Error>(STR("UDataTable::ForeachRow : DataTable '{}' has no RowStruct specified ({}).\n"),
                    GetPathName(), ContextString);
                return;
            }

            if (!GetRowStruct()->IsChildOf(T::StaticStruct()))
            {
                RC::Output::send<RC::LogLevel::Error>(STR("UDataTable::ForeachRow : Incorrect type specified for DataTable '{}' ({}).\n"),
                    GetPathName(), ContextString);
                return;
            }

            const RC::Unreal::TMap<RC::Unreal::FName, unsigned char*>& RowMapRef = GetRowMap();
            for (auto& Pair : RowMapRef)
            {
                T* Entry = reinterpret_cast<T*>(Pair.Value);
                Predicate(Pair.Key, *Entry);
            }
        }

        template <class T>
        void ForeachRow(const RC::StringType& ContextString, RC::Unreal::TFunctionRef<void(const RC::Unreal::FName& Key, const T& Value)> Predicate) const
        {
            ForeachRow<T>(ContextString, Predicate);
        }

        /**
         * Get an array of all the row names
         */
        RC::Unreal::TArray<RC::Unreal::FName> GetRowNames() const;

        /**
         * Implementations that work across all UE versions
         */
        void EmptyTable();
        void RemoveRow(RC::Unreal::FName RowName);
        void AddRow(RC::Unreal::FName RowName, const FTableRowBase& RowData);
        void AddRow(RC::Unreal::FName RowName, const RC::Unreal::uint8* RowData, const RC::Unreal::UScriptStruct* RowType);

    protected:
        RC::Unreal::UScriptStruct* GetEmptyUsingStruct() const;
        void RemoveRowInternal(RC::Unreal::FName RowName);
        void AddRowInternal(RC::Unreal::FName RowName, RC::Unreal::uint8* RowDataPtr);
    };
}