#pragma once

#include "Unreal/NameTypes.hpp"
#include "Loader/PalModLoaderBase.h"
#include "nlohmann/json.hpp"

namespace UECustom {
	class UDataTable;
}

namespace Palworld {
	class PalRawTableLoader : public PalModLoaderBase {
	public:
		PalRawTableLoader();

		~PalRawTableLoader();

		void Initialize();

        void Apply(UECustom::UDataTable* Table);

		virtual void Load(const nlohmann::json& json) override final;
    private:
        std::unordered_map<RC::StringType, std::vector<nlohmann::json>> m_tableDataMap;

        void AddData(const RC::StringType& TableName, const nlohmann::json& Data);
	};
}