#pragma once

#include "Loader/PalModLoaderBase.h"
#include "Unreal/NameTypes.hpp"

namespace RC::Unreal {
	class UObject;
}

namespace UECustom {
	class UDataAsset;
	class UDataTable;
}

namespace Palworld {
	class PalBuildingModLoader : public PalModLoaderBase {
	public:
		PalBuildingModLoader();

		~PalBuildingModLoader();

		void Initialize();

		virtual void Load(const nlohmann::json& json) override final;
	private:
		UECustom::UDataTable* m_mapObjectAssignData{};
		UECustom::UDataTable* m_mapObjectFarmCrop{};
		UECustom::UDataTable* m_mapObjectItemProductDataTable{};
		UECustom::UDataTable* m_mapObjectMasterDataTable{};
		UECustom::UDataTable* m_mapObjectNameTable{};
		UECustom::UDataTable* m_buildObjectDataTable{};
		UECustom::UDataTable* m_buildObjectIconDataTable{};
        UECustom::UDataTable* m_buildObjectDescTable{};
		UECustom::UDataTable* m_technologyRecipeUnlockTable{};
		UECustom::UDataTable* m_technologyNameTable{};
		UECustom::UDataTable* m_technologyDescTable{};

		void Add(const RC::Unreal::FName& BuildingId, const nlohmann::json& Data);

		void Edit(uint8_t* ExistingRow, const RC::Unreal::FName& BuildingId, const nlohmann::json& Data);

		void SetupBuildData(const RC::Unreal::FName& BuildingId, const nlohmann::json& Data);

		void SetupIconData(const RC::Unreal::FName& BuildingId, const nlohmann::json& Data);

		void SetupAssignments(const RC::Unreal::FName& BuildingId, const nlohmann::json& Data);

        void SetupAssignment(const RC::Unreal::FName& BuildingId, const nlohmann::json& Data);

		void SetupCropData(const RC::Unreal::FName& BuildingId, const nlohmann::json& Data);

		void SetupItemProductData(const RC::Unreal::FName& BuildingId, const nlohmann::json& Data);

		void SetupTechnologyData(const RC::Unreal::FName& BuildingId, const nlohmann::json& Data);

        void SetupTranslations(const RC::Unreal::FName& BuildingId, const nlohmann::json& Data);

        void SetupTranslation(const RC::StringType& RowKey, UECustom::UDataTable* DataTable, const nlohmann::json& Value);

        void ImportJson(const RC::Unreal::FName& BuildingId, const nlohmann::json& Data, UECustom::UDataTable* DataTable);

        RC::StringType GetAssignIDSuffixByWorkType(const std::string& WorkType);
    private:
        static inline int RadialIndex = 1000;

        static int GetNextRadialIndex();
	};
}