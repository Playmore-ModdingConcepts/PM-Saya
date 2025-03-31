#include "Unreal/UClass.hpp"
#include "SDK/Classes/UDataTable.h"
#include "Unreal/UObjectGlobals.hpp"
#include "Unreal/UScriptStruct.hpp"
#include "Unreal/FProperty.hpp"
#include "Unreal/NameTypes.hpp"
#include "Utility/DataTableHelper.h"
#include "Utility/Logging.h"
#include "Helpers/String.hpp"
#include "Loader/PalRawTableLoader.h"

using namespace RC;
using namespace RC::Unreal;

namespace fs = std::filesystem;

namespace Palworld {
	PalRawTableLoader::PalRawTableLoader() : PalModLoaderBase("raw") {}

	PalRawTableLoader::~PalRawTableLoader() {}

	void PalRawTableLoader::Initialize() {}

    void PalRawTableLoader::Apply(UECustom::UDataTable* Table)
    {
        auto Name = Table->GetNamePrivate().ToString();

        if (Name == STR("None"))
        {
            return;
        }

        m_tableMap.emplace(Name, Table);

        static auto CompositeDataTableClass = UObjectGlobals::StaticFindObject<UClass*>(nullptr, nullptr, STR("/Script/Engine.CompositeDataTable"));
        if (Table->GetClassPrivate() == CompositeDataTableClass) return;

        RC::StringType Suffix = STR("_Common");
        size_t Pos = Name.rfind(Suffix);

        if (Pos != RC::StringType::npos && Pos + Suffix.size() == Name.size()) {
            Name.erase(Pos);
        }

        // Temporary fix for DT_PassiveSkill_Main
        // "AddPal" and similar boolean flags are stored in BP_PalPassiveSkillManager and with current Pal Schema logic, it does not load DT_PassiveSkill_Main early enough.
        // This results in Pals not being able to obtain customized Passives despite the "AddPal" being set to true in the Data Table.
        // Proper fix will most likely have to wait for FScriptMapHelper addition.
        if (m_isUnrealReady || Name == STR("DT_PassiveSkill_Main"))
        {
            auto It = m_tableDataMap.find(Name);
            if (It != m_tableDataMap.end())
            {
                LoadResult Result{};

                for (auto& Data : It->second)
                {
                    Apply(Data, Table, Result);
                }

                PS::Log<LogLevel::Normal>(STR("{}: {} rows updated, {} rows added, {} rows deleted, {} error{}.\n"),
                    Name, Result.SuccessfulModifications, Result.SuccessfulAdditions,
                    Result.SuccessfulDeletions, Result.ErrorCount, Result.ErrorCount > 1 || Result.ErrorCount == 0 ? STR("s") : STR(""));
            }
            m_tableDataMap.erase(Name);
        }
    }

    void PalRawTableLoader::Apply(const nlohmann::json& Data, UECustom::UDataTable* Table, LoadResult& OutResult)
    {
        auto Name = Table->GetNamePrivate().ToString();
        for (auto& [row_key, row_data] : Data.items())
        {
            auto RowStruct = Table->GetRowStruct().UnderlyingObjectPointer;
            if (!row_key.contains("*"))
            {
                auto RowKey = FName(RC::to_generic_string(row_key), FNAME_Add);
                auto Row = Table->FindRowUnchecked(RowKey);
                if (Row)
                {
                    if (row_data.is_null())
                    {
                        Table->RemoveRow(RowKey);
                        OutResult.SuccessfulDeletions++;
                    }
                    else
                    {
                        try
                        {
                            for (auto Property : RowStruct->ForEachPropertyInChain())
                            {
                                auto PropertyName = RC::to_string(Property->GetName());
                                if (row_data.contains(PropertyName))
                                {
                                    DataTableHelper::CopyJsonValueToTableRow(Row, Property, row_data.at(PropertyName));
                                }
                            }

                            OutResult.SuccessfulModifications++;
                        }
                        catch (const std::exception& e)
                        {
                            OutResult.ErrorCount++;
                            PS::Log<LogLevel::Error>(STR("Failed to modify Row '{}' in {}: {}\n"), RowKey.ToString(), Name, RC::to_generic_string(e.what()));
                        }
                    }
                }
                else
                {
                    auto NewRowData = FMemory::Malloc(RowStruct->GetStructureSize());
                    RowStruct->InitializeStruct(NewRowData);

                    try
                    {
                        for (auto Property : RowStruct->ForEachPropertyInChain())
                        {
                            auto PropertyName = RC::to_string(Property->GetName());
                            if (row_data.contains(PropertyName))
                            {
                                DataTableHelper::CopyJsonValueToTableRow(NewRowData, Property, row_data.at(PropertyName));
                            }
                        }

                        Table->AddRow(RowKey, *reinterpret_cast<UECustom::FTableRowBase*>(NewRowData));

                        OutResult.SuccessfulAdditions++;
                    }
                    catch (const std::exception& e)
                    {
                        OutResult.ErrorCount++;
                        FMemory::Free(NewRowData);
                        PS::Log<LogLevel::Error>(STR("Failed to add Row '{}' in {}: {}\n"), RowKey.ToString(), Name, RC::to_generic_string(e.what()));
                    }
                }
            }
            else
            {
                try
                {
                    for (auto& [Key, Row] : Table->GetRowMap())
                    {
                        for (auto Property : RowStruct->ForEachPropertyInChain())
                        {
                            auto PropertyName = RC::to_string(Property->GetName());
                            if (row_data.contains(PropertyName))
                            {
                                DataTableHelper::CopyJsonValueToTableRow(Row, Property, row_data.at(PropertyName));
                            }
                        }

                        OutResult.SuccessfulModifications++;
                    }
                }
                catch (const std::exception& e)
                {
                    OutResult.ErrorCount++;
                    PS::Log<LogLevel::Error>(STR("Failed to do wildcard modification in {}: {}\n"), Name, RC::to_generic_string(e.what()));
                }
            }
        }
    }

    void PalRawTableLoader::ApplyLate()
    {
        for (auto& [Name, Table] : m_tableMap)
        {
            auto It = m_tableDataMap.find(Name);
            if (It != m_tableDataMap.end())
            {
                LoadResult Result{};

                for (auto& Data : It->second)
                {
                    Apply(Data, Table, Result);
                }

                PS::Log<LogLevel::Normal>(STR("{}: {} rows updated, {} rows added, {} rows deleted, {} error{}.\n"),
                    Name, Result.SuccessfulModifications, Result.SuccessfulAdditions,
                    Result.SuccessfulDeletions, Result.ErrorCount, Result.ErrorCount > 1 || Result.ErrorCount == 0 ? STR("s") : STR(""));
            }
            m_tableDataMap.erase(Name);
        }
    }

	void PalRawTableLoader::Load(const nlohmann::json& Data)
	{
        for (auto& [table_key, table_data] : Data.items())
        {
            auto TableKey = RC::to_generic_string(table_key);
            AddData(TableKey, table_data);
        }
	}

    void PalRawTableLoader::Reload(const nlohmann::json& Data)
    {
        for (auto& [table_key, table_data] : Data.items())
        {
            auto TableKey = RC::to_generic_string(table_key);
            auto TableIt = m_tableMap.find(TableKey);
            if (TableIt != m_tableMap.end())
            {
                auto Table = TableIt->second;
                auto Name = Table->GetNamePrivate().ToString();
                LoadResult Result;
                Apply(table_data, Table, Result);

                PS::Log<LogLevel::Normal>(STR("{}: {} rows updated, {} rows added, {} rows deleted, {} error{}.\n"),
                    Name, Result.SuccessfulModifications, Result.SuccessfulAdditions,
                    Result.SuccessfulDeletions, Result.ErrorCount, Result.ErrorCount > 1 || Result.ErrorCount == 0 ? STR("s") : STR(""));
            }
        }
    }

    void PalRawTableLoader::SetIsUnrealReady(bool IsReady)
    {
        m_isUnrealReady = IsReady;
    }

    void PalRawTableLoader::AddData(const RC::StringType& TableName, const nlohmann::json& Data)
    {
        auto It = m_tableDataMap.find(TableName);
        if (It != m_tableDataMap.end())
        {
            It->second.push_back(Data);
        }
        else
        {
            std::vector<nlohmann::json> NewDataArray{
                Data
            };
            m_tableDataMap.emplace(TableName, NewDataArray);
        }
    }
}