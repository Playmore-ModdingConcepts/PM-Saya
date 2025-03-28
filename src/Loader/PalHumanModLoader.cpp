#include "Unreal/UObjectGlobals.hpp"
#include "Loader/PalHumanModLoader.h"

using namespace RC;
using namespace RC::Unreal;

namespace Palworld {
	PalHumanModLoader::PalHumanModLoader() : PalModLoaderBase("npcs") {}

	PalHumanModLoader::~PalHumanModLoader() {}

	void PalHumanModLoader::Load(const nlohmann::json& json)
	{

	}
}