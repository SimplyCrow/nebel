#pragma once
#ifndef NEBEL_RENDERER_RENDERER_HPP
#define NEBEL_RENDERER_RENDERER_HPP

#include <SDL3/SDL.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <cstdint>
#include <map>
#include "shader.hpp"
#include "../simulation.hpp"

class Renderer {
public:
        Renderer(int initial_width, int initial_height, uint64_t target_fps, sim::Simulation &simulation);
        ~Renderer();

        void iteration();

        bool should_close()
        {
                return !m_window.running;
        }

private:
        struct window_data {
                bool running;
                int  width;
                int  height;
                SDL_Window *window;
        } m_window;

        struct frame_data {
                uint64_t current_frame;
                uint64_t target_fps;
                uint64_t ns_per_frame;

                uint64_t start_frame_time;
                uint64_t last_frame_time;
                float    delta_time;
                float    total_runtime;

                float    fps_timer;
                uint64_t fps_count;
        } m_frame;

        struct gl_data {
                SDL_GLContext context;
                std::unordered_map<std::string, Shader> shaders;
                GLuint instanced_square_vao;
                GLuint unit_square_vao;
        } m_gl;

        struct camera_data {
                glm::vec2 position;
                float zoom;
        } m_camera;

        sim::Simulation &m_simulation;

        void setup_sdl();
        void setup_opengl();
        void setup_render_data();
        void load_shader(std::string name, std::string vertex_path, std::string fragment_path);
        void handle_events();
        void handle_resize();
        void calculate_camera();

        void frame_start();
        void frame_end();
        void calculate_matrices(glm::mat4 &view, glm::mat4 &projection);
        void clear_screen();
};

#endif // NEBEL_RENDERER_RENDERER_HPP
