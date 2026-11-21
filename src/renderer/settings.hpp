#pragma once
#ifndef NEBEL_RENDERER_SETTINGS_HPP
#define NEBEL_RENDERER_SETTINGS_HPP

#include <string>
#include <cstdint>

#include "logger.hpp"
#include "../config/config.hpp"

namespace renderer {

        struct Settings {
                std::string window_title  = "Nebel";
                uint32_t    window_width  = 1280;
                uint32_t    window_height = 960;
                uint32_t    target_fps    = 60;
        };

}; // namespace renderer

template <>
inline renderer::Settings config::Config::get_config<renderer::Settings>() const
{
        if(!exists("renderer")) {
                PANIC(renderer, "Render configuration not found but expected");
        }

        renderer::Settings settings;
        try_lookup("renderer.window_title", settings.window_title);
        try_lookup("renderer.window_width", settings.window_width);
        try_lookup("renderer.window_height", settings.window_height);
        try_lookup("renderer.target_fps",   settings.target_fps);

        renderer::info("Configuration loaded:");
        renderer::info("- window_title: {}", settings.window_title);
        renderer::info("- window_width: {}", settings.window_width);
        renderer::info("- window_height: {}", settings.window_height);
        renderer::info("- target_fps: {}",   settings.target_fps);

        return settings;
}

#endif // NEBEL_RENDERER_SETTINGS_HPP
