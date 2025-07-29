#pragma once

#include <string>
#include <unordered_set>
#include "nlohmann/json.hpp"

namespace PS {
    class PSConfig {
    public:
        static void Load();

        static std::string GetLanguageOverride();

        static bool IsAutoReloadEnabled();
    private:
        static inline std::unique_ptr<PSConfig> s_config;

        static inline std::unordered_set<std::string> s_deprecatedValues = {
            "enableExperimentalBlueprintSupport"
        };
    private:
        std::string m_languageOverride = "";
        bool m_enableAutoReload = false;

        static bool TryRemoveDeprecatedValues(nlohmann::ordered_json& Data);

        static bool GetString(const nlohmann::ordered_json& Data, const std::string& Key, const std::string& DefaultValue, std::string& OutValue);

        static bool GetBool(const nlohmann::ordered_json& Data, const std::string& Key, const bool& DefaultValue, bool& OutValue);
    };
}