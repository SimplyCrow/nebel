#include <spdlog/spdlog.h>
#include <SDL3/SDL.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer.hpp"
#include "../panic.hpp"

Renderer::Renderer(int initial_width, int initial_height, int target_fps, sim::Simulation &simulation)
        : m_simulation(simulation), m_width(initial_width), m_height(initial_height), m_ns_per_frame(1000000000ULL / target_fps)
{
        setup_sdl();
        setup_opengl();
        setup_render_data();
}

void Renderer::setup_sdl()
{
        int sdl_version = SDL_GetVersion();
        spdlog::info("(Renderer) Initializing SDL version: {}.{}.{}"
                        , SDL_VERSIONNUM_MAJOR(sdl_version)
                        , SDL_VERSIONNUM_MINOR(sdl_version)
                        , SDL_VERSIONNUM_MICRO(sdl_version));

        if(!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
                PANIC("(Renderer) Could not initialize SDL subsystems");
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        m_window = SDL_CreateWindow("Nebel", m_width, m_height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        if(m_window == nullptr) {
                PANIC("(Renderer) Could not create window");
        }

        spdlog::info("(Renderer) Window created w={} h={}", m_width, m_height);
}

void Renderer::setup_opengl()
{
        SDL_GLContext m_context = SDL_GL_CreateContext(m_window);
        if(!m_context) {
                PANIC("(Renderer) Could not create OpenGL context: {}", SDL_GetError());
        }

        SDL_GL_SetSwapInterval(1);

        int open_version = gladLoadGL(SDL_GL_GetProcAddress);
        if(open_version == 0) {
                PANIC("Could not load OpenGL functions");
        }

        spdlog::info("(Renderer) OpenGL version loaded: {}.{}", GLAD_VERSION_MAJOR(open_version), GLAD_VERSION_MINOR(open_version));

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        handle_resize();
}

void Renderer::setup_render_data()
{
        spdlog::info("(Renderer) Initializes render data");

        Shader shader("./res/shader/vert.vs", "./res/shader/frag.fs");
        m_circle_shader = shader;
        m_circle_shader.use();

        float vertices[] = {
                -1.0f, -1.0f,
                -1.0f,  1.0f,
                 1.0f, -1.0f,
                -1.0f,  1.0f,
                 1.0f,  1.0f,
                 1.0f, -1.0f,
        };

        GLuint VBO;
        glCreateBuffers(1, &VBO);
        glNamedBufferStorage(VBO, sizeof(vertices), vertices, GL_DYNAMIC_STORAGE_BIT);

        glCreateVertexArrays(1, &m_vao);
        glVertexArrayVertexBuffer(m_vao, 0, VBO, 0, 2 * sizeof(float));

        glEnableVertexArrayAttrib(m_vao, 0);
        glVertexArrayAttribFormat(m_vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_vao, 0, 0);

        glEnableVertexArrayAttrib(m_vao, 1);
        glVertexArrayAttribFormat(m_vao, 1, 2, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_vao, 1, 1);

        const sim::Map &map = m_simulation.get_map();

        m_zoom = 15.0f;
        m_camera_position = 0.5f * glm::vec2(map.width(), map.height());
        calculate_camera();

        m_last_time = SDL_GetTicksNS();
        m_frame = 0;
}

Renderer::~Renderer()
{
        SDL_Quit();
        spdlog::info("(Renderer) Exited");
}

void Renderer::handle_events()
{
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
                if(event.type == SDL_EVENT_QUIT) {
                        m_running = false;
                        continue;
                }

                if(event.type == SDL_EVENT_WINDOW_RESIZED) {
                        m_width = event.window.data1;
                        m_height = event.window.data2;
                        handle_resize();
                        spdlog::info("(Renderer) Resized w={} h={}", m_width, m_height);
                }
        }

        const bool *key_states = SDL_GetKeyboardState(NULL);
        float speed = 20.0f;
        float zoom_speed = 10.0f;
        glm::vec2 dir(0.0f);
        if(key_states[SDL_SCANCODE_W]) {
                dir.y += 1.0f;
        }
        if(key_states[SDL_SCANCODE_S]) {
                dir.y -= 1.0f;
        }
        if(key_states[SDL_SCANCODE_A]) {
                dir.x -= 1.0f;
        }
        if(key_states[SDL_SCANCODE_D]) {
                dir.x += 1.0f;
        }

        float zoom_direction = 0.0f;
        if(key_states[SDL_SCANCODE_Q]) {
                zoom_direction += 1.0f;
        }
        if(key_states[SDL_SCANCODE_E]) {
                zoom_direction -= 1.0f;
        }

        if(dir.x != 0.0f || dir.y != 0.0f) {
                glm::vec2 mov = dir * (m_delta_time * speed);
                m_camera_position += mov;
                calculate_camera();
        }

        if(zoom_direction) {
                m_zoom += zoom_direction * zoom_speed * m_delta_time;
                if(m_zoom < 0.0f)
                        m_zoom = 0.0f;
                handle_resize();
        }
}

void Renderer::handle_resize()
{
        glViewport(0, 0, m_width, m_height);
        float aspect_ratio = (float)m_width / (float)m_height;
        float world_width = 10.0f;
        float world_height = world_width / aspect_ratio;

        float left   = (-world_width / 2.0f) * m_zoom;
        float right  = ( world_width / 2.0f) * m_zoom;
        float bottom = (-world_height / 2.0f) * m_zoom;
        float top    = ( world_height / 2.0f) * m_zoom;
        m_projection = glm::ortho(left, right, bottom, top, -10.0f, 10.0f);
}

void Renderer::calculate_camera()
{
        m_view = glm::mat4(1.0f);
        m_view = glm::translate(m_view, glm::vec3(-m_camera_position, 0.0f));
}

void Renderer::iteration()
{
        uint64_t frame_start = SDL_GetTicksNS();
        m_delta_time = (float)(frame_start - m_last_time) / 1000000000.0f;
        m_runtime = (float)frame_start / 1000000000.0f;
        m_last_time = frame_start;

        handle_events();
        clear_screen();

        const sim::Map &map = m_simulation.get_map();

        std::vector<float> translations;
        translations.reserve(2 * map.width() * map.height());
        for(size_t y = 0; y < map.height(); ++y) {
                for(size_t x = 0; x < map.width(); ++x) {
                        if(!map.contains(x, y))
                                continue;
                        const sim::Node &node = m_simulation.get_node(map.get(x, y));
                        translations.emplace_back(node.position.x);
                        translations.emplace_back(node.position.y);
                }
        }

        glDeleteBuffers(1, &m_instanced);
        glCreateBuffers(1, &m_instanced);
        glNamedBufferStorage(m_instanced, translations.size() * sizeof(float), translations.data(), GL_DYNAMIC_STORAGE_BIT);
        glVertexArrayVertexBuffer(m_vao, 1, m_instanced, 0, 2 * sizeof(float));
        glVertexArrayBindingDivisor(m_vao, 1, 1);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0f, 3.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(1.0f));

        m_circle_shader.use();
        m_circle_shader.set_mat4("uModel", model);
        m_circle_shader.set_mat4("uView", m_view);
        m_circle_shader.set_mat4("uProjection", m_projection);
        glBindVertexArray(m_vao);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, translations.size() / 2);
        glBindVertexArray(0);

        SDL_GL_SwapWindow(m_window);
        ++m_frame;

        uint64_t frame_time = SDL_GetTicksNS() - frame_start;
        if(frame_time < m_ns_per_frame) {
                SDL_DelayNS(m_ns_per_frame - frame_time);
        }
}

void Renderer::clear_screen()
{
        float background = 0x20 / (float)0xff;
        glClearColor(background, background, background, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
