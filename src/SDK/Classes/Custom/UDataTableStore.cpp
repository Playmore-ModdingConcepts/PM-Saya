#include "SDK/Classes/Custom/UDataTableStore.h"
#include "SDK/Classes/Custom/UObjectGlobals.h"
#include "Utility/Logging.h"
#include "Unreal/UClass.hpp"
#include "Unreal/UObject.hpp"
#include "Helpers/String.hpp"

using namespace RC;
using namespace RC::Unreal;

UECustom::UDataTable* UECustom::UDataTableStore::GetTableByName(const std::string& Name)
{
    auto Table = TableMap.find(Name);
    if (Table != TableMap.end())
    {
        return Table->second;
    }

    return nullptr;
}

void UECustom::UDataTableStore::Store(const std::string& Name, UECustom::UDataTable* Table)
{
    TableMap.insert_or_assign(Name, Table);
}

void UECustom::UDataTableStore::Store(UECustom::UDataTable* Table)
{
    auto Name = Table->GetNamePrivate().ToString();
    UECustom::UDataTableStore::Store(RC::to_string(Name), Table);
}

void UECustom::UDataTableStore::Initialize()
{
    TArray<UObject*> Results;
    UECustom::UObjectGlobals::GetObjectsOfClass(UECustom::UDataTable::StaticClass(), Results);

    for (auto& Object : Results)
    {
        auto DT = static_cast<UECustom::UDataTable*>(Object);
        auto Name = Object->GetNamePrivate().ToString();
        UECustom::UDataTableStore::Store(RC::to_string(Name), DT);
    }

    PS::Log<LogLevel::Normal>(STR("Finished initializing UDataTableStore.\n"));
}
