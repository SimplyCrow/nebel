#pragma once
#ifndef NEBEL_LOGGER_HPP
#define NEBEL_LOGGER_HPP

#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace logger {

        inline std::shared_ptr<spdlog::logger> get_or_create_logger(const std::string& name)
        {
                auto logger = spdlog::get(name);
                if(!logger) {
                        logger = spdlog::stdout_color_mt(name);
                }
                return logger;
        }

        inline std::shared_ptr<spdlog::logger> get_logger()
        {
                return get_or_create_logger("Main");
        }

        template<typename... Args>
        inline void info(fmt::format_string<Args...> fmt, Args&&... args)
        {
                get_logger()->info(fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        inline void warn(fmt::format_string<Args...> fmt, Args&&... args)
        {
                get_logger()->warn(fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        inline void error(fmt::format_string<Args...> fmt, Args&&... args)
        {
                get_logger()->error(fmt, std::forward<Args>(args)...);
        }

        template<typename... Args>
        inline void critical(fmt::format_string<Args...> fmt, Args&&... args)
        {
                get_logger()->critical(fmt, std::forward<Args>(args)...);
        }


}; // namespace logger

#endif // NEBEL_LOGGER_HPP
