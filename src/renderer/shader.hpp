#pragma once
#ifndef NEBEL_RENDERER_SHADER_HPP
#define NEBEL_RENDERER_SHADER_HPP

#include <filesystem>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
        using path=std::filesystem::path;
        Shader() = default;
        Shader(Shader::path vertex, Shader::path fragment);

        void use()
        {
                glUseProgram(m_program);
        }

        void set_bool(const std::string &name, bool value) const
        {
                int location = glGetUniformLocation(m_program, name.c_str());
                if(location == -1) {
                        return;
                }
                glUniform1i(location, (int)value);
        }

        void set_int(const std::string &name, int value) const
        {
                int location = glGetUniformLocation(m_program, name.c_str());
                if(location == -1) {
                        return;
                }
                glUniform1i(location, value);
        }

        void set_float(const std::string &name, float value) const
        {
                int location = glGetUniformLocation(m_program, name.c_str());
                if(location == -1) {
                        return;
                }
                glUniform1f(location, value);
        }

        void set_vec3(const std::string &name, float x, float y, float z) const
        {
                int location = glGetUniformLocation(m_program, name.c_str());
                if(location == -1) {
                        return;
                }
                glUniform3f(location, x, y, z);
        }

        void set_vec3(const std::string &name, glm::vec3 vec) const
        {
                int location = glGetUniformLocation(m_program, name.c_str());
                if(location == -1) {
                        return;
                }
                glUniform3f(location, vec.x, vec.y, vec.z);
        }

        void set_mat4(const std::string &name, const glm::mat4 &value) const
        {
                int location = glGetUniformLocation(m_program, name.c_str());
                if(location == -1) {
                        return;
                }
                glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
        }


        ~Shader() = default;
private:
        GLuint m_program = 0;

        std::string read(Shader::path &path);
        GLuint      compile(Shader::path &path, GLenum type);
        GLuint      link(GLuint vertex, GLuint fragment);
};

#endif // NEBEL_RENDERER_SHADER_HPP
