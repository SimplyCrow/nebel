#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL3/SDL.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H


#include "renderer.hpp"
#include "logger.hpp"
#include "../panic.hpp"
#include "../simulation/world.hpp"

namespace renderer {

        Renderer::Renderer(Settings settings)
        {
                m_window = {
                        .running = true,
                        .width   = settings.window_width,
                        .height  = settings.window_height,
                        .window  = nullptr,
                        .target = {
                                .width   = settings.window_width,
                                .height  = settings.window_height,
                        }
                };

                m_frame = {
                        .current_frame = 0,
                        .target_fps    = settings.target_fps,
                        .ns_per_frame  = 1000000000ULL / settings.target_fps,
                        .start_frame_time = 0,
                        .last_frame_time  = 0,
                        .delta_time       = 0,
                        .total_runtime    = 0,
                        .fps_timer        = 0,
                };

                m_gl = {
                        .context = nullptr,
                        .shaders = {},
                        .instanced_square_vao = 0,
                        .unit_square_vao = 0,
                };

                setup_sdl();
                setup_opengl();
                setup_imgui();
                setup_render_data();
        }

        void Renderer::setup_sdl()
        {
                int sdl_version = SDL_GetVersion();
                info("Initializing SDL version: {}.{}.{}"
                                , SDL_VERSIONNUM_MAJOR(sdl_version)
                                , SDL_VERSIONNUM_MINOR(sdl_version)
                                , SDL_VERSIONNUM_MICRO(sdl_version));

                if(!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
                        PANIC(renderer, "Could not initialize SDL subsystems");
                }

                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
                SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
                SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 24);
                SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

                m_window.window = SDL_CreateWindow("Nebel", m_window.width, m_window.height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
                if(m_window.window == nullptr) {
                        PANIC(renderer, "Could not create window");
                }

                info("Window created w={} h={} target-fps={}", m_window.width, m_window.height, m_frame.target_fps);
                info("Setuped SDL");
        }

        void Renderer::setup_opengl()
        {
                info("Setup opengl");
                m_gl.context = SDL_GL_CreateContext(m_window.window);
                if(!m_gl.context) {
                        PANIC(renderer, "Could not create OpenGL context: {}", SDL_GetError());
                }

                SDL_GL_SetSwapInterval(0);

                int open_version = gladLoadGL(SDL_GL_GetProcAddress);
                if(open_version == 0) {
                        PANIC(renderer, "Could not load OpenGL functions");
                }

                info("OpenGL version loaded: {}.{}", GLAD_VERSION_MAJOR(open_version), GLAD_VERSION_MINOR(open_version));

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                resize_render_target(m_window.target, m_window.target.width, m_window.target.height);

                info("Setuped opengl");
        }

        void Renderer::setup_imgui()
        {
                info("Setup imgui");
                float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
                info("scale: {}", main_scale);

                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO(); (void)io;
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
                io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

                ImGuiStyle& style = ImGui::GetStyle();
                ImGui::StyleColorsDark();
                style.FrameRounding = 3.0f;
                style.GrabRounding = 3.0f;
                style.ScaleAllSizes(main_scale);
                style.FontScaleDpi = main_scale;

                io.Fonts->AddFontFromFileTTF("./res/fonts/font.ttf");

                ImGui_ImplSDL3_InitForOpenGL(m_window.window, m_gl.context);
                ImGui_ImplOpenGL3_Init("#version 460");

                info("Setuped imgui");
        }

        void Renderer::setup_render_data()
        {
                info("Initializes render data");

                load_shader("circle", "./res/shader/instanced_vertex.vs", "./res/shader/circle.fs");
                load_shader("grid", "./res/shader/vertex.vs", "./res/shader/grid.fs");
                load_shader("heat_grid", "./res/shader/vertex.vs", "./res/shader/heat_grid.fs");
                load_shader("origin", "./res/shader/vertex.vs", "./res/shader/origin.fs");
                load_shader("font", "./res/shader/font.vs", "./res/shader/font.fs");

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

                // font
                glCreateVertexArrays(1, &m_chars_vao);
                glCreateBuffers(1, &m_chars_vbo);
                glNamedBufferData(m_chars_vbo, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
                glVertexArrayVertexBuffer(m_chars_vao, 0, m_chars_vbo, 0, 4 * sizeof(float));
                // vertex
                glEnableVertexArrayAttrib(m_chars_vao, 0);
                glVertexArrayAttribFormat(m_chars_vao, 0, 4, GL_FLOAT, GL_FALSE, 0);
                glVertexArrayAttribBinding(m_chars_vao, 0, 0);

                calculate_camera();

                FT_Library ft;
                if(FT_Init_FreeType(&ft)) {
                        PANIC(renderer, "Could not init free type");
                }

                int free_major = 0, free_minor = 0, free_patch = 0;
                FT_Library_Version(ft, &free_major, &free_minor, &free_patch);
                info("free type version: {}.{}.{}", free_major, free_minor, free_patch);

                FT_Face face;
                if(FT_New_Face(ft, "./res/fonts/font.ttf", 0, &face)) {
                        PANIC(renderer, "Could not load font");
                }

                FT_Set_Pixel_Sizes(face, 0, 48);

                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                for(unsigned char c = 0; c < 128; ++c) {
                        if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                                PANIC(renderer, "Could not load glyph {}", c);
                        }

                        unsigned int texture;
                        glGenTextures(1, &texture);
                        glBindTexture(GL_TEXTURE_2D, texture);
                        glTexImage2D(
                                GL_TEXTURE_2D,
                                0,
                                GL_RED,
                                face->glyph->bitmap.width,
                                face->glyph->bitmap.rows,
                                0,
                                GL_RED,
                                GL_UNSIGNED_BYTE,
                                face->glyph->bitmap.buffer
                        );
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        Char character = {
                                texture,
                                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                                face->glyph->advance.x,
                        };
                        m_chars.insert(std::pair<char, Char>(c, character));
                }

                FT_Done_Face(face);
                FT_Done_FreeType(ft);

                m_frame.last_frame_time = SDL_GetTicksNS();
                info("Setuped render data");
        }

        void Renderer::load_shader(std::string name, std::string vertex_path, std::string fragment_path)
        {
                auto [it, inserted] = m_gl.shaders.try_emplace(name, vertex_path, fragment_path);
                if(!inserted) {
                        PANIC(renderer, "Tried to load shader with an already used name \"{}\" (vs={}, fs={})", name, vertex_path, fragment_path);
                }
        }

        Renderer::~Renderer()
        {
                SDL_Quit();
                info("Exited");
        }

        void Renderer::handle_events()
        {
                SDL_Event event;
                while(SDL_PollEvent(&event)) {
                        ImGui_ImplSDL3_ProcessEvent(&event);
                        if(event.type == SDL_EVENT_QUIT) {
                                m_window.running = false;
                                continue;
                        }

                        if(event.type == SDL_EVENT_WINDOW_RESIZED) {
                                m_window.width = event.window.data1;
                                m_window.height = event.window.data2;
                                info("Resized Window w={} h={}", m_window.width, m_window.height);
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
                        dir = glm::normalize(dir);
                        m_camera.position += dir * (m_camera.speed * m_camera.zoom) * m_frame.delta_time;
                        calculate_camera();
                }

                if(zoom_direction) {
                        m_camera.zoom += zoom_direction * zoom_speed * m_frame.delta_time;
                        m_camera.zoom = std::max(m_camera.zoom, m_camera.minimum_zoom);
                }
        }

        void Renderer::calculate_camera()
        {
        }

        void Renderer::render_text(Shader &s, std::string text, float x, float y, float scale, glm::vec3 color)
        {
                s.use();
                s.set_vec3("text_color", color);
                glActiveTexture(GL_TEXTURE0);
                glBindVertexArray(m_chars_vao);

                std::string::const_iterator c;
                for(c = text.begin(); c != text.end(); ++c) {
                        Char character = m_chars[*c];

                        float xpos = x + character.bearing.x * scale;
                        float ypos = y - (character.size.y - character.bearing.y) * scale;

                        float w = character.size.x * scale;
                        float h = character.size.y * scale;
                        float vertices[6][4] = {
                                {xpos,     ypos + h,   0.0f, 0.0f},
                                {xpos,     ypos,       0.0f, 1.0f},
                                {xpos + w, ypos,       1.0f, 1.0f},

                                {xpos,     ypos + h,   0.0f, 0.0f},
                                {xpos + w, ypos,       1.0f, 1.0f},
                                {xpos + w, ypos + h,   1.0f, 0.0f},
                        };

                        glBindTexture(GL_TEXTURE_2D, character.texture);
                        glNamedBufferSubData(m_chars_vbo, 0, sizeof(vertices), vertices);
                        glDrawArrays(GL_TRIANGLES, 0, 6);

                        x += (character.advance >> 6) * scale;
                }
                glBindVertexArray(0);
                glBindTexture(GL_TEXTURE_2D, 0);
        }

        void Renderer::resize_render_target(RenderTarget &target, int width, int height)
        {
                if(width <= 0 || height <= 0 ||
                        (target.width == width && height == target.height && target.fbo != 0))
                        return;

                if(target.fbo == 0)
                        glCreateFramebuffers(1, &target.fbo);

                if(target.color_texture != 0)
                        glDeleteTextures(1, &target.color_texture);

                glCreateTextures(GL_TEXTURE_2D, 1, &target.color_texture);
                glTextureStorage2D(target.color_texture, 1, GL_RGBA8, width, height);
                glTextureParameteri(target.color_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(target.color_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(target.color_texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(target.color_texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                if(target.depth_rbo != 0)
                        glDeleteRenderbuffers(1, &target.depth_rbo);
                glCreateRenderbuffers(1, &target.depth_rbo);
                glNamedRenderbufferStorage(target.depth_rbo, GL_DEPTH24_STENCIL8, width, height);

                glNamedFramebufferTexture(target.fbo, GL_COLOR_ATTACHMENT0, target.color_texture, 0);
                glNamedFramebufferRenderbuffer(target.fbo, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, target.depth_rbo);

                if(glCheckNamedFramebufferStatus(target.fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                        PANIC(renderer, "Could not resize frame buffer object");

                target.width = width;
                target.height = height;
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

                glViewport(0, 0, m_window.target.width, m_window.target.height);

                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplSDL3_NewFrame();
                ImGui::NewFrame();
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
                m_frame.current_fps = 1.0f / m_frame.delta_time;
                if(m_frame.fps_timer > 5.0f) {
                        info("fps: {}", m_frame.current_fps);
                        m_frame.fps_timer = 0.0f;
                }

        }

        void Renderer::calculate_matrices(glm::mat4 &view, glm::mat4 &projection, glm::mat4 &ui_projection)
        {
                view = glm::translate(glm::mat4(1.0f), glm::vec3(-m_camera.position, 0.0f));

                float aspect_ratio = static_cast<float>(m_window.target.width) / static_cast<float>(m_window.target.height);
                float world_width = 10.0f;
                float world_height = world_width / aspect_ratio;

                float left   = (-world_width / 2.0f) * m_camera.zoom;
                float right  = ( world_width / 2.0f) * m_camera.zoom;
                float bottom = (-world_height / 2.0f) * m_camera.zoom;
                float top    = ( world_height / 2.0f) * m_camera.zoom;
                projection = glm::ortho(left, right, bottom, top, -10.0f, 10.0f);
                ui_projection = glm::ortho(0.0f, static_cast<float>(m_window.target.width), 0.0f, static_cast<float>(m_window.target.height));
        }

        void Renderer::set_shader_mvp(Shader &shader, glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection)
        {
                shader.set_mat4("uModel", model);
                shader.set_mat4("uView", view);
                shader.set_mat4("uProjection", projection);
        }

        void Renderer::set_shader_anchor(Shader &shader, glm::vec2 anchor)
        {
                shader.set_vec2("uAnchor", anchor);
        }

        void Renderer::render_grid(glm::mat4 &view, glm::mat4 &projection, int width, int height)
        {
                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
                model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                model = glm::scale(model, glm::vec3(width, height, 1.0f));

                Shader &shader = m_gl.shaders["grid"];
                shader.use();
                set_shader_mvp(shader, model, view, projection);
                set_shader_anchor(shader, glm::vec2(-0.5f));

                shader.set_int("uGridWidth", width);
                shader.set_int("uGridHeight", height);

                glBindVertexArray(m_gl.unit_square_vao);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);
        }

        void Renderer::iteration(simulation::WorldBuffer::SharedPtr world_buffer)
        {
                frame_start();

                glBindFramebuffer(GL_FRAMEBUFFER, m_window.target.fbo);
                clear_screen();

                glm::mat4 view_matrix, projection_matrix, ui_projection_matrix;
                calculate_matrices(view_matrix, projection_matrix, ui_projection_matrix);
                auto &world = world_buffer->acquire_render_world();

                render_grid(view_matrix, projection_matrix, world.width(), world.height());

                const auto entities = world.get_entities();
                std::vector<float> translations;
                translations.reserve(2 * entities.size());
                for(const simulation::Entity& entity : entities) {
                        translations.emplace_back(entity.x);
                        translations.emplace_back(entity.y);
                }

                GLuint translations_buffer;
                glCreateBuffers(1, &translations_buffer);
                glNamedBufferStorage(translations_buffer, translations.size() * sizeof(float), translations.data(), GL_DYNAMIC_STORAGE_BIT);
                glVertexArrayVertexBuffer(m_gl.instanced_square_vao, 1, translations_buffer, 0, 2 * sizeof(float));
                glVertexArrayBindingDivisor(m_gl.instanced_square_vao, 1, 1);

                glm::mat4 model_circle = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.0f));
                model_circle = glm::rotate(model_circle, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                model_circle = glm::scale(model_circle, glm::vec3(1.0f));

                m_gl.shaders["circle"].use();
                m_gl.shaders["circle"].set_vec2("uAnchor", glm::vec2(0.0f));
                m_gl.shaders["circle"].set_mat4("uModel", model_circle);
                m_gl.shaders["circle"].set_mat4("uView", view_matrix); m_gl.shaders["circle"].set_mat4("uProjection", projection_matrix);

                glBindVertexArray(m_gl.instanced_square_vao);
                glDrawArraysInstanced(GL_TRIANGLES, 0, 6, translations.size() / 2);
                glBindVertexArray(0);
                glDeleteBuffers(1, &translations_buffer);
                glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);

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

                m_gl.shaders["font"].use();
                m_gl.shaders["font"].set_mat4("projection", ui_projection_matrix);
                render_text(m_gl.shaders["font"], std::to_string(m_frame.current_fps), 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                static float inspector_width = 300.0f;
                const float splitter_thickness = 6.0f;

                ImGuiViewport *vp = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(vp->WorkPos);
                ImGui::SetNextWindowSize(vp->WorkSize);

                ImGuiWindowFlags root_flags =
                        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

                ImGui::Begin("Nebel", nullptr, root_flags);

                float avail_w = ImGui::GetContentRegionAvail().x;
                float avail_h = ImGui::GetContentRegionAvail().y;
                if(inspector_width < 150.0f)
                        inspector_width = 150.0f;
                if(inspector_width > avail_w - 200.0f)
                        inspector_width = avail_w - 200.0f;

                ImGui::BeginChild("Inspector", ImVec2(inspector_width, avail_h), true);
                ImGui::Text("Inspector");
                ImGui::Text("fps: %.2f", m_frame.current_fps);
                ImGui::Text("zoom: %.2f", m_camera.zoom);
                ImGui::Text("camera position: x=%.2f y=%.2f", m_camera.position.x, m_camera.position.y);
                ImGui::Text("iteration: %zu", world.get_iteration());
                if(ImGui::Button("Hello")) {
                        ImGui::Text("Test");
                }
                ImGui::EndChild();

                ImGui::SameLine(0.0f, 0.0f);
                ImGui::InvisibleButton("##Splitter", ImVec2(splitter_thickness, avail_h));
                if(ImGui::IsItemHovered() || ImGui::IsItemActive())
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                bool dragging = ImGui::IsItemActive();
                if(dragging)
                        inspector_width += ImGui::GetIO().MouseDelta.x;

                ImGui::SameLine(0.0f, 0.0f);

                ImGui::BeginChild("Viewport", ImVec2(0.0f, avail_h), false, ImGuiWindowFlags_NoScrollbar);
                ImVec2 view_size = ImGui::GetContentRegionAvail();
                int w = static_cast<int>(view_size.x);
                int h = static_cast<int>(view_size.y);

                if(w > 0 && h > 0 && !dragging) {
                        if(m_window.target.width != w || m_window.target.height != h) {
                                resize_render_target(m_window.target, w, h);
                        }
                        ImGui::Image(
                                (ImTextureID)m_window.target.color_texture,
                                ImVec2(static_cast<float>(m_window.target.width), static_cast<float>(m_window.target.height)),
                                ImVec2(0.0f, 1.0f),
                                ImVec2(1.0f, 0.0f)
                        );
                } else {
                        const char* text = "RESIZING";
                        ImGui::SetWindowFontScale(10.0f);
                        ImVec2 windowSize = ImGui::GetWindowSize();
                        ImVec2 textSize   = ImGui::CalcTextSize(text);
                        ImGui::SetCursorPosX((windowSize.x - textSize.x) * 0.5f);
                        ImGui::SetCursorPosY((windowSize.y - textSize.y) * 0.5f);
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%s", text);
                        ImGui::SetWindowFontScale(1.0f);
                }

                ImGui::EndChild();

                ImGui::End();

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                frame_end();
        }

        void Renderer::clear_screen()
        {
                float background = 0x20 / (float)0xff;
                glClearColor(background, background, background, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

}; // namespace renderer

