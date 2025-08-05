#pragma once

#include "Loader/PalModLoaderBase.h"
#include "SDK/Classes/PalStaticItemDataAsset.h"

namespace UECustom {
	class UDataTable;
}

namespace Palworld {
    class UPalStaticItemDataTable;

	class PalItemModLoader : public PalModLoaderBase {
	public:
		PalItemModLoader();

		~PalItemModLoader();

		void Initialize();

		virtual void Load(const nlohmann::json& Data) override final;
    public:
        static UPalStaticItemDataBase* AddDummyItem(UPalStaticItemDataTable* StaticItemDataTable, const RC::Unreal::FName& ItemId);
	private:
		void Add(const RC::Unreal::FName& ItemId, const nlohmann::json& Data);

		void Edit(const RC::Unreal::FName& ItemId, UPalStaticItemDataBase* Item, const nlohmann::json& Data);

		void AddRecipe(const RC::Unreal::FName& ItemId, const nlohmann::json& Recipe);

		void EditRecipe(const RC::Unreal::FName& ItemId, const nlohmann::json& Recipe);

		void AddTranslations(const RC::Unreal::FName& ItemId, const nlohmann::json& Data);

		void EditTranslations(const RC::Unreal::FName& ItemId, const nlohmann::json& Data);

        void InitializeDummyTranslations();

		UPalStaticItemDataAsset* m_itemDataAsset{};
		UECustom::UDataTable* m_itemRecipeTable{};
		UECustom::UDataTable* m_nameTranslationTable{};
		UECustom::UDataTable* m_descriptionTranslationTable{};
	};
}