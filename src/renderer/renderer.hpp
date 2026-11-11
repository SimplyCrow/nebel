#pragma once
#ifndef NEBEL_RENDERER_RENDERER_HPP
#define NEBEL_RENDERER_RENDERER_HPP

#include <SDL3/SDL.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <cstdint>
#include "shader.hpp"
#include "../simulation.hpp"

class Renderer {
public:
        Renderer(int initial_width, int initial_height, int target_fps, sim::Simulation &simulation);
        ~Renderer();

        void iteration();

        bool should_close()
        {
                return !m_running;
        }

private:
        sim::Simulation &m_simulation;
        bool m_running = true;
        int m_width;
        int m_height;
        SDL_Window *m_window;
        SDL_GLContext *m_context;
        Shader m_circle_shader;
        GLuint m_vao = 0;
        GLuint m_instanced = 0;
        glm::vec2 m_camera_position;
        float m_zoom;
        glm::mat4 m_projection;
        glm::mat4 m_view;
        uint64_t m_last_time;
        float m_delta_time;
        float m_runtime;
        uint64_t m_frame;
        uint64_t m_target_fps;
        uint64_t m_ns_per_frame;

        void setup_sdl();
        void setup_opengl();
        void setup_render_data();
        void handle_events();
        void handle_resize();
        void calculate_camera();

        void clear_screen();
};

#endif // NEBEL_RENDERER_RENDERER_HPP
