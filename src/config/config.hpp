#pragma once
#ifndef NEBEL_CONFIG_CONFIG_HPP
#define NEBEL_CONFIG_CONFIG_HPP

#include <filesystem>
#include <optional>
#include <libconfig.h++>

#include "../panic.hpp"
#include "logger.hpp"

namespace config {

        class Config {
                public:
                        Config(std::filesystem::path root_config_file);

                        template <typename T>
                                T get_config() const = delete;

                        template <typename T>
                                void lookup(const std::string &path, T& t) const
                                {
                                        if(!m_config.lookupValue(path, t)) {
                                                PANIC(config, "config value \"{}\" not found", path);
                                        }
                                }

                        template <typename T>
                                void try_lookup(const std::string &path, T& t) const
                                {
                                        m_config.lookupValue(path, t);
                                }

                        bool exists(const std::string &path) const
                        {
                                return m_config.exists(path);
                        }

                private:
                        libconfig::Config m_config;
        };

}; // namespace config

#endif // NEBEL_CONFIG_CONFIG_HPP
