#include <spdlog/spdlog.h>
#include <SDL3/SDL.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer.hpp"
#include "../panic.hpp"

Renderer::Renderer(int initial_width, int initial_height, uint64_t target_fps, sim::Simulation &simulation)
        : m_simulation(simulation)
{
        m_window = {
                .running = true,
                .width   = initial_width,
                .height  = initial_height,
                .window  = nullptr
        };

        m_frame = {
                .current_frame = 0, .target_fps    = target_fps,
                .ns_per_frame  = 1000000000ULL / target_fps,
                .start_frame_time = 0,
                .last_frame_time  = 0,
                .delta_time       = 0,
                .total_runtime    = 0,
                .fps_timer        = 0,
                .fps_count        = 0
        };

        m_gl = {
                .context = nullptr,
                .shaders = {},
                .instanced_square_vao = 0,
                .unit_square_vao = 0,
        };

        m_camera = {
                .position = {0, 0},
                .zoom = 15.0f
        };

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

        m_window.window = SDL_CreateWindow("Nebel", m_window.width, m_window.height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        if(m_window.window == nullptr) {
                PANIC("(Renderer) Could not create window");
        }

        spdlog::info("(Renderer) Window created w={} h={} target-fps={}", m_window.width, m_window.height, m_frame.target_fps);
}

void Renderer::setup_opengl()
{
        m_gl.context = SDL_GL_CreateContext(m_window.window);
        if(!m_gl.context) {
                PANIC("(Renderer) Could not create OpenGL context: {}", SDL_GetError());
        }

        SDL_GL_SetSwapInterval(1);

        int open_version = gladLoadGL(SDL_GL_GetProcAddress);
        if(open_version == 0) {
                PANIC("Could not load OpenGL functions");
        }

        spdlog::info("(Renderer) OpenGL version loaded: {}.{}", GLAD_VERSION_MAJOR(open_version), GLAD_VERSION_MINOR(open_version));

        glEnable(GL_BLEND);

        glViewport(0, 0, m_window.width, m_window.height);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::setup_render_data()
{
        spdlog::info("(Renderer) Initializes render data");

        load_shader("circle", "./res/shader/instanced_vertex.vs", "./res/shader/circle.fs");
        load_shader("grid", "./res/shader/vertex.vs", "./res/shader/grid.fs");
        load_shader("origin", "./res/shader/vertex.vs", "./res/shader/origin.fs");

        float vertices[] = {
                /* bl */ -0.5f, -0.5f,   0.0f, 0.0f,
                /* tl */ -0.5f,  0.5f,   0.0f, 1.0f,
                /* br */  0.5f, -0.5f,   1.0f, 0.0f,
                /* tl */ -0.5f,  0.5f,   0.0f, 1.0f,
                /* tr */  0.5f,  0.5f,   1.0f, 1.0f,
                /* br */  0.5f, -0.5f,   1.0f, 0.0f,
        };

        GLuint VBO;

        // unit square
        glCreateBuffers(1, &VBO);
        glNamedBufferStorage(VBO, sizeof(vertices), vertices, GL_DYNAMIC_STORAGE_BIT);
        glCreateVertexArrays(1, &m_gl.unit_square_vao);
        glVertexArrayVertexBuffer(m_gl.unit_square_vao, 0, VBO, 0, 4 * sizeof(float));
        // position
        glEnableVertexArrayAttrib(m_gl.unit_square_vao, 0);
        glVertexArrayAttribFormat(m_gl.unit_square_vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_gl.unit_square_vao, 0, 0);
        // texture coord
        glEnableVertexArrayAttrib(m_gl.unit_square_vao, 1);
        glVertexArrayAttribFormat(m_gl.unit_square_vao, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float));
        glVertexArrayAttribBinding(m_gl.unit_square_vao, 1, 0);

        // instanced square
        glCreateBuffers(1, &VBO);
        glNamedBufferStorage(VBO, sizeof(vertices), vertices, GL_DYNAMIC_STORAGE_BIT);
        glCreateVertexArrays(1, &m_gl.instanced_square_vao);
        glVertexArrayVertexBuffer(m_gl.instanced_square_vao, 0, VBO, 0, 4 * sizeof(float));
        // position
        glEnableVertexArrayAttrib(m_gl.instanced_square_vao, 0);
        glVertexArrayAttribFormat(m_gl.instanced_square_vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_gl.instanced_square_vao, 0, 0);
        // texture coord
        glEnableVertexArrayAttrib(m_gl.instanced_square_vao, 1);
        glVertexArrayAttribFormat(m_gl.instanced_square_vao, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float));
        glVertexArrayAttribBinding(m_gl.instanced_square_vao, 1, 0);
        // instance offset
        glEnableVertexArrayAttrib(m_gl.instanced_square_vao, 2);
        glVertexArrayAttribFormat(m_gl.instanced_square_vao, 2, 2, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(m_gl.instanced_square_vao, 2, 1);

        const World &world = m_simulation.get_world();
        m_camera.position = 0.5f * glm::vec2(world.width(), world.height());
        calculate_camera();

        m_frame.last_frame_time = SDL_GetTicksNS();
}

void Renderer::load_shader(std::string name, std::string vertex_path, std::string fragment_path)
{
        auto [it, inserted] = m_gl.shaders.try_emplace(name, vertex_path, fragment_path);
        if(!inserted) {
                PANIC("(Renderer) Tried to load shader with an already used name \"{}\" (vs={}, fs={})", name, vertex_path, fragment_path);
        }
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
                        m_window.running = false;
                        continue;
                }

                if(event.type == SDL_EVENT_WINDOW_RESIZED) {
                        m_window.width = event.window.data1;
                        m_window.height = event.window.data2;
                        glViewport(0, 0, m_window.width, m_window.height);
                        spdlog::info("(Renderer) Resized w={} h={}", m_window.width, m_window.height);
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
                glm::vec2 mov = dir * (m_frame.delta_time * speed);
                m_camera.position += mov;
                calculate_camera();
        }

        if(zoom_direction) {
                m_camera.zoom += zoom_direction * zoom_speed * m_frame.delta_time;
                if(m_camera.zoom < 0.0f)
                        m_camera.zoom = 0.0f;
        }
}

void Renderer::calculate_camera()
{
}

void Renderer::frame_start()
{
        m_frame.start_frame_time = SDL_GetTicksNS();
        m_frame.total_runtime = static_cast<float>(m_frame.start_frame_time / 1000000000.0f);
        m_frame.delta_time = static_cast<float>(
                (m_frame.start_frame_time - m_frame.last_frame_time) / 1000000000.0f
        );
        m_frame.fps_timer += m_frame.delta_time;

        handle_events();
        clear_screen();
}

void Renderer::frame_end()
{
        uint64_t frame_time = SDL_GetTicksNS() - m_frame.start_frame_time;
        if(frame_time < m_frame.ns_per_frame) {
                SDL_DelayNS(m_frame.ns_per_frame - frame_time);
        }
        m_frame.last_frame_time = m_frame.start_frame_time;
        SDL_GL_SwapWindow(m_window.window);
        ++m_frame.current_frame;
        ++m_frame.fps_count;
        if(m_frame.fps_timer > 5.0f) {

                float fps = m_frame.fps_count / m_frame.fps_timer;
                spdlog::info("(Renderer) fps: {}", fps);

                m_frame.fps_timer = 0.0f;
                m_frame.fps_count = 0;
        }
}

void Renderer::calculate_matrices(glm::mat4 &view, glm::mat4 &projection)
{
        view = glm::translate(glm::mat4(1.0f), glm::vec3(-m_camera.position, 0.0f));

        float aspect_ratio = static_cast<float>(m_window.width) / static_cast<float>(m_window.height);
        float world_width = 10.0f;
        float world_height = world_width / aspect_ratio;

        float left   = (-world_width / 2.0f) * m_camera.zoom;
        float right  = ( world_width / 2.0f) * m_camera.zoom;
        float bottom = (-world_height / 2.0f) * m_camera.zoom;
        float top    = ( world_height / 2.0f) * m_camera.zoom;
        projection = glm::ortho(left, right, bottom, top, -10.0f, 10.0f);
}

void Renderer::iteration()
{
        frame_start();

        glm::mat4 view_matrix, projection_matrix;
        calculate_matrices(view_matrix, projection_matrix);

        const World &world = m_simulation.get_world();
        const auto entities = world.get_entities();
        std::vector<float> translations;
        translations.reserve(2 * entities.size());
        for(const Entity& entity : entities) {
                translations.emplace_back(entity.x);
                translations.emplace_back(entity.y);
        }

        GLuint translations_buffer;
        glCreateBuffers(1, &translations_buffer);
        glNamedBufferStorage(translations_buffer, translations.size() * sizeof(float), translations.data(), GL_DYNAMIC_STORAGE_BIT);
        glVertexArrayVertexBuffer(m_gl.instanced_square_vao, 1, translations_buffer, 0, 2 * sizeof(float));
        glVertexArrayBindingDivisor(m_gl.instanced_square_vao, 1, 1);

        glm::mat4 model_grid = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        model_grid = glm::rotate(model_grid, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model_grid = glm::scale(model_grid, glm::vec3(100));
        m_gl.shaders["grid"].use();
        m_gl.shaders["grid"].set_vec2("uAnchor", glm::vec2(-0.5f));
        m_gl.shaders["grid"].set_mat4("uModel", model_grid);
        m_gl.shaders["grid"].set_mat4("uView", view_matrix);
        m_gl.shaders["grid"].set_mat4("uProjection", projection_matrix);
        m_gl.shaders["grid"].set_int("uGridWidth", world.width());
        m_gl.shaders["grid"].set_int("uGridHeight", world.height());
        glBindVertexArray(m_gl.unit_square_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glm::mat4 model_circle = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.0f));
        model_circle = glm::rotate(model_circle, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model_circle = glm::scale(model_circle, glm::vec3(1.0f));

        m_gl.shaders["circle"].use();
        m_gl.shaders["circle"].set_vec2("uAnchor", glm::vec2(0.0f));
        m_gl.shaders["circle"].set_mat4("uModel", model_circle);
        m_gl.shaders["circle"].set_mat4("uView", view_matrix);
        m_gl.shaders["circle"].set_mat4("uProjection", projection_matrix);
        glBindVertexArray(m_gl.instanced_square_vao);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, translations.size() / 2);
        glBindVertexArray(0);
        glDeleteBuffers(1, &translations_buffer);

        glm::mat4 model_origin = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        model_origin = glm::rotate(model_origin, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model_origin = glm::scale(model_origin, glm::vec3(1.0f));
        m_gl.shaders["origin"].use();
        m_gl.shaders["origin"].set_vec2("uAnchor", glm::vec2(0.0f));
        m_gl.shaders["origin"].set_mat4("uModel", model_origin);
        m_gl.shaders["origin"].set_mat4("uView", view_matrix);
        m_gl.shaders["origin"].set_mat4("uProjection", projection_matrix);
        glBindVertexArray(m_gl.unit_square_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        frame_end();
}

void Renderer::clear_screen()
{
        float background = 0x20 / (float)0xff;
        glClearColor(background, background, background, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
