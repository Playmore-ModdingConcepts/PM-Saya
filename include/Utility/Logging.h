#pragma once

#include <HAL/Platform.hpp>
#include <DynamicOutput/DynamicOutput.hpp>
#include "Utility/Config.h"

namespace PS {
    template <RC::Unreal::int32 optional_arg, typename... FmtArgs>
    auto Log(RC::File::StringViewType content, FmtArgs... fmt_args) -> void
    {
        if (optional_arg == RC::LogLevel::Error)
        {
            auto formatted_log = std::format(STR("[SAYA] [error] {}"), content);
            RC::Output::send<optional_arg>(formatted_log, fmt_args...);
        }
        else if (optional_arg == RC::LogLevel::Warning)
        {
            auto formatted_log = std::format(STR("[SAYA] [warning] {}"), content);
            RC::Output::send<optional_arg>(formatted_log, fmt_args...);
        }
        else if (optional_arg == RC::LogLevel::Verbose)
        {
            if (!PS::PSConfig::IsDebugLoggingEnabled()) return;

            auto formatted_log = std::format(STR("[SAYA] [debug] {}"), content);
            RC::Output::send<optional_arg>(formatted_log, fmt_args...);
        }
        else
        {
            auto formatted_log = std::format(STR("[SAYA] {}"), content);
            RC::Output::send<optional_arg>(formatted_log, fmt_args...);
        }
    }
}