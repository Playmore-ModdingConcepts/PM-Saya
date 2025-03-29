#pragma once

#include "Unreal/NameTypes.hpp"
#include "Loader/PalModLoaderBase.h"
#include "nlohmann/json.hpp"

namespace UECustom {
	class UDataTable;
}

namespace Palworld {
	class PalRawTableLoader : public PalModLoaderBase {
        struct LoadResult {
            int SuccessfulModifications = 0;
            int SuccessfulAdditions = 0;
            int SuccessfulDeletions = 0;
            int ErrorCount = 0;
        };
	public:
		PalRawTableLoader();

		~PalRawTableLoader();

		void Initialize();

        void Apply(UECustom::UDataTable* Table);

        void Apply(const nlohmann::json& Data, UECustom::UDataTable* Table, LoadResult& OutResult);

        void ApplyLate();

		virtual void Load(const nlohmann::json& Data) override final;

        void Reload(const nlohmann::json& Data);

        void SetIsUnrealReady(bool IsReady);
    private:
        std::unordered_map<RC::StringType, std::vector<nlohmann::json>> m_tableDataMap;
        std::unordered_map<RC::StringType, UECustom::UDataTable*> m_tableMap;

        bool m_isUnrealReady = false;

        void AddData(const RC::StringType& TableName, const nlohmann::json& Data);
	};
}