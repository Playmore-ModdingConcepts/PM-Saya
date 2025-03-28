#pragma once

#include "Unreal/NameTypes.hpp"
#include "nlohmann/json.hpp"
#include <string>

namespace UECustom {
	class UDataTable;
}

namespace Palworld {
	class PalModLoaderBase {
	public:
		PalModLoaderBase(const std::string& modFolderName);

		virtual ~PalModLoaderBase();

        virtual void Apply(UECustom::UDataTable* Table);

		virtual void Load(const nlohmann::json& Data);

		void Initialize();
    protected:
        std::string m_modFolderName = "";
	};
}