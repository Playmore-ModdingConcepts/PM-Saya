#include "Unreal/UClass.hpp"
#include "Unreal/UObject.hpp"
#include "Unreal/UObjectGlobals.hpp"
#include "Unreal/UScriptStruct.hpp"
#include "Unreal/FProperty.hpp"
#include "SDK/Classes/UDataAsset.h"
#include "SDK/Classes/UDataTable.h"
#include "SDK/Classes/KismetInternationalizationLibrary.h"
#include "SDK/Structs/FPalCharacterIconDataRow.h"
#include "SDK/Structs/FPalBPClassDataRow.h"
#include "SDK/EnumCache.h"
#include "Utility/DataTableHelper.h"
#include "Utility/Logging.h"
#include "Helpers/String.hpp"
#include "Loader/PalSkinModLoader.h"

using namespace RC;
using namespace RC::Unreal;

UClass* PalSkinDataPalCharacterClass = nullptr;

namespace Palworld {
	PalSkinModLoader::PalSkinModLoader() : PalModLoaderBase("skins") {}

	PalSkinModLoader::~PalSkinModLoader() {}

	void PalSkinModLoader::Initialize()
	{
		m_skinDataAsset = UObjectGlobals::StaticFindObject<UECustom::UDataAsset*>(nullptr, nullptr,
			STR("/Game/Pal/DataAsset/Skin/DA_StaticSkinDataAsset.DA_StaticSkinDataAsset"));

		m_skinIconTable = UObjectGlobals::StaticFindObject<UECustom::UDataTable*>(nullptr, nullptr,
			STR("/Game/Pal/DataTable/Character/DT_PalCharacterIconDataTable_SkinOverride.DT_PalCharacterIconDataTable_SkinOverride"));

		m_skinTranslationTable = UObjectGlobals::StaticFindObject<UECustom::UDataTable*>(nullptr, nullptr,
			STR("/Game/Pal/DataTable/Text/DT_UI_Common_Text.DT_UI_Common_Text"));
	}

	void PalSkinModLoader::Load(const nlohmann::json& json)
	{
		auto StaticSkinMap = m_skinDataAsset->GetValuePtrByPropertyName<TMap<FName, UObject*>>(STR("StaticSkinMap"));
		if (!StaticSkinMap)
		{
			throw std::runtime_error("Property 'StaticSkinMap' has changed name in DA_StaticSkinDataAsset, update is required");
		}

		for (auto& [skin_id, value] : json.items())
		{
			auto SkinId = FName(RC::to_generic_string(skin_id), FNAME_Add);
			if (StaticSkinMap->Contains(SkinId))
			{
				auto Row = StaticSkinMap->FindRef(SkinId);
				Edit(SkinId, Row, value);
			}
			else
			{
				Add(SkinId, StaticSkinMap, value);
			}
		}
	}

	void PalSkinModLoader::Add(const FName& SkinId, TMap<FName, UObject*>* StaticSkinMap, const nlohmann::json& Data)
	{
		if (!Data.contains("SkinType"))
		{
			throw std::runtime_error("You must supply SkinType when adding new skins.");
		}

		if (!Data.contains("NormalCharacterClass"))
		{
			throw std::runtime_error("You must supply NormalCharacterClass when adding new skins.");
		}

		if (!Data.contains("BossCharacterClass"))
		{
			throw std::runtime_error("You must supply BossCharacterClass when adding new skins.");
		}

		if (!Data.contains("TargetPalName"))
		{
			throw std::runtime_error("You must supply TargetPalName when adding new skins.");
		}

		auto TargetPalName = FName(RC::to_generic_string(Data.at("TargetPalName").get<std::string>()), FNAME_Add);

		UClass* DatabaseClass = nullptr;

		auto Type = Data.at("SkinType").get<std::string>();
		if (Type == "EPalSkinType::Pal" || Type == "Pal")
		{
			DatabaseClass = LoadOrCacheSkinDataBaseClass(STR("/Script/Pal.PalSkinDataPalCharacterClass"));
		}
		else if (Type == "EPalSkinType::Head" || Type == "Head" || Type == "EPalSkinType::Body" || Type == "Body")
		{
			DatabaseClass = LoadOrCacheSkinDataBaseClass(STR("/Script/Pal.PalSkinDataArmor"));
		}
		else
		{
			throw std::runtime_error(std::format("Unsupported skin type '{}'", Type));
		}

		if (!DatabaseClass->GetPropertyByNameInChain(STR("SkinName")))
		{
			throw std::runtime_error("Property 'SkinName' has changed in DA_StaticSkinDataAsset. Update to Pal Schema is needed.");
		}

		FStaticConstructObjectParameters ConstructParams(DatabaseClass, m_skinDataAsset);
		ConstructParams.Name = NAME_None;

		auto Item = UObjectGlobals::StaticConstructObject(ConstructParams);

		auto SkinNameProperty = Item->GetValuePtrByPropertyNameInChain<FName>(STR("SkinName"));
		if (SkinNameProperty)
		{
			*SkinNameProperty = SkinId;
		}

		for (auto& Property : DatabaseClass->ForEachPropertyInChain())
		{
			auto PropertyName = RC::to_string(Property->GetName());
			if (Data.contains(PropertyName))
			{
				DataTableHelper::CopyJsonValueToTableRow(reinterpret_cast<uint8_t*>(Item), Property, Data.at(PropertyName));
			}
		}

		if (Data.contains("IconTexture"))
		{
			auto IconRowStruct = m_skinIconTable->GetRowStruct().UnderlyingObjectPointer;
			auto IconProperty = IconRowStruct->GetPropertyByName(STR("Icon"));
			if (!IconProperty)
			{
				throw std::runtime_error("Property 'Icon' has changed in DT_PalCharacterIconDataTable_SkinOverride. Update to Pal Schema is needed.");
			}

			auto IconRowData = FMemory::Malloc(IconRowStruct->GetStructureSize());
			IconRowStruct->InitializeStruct(IconRowData);

			try
			{
				DataTableHelper::CopyJsonValueToTableRow(IconRowData, IconProperty, Data.at("IconTexture"));
			}
			catch (const std::exception& e)
			{
				FMemory::Free(IconRowData);
				throw std::runtime_error(e.what());
			}

			m_skinIconTable->AddRow(SkinId, *reinterpret_cast<UECustom::FTableRowBase*>(IconRowData));
		}

		if (Data.contains("Name"))
		{
			AddTranslation(SkinId, Data.at("Name"));
		}

		StaticSkinMap->Add(SkinId, Item);

		PS::Log<RC::LogLevel::Normal>(STR("Added new Skin '{}'\n"), SkinId.ToString());
	}

	void PalSkinModLoader::Edit(const FName& SkinId, UObject* Item, const nlohmann::json& Data)
	{
		for (auto& Property : Item->GetClassPrivate()->ForEachPropertyInChain())
		{
			auto PropertyName = RC::to_string(Property->GetName());
			if (Data.contains(PropertyName))
			{
				DataTableHelper::CopyJsonValueToTableRow(reinterpret_cast<uint8_t*>(Item), Property, Data.at(PropertyName));
			}
		}

		auto SkinNameProperty = Item->GetValuePtrByPropertyNameInChain<FName>(STR("SkinName"));
		if (SkinNameProperty)
		{
			if (Data.contains("Icon"))
			{
				auto IconRowStruct = m_skinIconTable->GetRowStruct().UnderlyingObjectPointer;
				auto IconProperty = IconRowStruct->GetPropertyByName(STR("Icon"));
				if (IconProperty)
				{
					auto Row = m_skinIconTable->FindRowUnchecked(*SkinNameProperty);
					if (Row)
					{
						DataTableHelper::CopyJsonValueToTableRow(Row, IconProperty, Data.at("Icon"));
					}
				}
			}
		}

		if (Data.contains("Name"))
		{
			EditTranslation(SkinId, Data.at("Name"));
		}
	}

	void PalSkinModLoader::AddTranslation(const RC::Unreal::FName& SkinId, const nlohmann::json& Data)
	{
		auto FixedSkinId = std::format(STR("SKIN_NAME_{}"), SkinId.ToString());
		auto TranslationRowStruct = m_skinTranslationTable->GetRowStruct().UnderlyingObjectPointer;
		auto TextProperty = TranslationRowStruct->GetPropertyByName(STR("TextData"));
		if (TextProperty)
		{
			auto TranslationRowData = FMemory::Malloc(TranslationRowStruct->GetStructureSize());
			TranslationRowStruct->InitializeStruct(TranslationRowData);

			try
			{
				DataTableHelper::CopyJsonValueToTableRow(TranslationRowData, TextProperty, Data);
			}
			catch (const std::exception& e)
			{
				FMemory::Free(TranslationRowData);
				throw std::runtime_error(e.what());
			}

			m_skinTranslationTable->AddRow(FName(FixedSkinId, FNAME_Add), *reinterpret_cast<UECustom::FTableRowBase*>(TranslationRowData));
		}
	}

	void PalSkinModLoader::EditTranslation(const RC::Unreal::FName& SkinId, const nlohmann::json& Data)
	{
		auto FixedSkinId = std::format(STR("SKIN_NAME_{}"), SkinId.ToString());
		auto TranslationRowStruct = m_skinTranslationTable->GetRowStruct().UnderlyingObjectPointer;
		auto TextProperty = TranslationRowStruct->GetPropertyByName(STR("TextData"));
		if (TextProperty)
		{
			auto Row = m_skinTranslationTable->FindRowUnchecked(FName(FixedSkinId, FNAME_Add));
			if (Row)
			{
				DataTableHelper::CopyJsonValueToTableRow(Row, TextProperty, Data);
			}
		}
	}

	RC::Unreal::UClass* PalSkinModLoader::LoadOrCacheSkinDataBaseClass(const RC::StringType& Path)
	{
		auto Iterator = m_palSkinDataBaseClassCache.find(Path);
		if (Iterator != m_palSkinDataBaseClassCache.end())
		{
			return Iterator->second;
		}

		auto NewSkinDataBaseClass = UObjectGlobals::StaticFindObject<UClass*>(nullptr, nullptr, Path);
		if (!NewSkinDataBaseClass)
		{
			throw std::runtime_error(std::format("Failed to cache PalSkinDataBaseClass, '{}' doesn't exist", RC::to_string(Path)));
		}

		return NewSkinDataBaseClass;
	}
}