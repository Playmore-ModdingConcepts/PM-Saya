#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <DynamicOutput/DynamicOutput.hpp>
#include <Helpers/String.hpp>
#include "Utility/Config.h"
#include "Utility/Logging.h"
#include "UE4SSProgram.hpp"

namespace fs = std::filesystem;

using namespace RC;

namespace PS {
    void PSConfig::Load()
    {
        if (s_config) return;

        try
        {
            s_config = std::make_unique<PSConfig>();

            auto cwd = fs::path(UE4SSProgram::get_program().get_working_directory()) / "Mods" / "PM-Saya" / "SAYA" / "config";

            if (!fs::exists(cwd))
            {
                fs::create_directories(cwd);
            }

            std::ifstream f(cwd / "config.json");
            bool ShouldResave = false;

            nlohmann::ordered_json data = {};
            if (f.fail()) {
                // data["languageOverride"] = "";
                data["enableAutoReload"] = false;
                data["enableDebugLogging"] = false;
                std::ofstream out_file(cwd / "config.json");
                out_file << data.dump(4);
                out_file.close();
            }
            else
            {
                data = nlohmann::ordered_json::parse(f);
            }

            // if (!GetString(data, "languageOverride", "", s_config->m_languageOverride))
            // {
                // data["languageOverride"] = "";
                // ShouldResave = true;
            // }

            if (!GetBool(data, "enableAutoReload", false, s_config->m_enableAutoReload))
            {
                data["enableAutoReload"] = false;
                ShouldResave = true;
            }

            if (!GetBool(data, "enableDebugLogging", false, s_config->m_enableDebugLogging))
            {
                data["enableDebugLogging"] = false;
                ShouldResave = true;
            }

            if (TryRemoveDeprecatedValues(data))
            {
                ShouldResave = true;
            }

            if (ShouldResave)
            {
                std::ofstream out_file(cwd / "config.json");
                out_file << data.dump(4);
                out_file.close();
            }

            PS::Log<RC::LogLevel::Normal>(STR("Config loaded.\n"));
        }
        catch (const std::exception& e)
        {
            PS::Log<RC::LogLevel::Error>(STR("Failed to load Config - {}\n"), RC::to_generic_string(e.what()));
        }
    }

    std::string PSConfig::GetLanguageOverride()
    {
        if (s_config)
        {
            return s_config->m_languageOverride;
        }

        PS::Log<RC::LogLevel::Error>(STR("Config must be initialized first before accessing GetLanguageOverride!"));

        return "";
    }

    bool PSConfig::IsAutoReloadEnabled()
    {
        if (s_config)
        {
            return s_config->m_enableAutoReload;
        }

        PS::Log<RC::LogLevel::Error>(STR("Config must be initialized first before accessing IsAutoReloadEnabled!"));

        return "";
    }

    bool PSConfig::IsDebugLoggingEnabled()
    {
        if (s_config)
        {
            return s_config->m_enableDebugLogging;
        }

        PS::Log<RC::LogLevel::Error>(STR("Config must be initialized first before accessing IsDebugLoggingEnabled!"));

        return false;
    }

    bool PSConfig::TryRemoveDeprecatedValues(nlohmann::ordered_json& Data)
    {
        bool wasDeprecationPerformed = false;
        for (auto& deprecatedValue : s_deprecatedValues)
        {
            if (Data.contains(deprecatedValue))
            {
                Data.erase(deprecatedValue);
                wasDeprecationPerformed = true;
            }
        }

        return wasDeprecationPerformed;
    }

    bool PSConfig::GetString(const nlohmann::ordered_json& Data, const std::string& Key, const std::string& DefaultValue, std::string& OutValue)
    {
        if (!Data.contains(Key))
        {
            PS::Log<RC::LogLevel::Warning>(STR("{} in config.json was missing, adding to config.\n"), RC::to_generic_string(Key));
            OutValue = DefaultValue;
            return false;
        }
        else
        {
            if (!Data.at(Key).is_string())
            {
                PS::Log<RC::LogLevel::Warning>(STR("{} in config.json wasn't a string, fixing.\n"), RC::to_generic_string(Key));
                OutValue = DefaultValue;
                return false;
            }
        }

        OutValue = Data[Key].get<std::string>();
        return true;
    }

    bool PSConfig::GetBool(const nlohmann::ordered_json& Data, const std::string& Key, const bool& DefaultValue, bool& OutValue)
    {
        if (!Data.contains(Key))
        {
            PS::Log<RC::LogLevel::Warning>(STR("{} in config.json was missing, adding to config.\n"), RC::to_generic_string(Key));
            OutValue = DefaultValue;
            return false;
        }
        else
        {
            if (!Data.at(Key).is_boolean())
            {
                PS::Log<RC::LogLevel::Warning>(STR("{} in config.json wasn't a boolean, fixing.\n"), RC::to_generic_string(Key));
                OutValue = DefaultValue;
                return false;
            }
        }

        OutValue = Data[Key].get<bool>();
        return true;
    }
}