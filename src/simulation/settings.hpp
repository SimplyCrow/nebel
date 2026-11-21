#pragma once
#ifndef NEBEL_SIMULATION_SETTINGS_HPP
#define NEBEL_SIMULATION_SETTINGS_HPP

#include <string>
#include <cstdint>

#include "logger.hpp"
#include "../config/config.hpp"

namespace simulation {

        struct Settings {
                std::string name     = "NO SIMULATION NAME PROVIDED";
                uint32_t    width    = 128;
                uint32_t    height   = 128;
                uint32_t    entities = 128;
                uint32_t    iterations_per_second = 10;
        };

}; // namespace simulation

template <>
inline simulation::Settings config::Config::get_config<simulation::Settings>() const
{
        if(!exists("simulation")) {
                PANIC(simulation, "Simulation configuration not found but expected");
        }

        simulation::Settings settings;
        try_lookup("simulation.name",     settings.name);
        try_lookup("simulation.width",    settings.width);
        try_lookup("simulation.height",   settings.height);
        try_lookup("simulation.entities", settings.entities);
        try_lookup("simulation.iterations_per_second", settings.iterations_per_second);

        simulation::info("Configuration loaded:");
        simulation::info("- name: {}",     settings.name);
        simulation::info("- width: {}",    settings.width);
        simulation::info("- height: {}",   settings.height);
        simulation::info("- entities: {}", settings.entities);
        simulation::info("- iterations_per_second: {}", settings.iterations_per_second);

        return settings;
}

#endif // NEBEL_SIMULATION_SETTINGS_HPP
