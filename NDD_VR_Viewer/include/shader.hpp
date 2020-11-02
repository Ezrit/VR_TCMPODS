#ifndef __MSI_VR__SHADER_HPP__
#define __MSI_VR__SHADER_HPP__

#include <string>
#include <map>
#include <functional>

#include <GL/glew.h>

namespace msi_vr
{
    class Shader
    {
        public:

        Shader();
        Shader(std::string const &vertexShaderPath, std::string const &fragmentShaderPath);

        void activate(std::string const &set = "base") const;
        void applyUniforms(std::string const &set = "base") const;

        GLint getUniform(std::string const &name);
        GLint getUniform(std::string const &name) const;
        bool attachUniform(std::string const &name, std::function<void(GLuint location)> attachFunction, std::string const &set = "base");

        bool loadShader(std::string const &vertexShaderPath, std::string const &fragmentShaderPath);

        GLuint programID;
        private:
        GLuint vertexShader;
        GLuint fragmentShader;

        std::map<std::string, GLint> uniformMap;
        std::map<std::string, std::map<GLuint, std::function<void(GLuint location)> > > applyUniformSets;
    };
} // namespace msi_vr

#endif