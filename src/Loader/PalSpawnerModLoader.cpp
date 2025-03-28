#include "Unreal/AActor.hpp"
#include "Unreal/UClass.hpp"
#include "Unreal/UObjectGlobals.hpp"
#include "Unreal/UScriptStruct.hpp"
#include "Unreal/FProperty.hpp"
#include "Unreal/NameTypes.hpp"
#include "SDK/Classes/KismetInternationalizationLibrary.h"
#include "SDK/Structs/FPalSpawnerGroupInfo.h"
#include "SDK/EnumCache.h"
#include "Helpers/String.hpp"
#include "Utility/Logging.h"
#include "Loader/PalSpawnerModLoader.h"

using namespace RC;
using namespace RC::Unreal;

namespace Palworld {
	PalSpawnerModLoader::PalSpawnerModLoader() : PalModLoaderBase("spawners") {}

	PalSpawnerModLoader::~PalSpawnerModLoader() {}

	void PalSpawnerModLoader::Initialize() {}

	void PalSpawnerModLoader::Load(const nlohmann::json& Data)
	{
		for (auto& [SpawnerName, SpawnGroupList] : Data.items())
		{
			auto RowId = FName(RC::to_generic_string(SpawnerName), FNAME_Add);
			m_spawners.emplace(RowId, std::vector<SpawnerDataRow>{});
			for (auto& SpawnGroup : SpawnGroupList)
			{
				auto SpawnerIterator = m_spawners.find(RowId);
				if (SpawnerIterator == m_spawners.end())
				{
					PS::Log<RC::LogLevel::Error>(STR("Failed to add Spawner {} because it was invalid\n"), RowId.ToString());
					continue;
				}

				SpawnerDataRow NewData;

				if (SpawnGroup.contains("Weight"))
				{
					if (!SpawnGroup.at("Weight").is_number())
					{
						throw std::runtime_error(std::format("Weight for {} must be an integer", SpawnerName));
					}

					NewData.Weight = SpawnGroup.at("Weight").get<int>();
				}

				if (SpawnGroup.contains("OnlyTime"))
				{
					if (!SpawnGroup.at("OnlyTime").is_string())
					{
						throw std::runtime_error(std::format("OnlyTime for {} must be a string", SpawnerName));
					}

					NewData.OnlyTime = SpawnGroup.at("OnlyTime").get<std::string>();
				}

				if (SpawnGroup.contains("OnlyWeather"))
				{
					if (!SpawnGroup.at("OnlyWeather").is_string())
					{
						throw std::runtime_error(std::format("OnlyWeather for {} must be a string", SpawnerName));
					}

					NewData.OnlyWeather = SpawnGroup.at("OnlyWeather").get<std::string>();
				}

				if (SpawnGroup.contains("PalList"))
				{
					if (!SpawnGroup.at("PalList").is_array())
					{
						throw std::runtime_error(std::format("PalList for {} must be an array", SpawnerName));
					}

					auto PalList = SpawnGroup.at("PalList").get<std::vector<nlohmann::json>>();
					for (auto& Row : PalList)
					{
						SpawnerPalData PalData;

						if (Row.contains("PalID"))
						{
							if (!Row.at("PalID").is_string())
							{
								throw std::runtime_error(std::format("PalID in PalList of {} must be a string", SpawnerName));
							}
							PalData.PalID = FName(RC::to_generic_string(Row.at("PalID").get<std::string>()), FNAME_Add);
						}

						if (Row.contains("Level"))
						{
							if (!Row.at("Level").is_number())
							{
								throw std::runtime_error(std::format("Level in PalList of {} must be an integer", SpawnerName));
							}
							PalData.Level = Row.at("Level").get<int>();
						}

						if (Row.contains("Level_Max"))
						{
							if (!Row.at("Level_Max").is_number())
							{
								throw std::runtime_error(std::format("Level_Max in PalList of {} must be an integer", SpawnerName));
							}
							PalData.Level_Max = Row.at("Level_Max").get<int>();
						}

						if (Row.contains("Num"))
						{
							if (!Row.at("Num").is_number())
							{
								throw std::runtime_error(std::format("Num in PalList of {} must be an integer", SpawnerName));
							}
							PalData.Num = Row.at("Num").get<int>();
						}

						if (Row.contains("Num_Max"))
						{
							if (!Row.at("Num_Max").is_number())
							{
								throw std::runtime_error(std::format("Num_Max in PalList of {} must be an integer", SpawnerName));
							}
							PalData.Num_Max = Row.at("Num_Max").get<int>();
						}

						NewData.PalList.push_back(PalData);
						PS::Log<RC::LogLevel::Normal>(STR("Added a spawn for {} (Level {}-{}) to {}\n"), PalData.PalID.ToString(), PalData.Level, PalData.Level_Max,
							RC::to_generic_string(SpawnerName));
					}
				}
				SpawnerIterator->second.push_back(NewData);
			}
		}
	}

	void PalSpawnerModLoader::ApplySpawnerMod(AActor* Spawner)
	{
		auto SpawnerName = Spawner->GetValuePtrByPropertyNameInChain<FName>(STR("SpawnerName"));
		if (SpawnerName)
		{
			auto Name = *SpawnerName;
			auto SpawnIterator = m_spawners.find(Name);
			if (SpawnIterator != m_spawners.end())
			{
				auto& SpawnGroupList = *Spawner->GetValuePtrByPropertyNameInChain<TArray<FPalSpawnerGroupInfo>>(STR("SpawnGroupList"));
				SpawnGroupList.Empty();

				for (auto& SpawnerData : SpawnIterator->second)
				{
					FPalSpawnerGroupInfo GroupInfo;
					uint8_t OnlyTime;
					uint8_t OnlyWeather;
					EnumCache::TryGetEnumValue(SpawnerData.OnlyTime, OnlyTime);
					EnumCache::TryGetEnumValue(SpawnerData.OnlyWeather, OnlyWeather);
					GroupInfo.Weight = SpawnerData.Weight;
					GroupInfo.OnlyTime = OnlyTime;
					GroupInfo.OnlyWeather = OnlyWeather;

					for (auto& Pal : SpawnerData.PalList)
					{
						FPalSpawnerOneTribeInfo SpawnGroup;
						SpawnGroup.Level = Pal.Level;
						SpawnGroup.Level_Max = Pal.Level_Max;
						SpawnGroup.Num = Pal.Num;
						SpawnGroup.Num_Max = Pal.Num_Max;
						SpawnGroup.PalID.Key = Pal.PalID;
						SpawnGroup.NPCID.Key = Pal.NPCID;
						GroupInfo.PalList.Add(SpawnGroup);
					}

					SpawnGroupList.Add(GroupInfo);
				}
			}
		}
	}
}