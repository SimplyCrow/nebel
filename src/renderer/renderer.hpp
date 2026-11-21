#pragma once
#ifndef NEBEL_RENDERER_RENDERER_HPP
#define NEBEL_RENDERER_RENDERER_HPP

#include <SDL3/SDL.h>
#include <glad/gl.h>
#include <glm/glm.hpp>

#include <cstdint>
#include <map>

#include "shader.hpp"
#include "settings.hpp"

#include "../simulation/world_buffer.hpp"

namespace renderer {

        struct RenderTarget {
                GLuint fbo = 0;
                GLuint color_texture = 0;
                GLuint depth_rbo = 0;
                int width = 0;
                int height = 0;
        };

        struct Char {
                unsigned   texture;
                glm::ivec2 size;
                glm::ivec2 bearing;
                unsigned   advance;
        };

        class Renderer {
                public:
                        Renderer(Settings settings);
                        ~Renderer();

                        void iteration(simulation::WorldBuffer::SharedPtr world_buffer);

                        bool should_close()
                        {
                                return !m_window.running;
                        }

                private:
                        struct window_data {
                                bool running;
                                int width;
                                int height;
                                SDL_Window *window;
                                RenderTarget target;
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
                                float    current_fps;
                        } m_frame;

                        struct gl_data {
                                SDL_GLContext context;
                                std::unordered_map<std::string, Shader> shaders;
                                GLuint instanced_square_vao;
                                GLuint unit_square_vao;
                        } m_gl;

                        struct camera_data {
                                glm::vec2 position{0, 0};
                                float zoom = 15.0f;
                                float minimum_zoom = 0.2f;
                                float speed = 6.0f;
                        } m_camera;

                        std::map<char, Char> m_chars;
                        GLuint m_chars_vbo;
                        GLuint m_chars_vao;

                        void setup_sdl();
                        void setup_opengl();
                        void setup_imgui();
                        void setup_render_data();
                        void load_shader(std::string name, std::string vertex_path, std::string fragment_path);
                        void handle_events();
                        void handle_resize();
                        void calculate_camera();
                        void render_text(Shader &s, std::string text, float x, float y, float scale, glm::vec3 color);
                        void resize_render_target(RenderTarget &target, int width, int height);

                        void set_shader_mvp(Shader &shader, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection);
                        void set_shader_anchor(Shader &shader, glm::vec2 anchor);
                        void render_grid(glm::mat4 &view, glm::mat4 &projection, int width, int height);

                        void frame_start();
                        void frame_end();
                        void calculate_matrices(glm::mat4 &view, glm::mat4 &projection, glm::mat4 &ui_projection);
                        void clear_screen();
        };

}; // namespace renderer

#endif // NEBEL_RENDERER_RENDERER_HPP
