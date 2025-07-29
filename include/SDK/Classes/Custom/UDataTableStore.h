#pragma once

#include <unordered_map>
#include <string>
#include "SDK/Classes/UDataTable.h"

namespace UECustom {
    class UDataTableStore {
    private:
        static inline std::unordered_map<std::string, UECustom::UDataTable*> TableMap;
    public:
        static UECustom::UDataTable* GetTableByName(const std::string& Name);

        static void Store(const std::string& Name, UECustom::UDataTable* Table);

        static void Store(UECustom::UDataTable* Table);

        static void Initialize();
    };
}