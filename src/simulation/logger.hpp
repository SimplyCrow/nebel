#pragma once
#ifndef NEBEL_SIMULATION_LOGGER_HPP
#define NEBEL_SIMULATION_LOGGER_HPP

#include "../logger.hpp"

namespace simulation {

        inline std::shared_ptr<spdlog::logger> get_logger()
        {
                return logger::get_or_create_logger("Simulation");
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

}; // namespace simulation

#endif // NEBEL_SIMULATION_LOGGER_HPP
