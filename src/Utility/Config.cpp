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
        if (m_config) return;

        try
        {
            m_config = std::make_unique<PSConfig>();

            auto cwd = fs::path(UE4SSProgram::get_program().get_working_directory()) / "Mods" / "PalSchema" / "config";

            if (!fs::exists(cwd))
            {
                fs::create_directories(cwd);
            }

            std::ifstream f(cwd / "config.json");
            bool ShouldResave = false;

            nlohmann::ordered_json data = {};
            if (f.fail()) {
                data["languageOverride"] = "";
                std::ofstream out_file(cwd / "config.json");
                out_file << data.dump(4);
                out_file.close();
            }
            else
            {
                data = nlohmann::ordered_json::parse(f);
            }

            if (!data.contains("languageOverride"))
            {
                data["languageOverride"] = "";
                ShouldResave = true;
            }
            else
            {
                if (!data.at("languageOverride").is_string())
                {
                    PS::Log<RC::LogLevel::Error>(STR("languageOverride in config.json wasn't a string, resetting to default.\n"));
                    data["languageOverride"] = "";
                    ShouldResave = true;
                }
                m_config->m_languageOverride = data["languageOverride"].get<std::string>();
            }

            if (ShouldResave)
            {
                std::ofstream out_file(cwd / "config.json");
                out_file << data.dump(4);
                out_file.close();
            }

            PS::Log<RC::LogLevel::Normal>(STR("PalSchema config loaded.\n"));
        }
        catch (const std::exception& e)
        {
            PS::Log<RC::LogLevel::Error>(STR("Failed to load PalSchema Config: {}\n"), RC::to_generic_string(e.what()));
        }
    }

    std::string PSConfig::GetLanguageOverride()
    {
        if (m_config)
        {
            return m_config->m_languageOverride;
        }

        PS::Log<RC::LogLevel::Error>(STR("PalSchema Config must be initialized first before accessing GetLanguageOverride!"));

        return "";
    }
}