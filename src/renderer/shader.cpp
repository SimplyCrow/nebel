#include <spdlog/spdlog.h>
#include <glad/gl.h>

#include <fstream>
#include <string>
#include <filesystem>

#include "shader.hpp"
#include "../panic.hpp"
#include "logger.hpp"

namespace renderer {

        Shader::Shader(Shader::path vertex, Shader::path fragment)
        {
                info("Loading v: {} f: {}", vertex.string(), fragment.string());
                GLuint vertex_shader = compile(vertex, GL_VERTEX_SHADER);
                GLuint fragment_shader = compile(fragment, GL_FRAGMENT_SHADER);
                m_program = link(vertex_shader, fragment_shader);
                glDeleteShader(vertex_shader);
                glDeleteShader(fragment_shader);
        }

        std::string Shader::read(Shader::path &path)
        {
                std::ifstream file(path);
                if(!file.is_open()) {
                        PANIC(renderer, "Could not open shader file: {}", path.string());
                }

                return std::string((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
        }

        GLuint Shader::compile(Shader::path &path, GLenum type)
        {
                std::string content = read(path);
                GLuint shader = glCreateShader(type);
                const char *code = content.c_str();
                glShaderSource(shader, 1, &code, NULL);
                glCompileShader(shader);

                GLint status;
                glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
                if(status == GL_TRUE) {
                        return shader;
                }

                GLint max_error_length = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_error_length);
                char *error = new char[max_error_length];
                glGetShaderInfoLog(shader, max_error_length, &max_error_length, error);
                PANIC(renderer, "Compilation error. shader: {}\n{}", path.string(), error);
                delete[] error;
                return 0;
        }

        GLuint Shader::link(GLuint vertex, GLuint fragment)
        {
                GLuint program = glCreateProgram();
                glAttachShader(program, vertex);
                glAttachShader(program, fragment);
                glLinkProgram(program);

                GLint status;
                glGetProgramiv(program, GL_LINK_STATUS, &status);
                if(status == GL_TRUE) {
                        return program;
                }

                GLint max_error_length = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_error_length);
                char *error = new char[max_error_length];
                glGetProgramInfoLog(program, max_error_length, &max_error_length, error);
                PANIC(renderer, "linking error\n%s", error);
                delete[] error;
                return 0;
        }

}; // namespace renderer
