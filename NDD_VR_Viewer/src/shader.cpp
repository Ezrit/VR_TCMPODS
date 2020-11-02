#include "shader.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

namespace msi_vr
{
    Shader::Shader()
    {
    }

    Shader::Shader(std::string const &vertexShaderPath, std::string const &fragmentShaderPath)
    {
        loadShader(vertexShaderPath, fragmentShaderPath);
    }

    void Shader::activate(std::string const &set) const
    {
        glUseProgram(programID);

        applyUniforms(set);
    }

    void Shader::applyUniforms(std::string const &set) const
    {
        auto it = applyUniformSets.find(set);
        if(it != applyUniformSets.end())
        {
            for(auto &uniform : it->second)
            {
                uniform.second(uniform.first);
            }
        }
    }

    GLint Shader::getUniform(std::string const &name)
    {
        auto it = uniformMap.find(name);

        if(it == uniformMap.end()) 
        {
            uniformMap[name] = glGetUniformLocation(programID, name.c_str());
            return uniformMap[name];
        }

        return it->second;
    }

    GLint Shader::getUniform(std::string const &name) const
    {
        auto it = uniformMap.find(name);

        if(it == uniformMap.end()) 
        {
            return 0;
        }

        return it->second;
    }

    bool Shader::attachUniform(std::string const &name, std::function<void(GLuint)> attachFunction, std::string const &set)
    {
        auto it = applyUniformSets.find(set);
        if(it == applyUniformSets.end())
        {
            applyUniformSets[set] = {};
        }
        auto &applyUniform = applyUniformSets[set];

        GLint uniformLocation = getUniform(name);
        if(uniformLocation < 0) return false;

        applyUniform[uniformLocation] = attachFunction;
        return true;
    }

    bool Shader::loadShader(std::string const &vertexShaderPath, std::string const &fragmentShaderPath)
    {
        // Create the shaders
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        // Read the Vertex Shader code from the file
        std::string VertexShaderCode;
        std::ifstream VertexShaderStream(vertexShaderPath, std::ios::in);
        if (VertexShaderStream.is_open())
        {
            std::stringstream sstr;
            sstr << VertexShaderStream.rdbuf();
            VertexShaderCode = sstr.str();
            VertexShaderStream.close();
        }
        else
        {
            std::cerr << "Impossible to open " << vertexShaderPath << ". Are you in the right directory ? Don't forget to read the FAQ !" << std::endl;
            getchar();
            return false;
        }

        // Read the Fragment Shader code from the file
        std::string FragmentShaderCode;
        std::ifstream FragmentShaderStream(fragmentShaderPath, std::ios::in);
        if (FragmentShaderStream.is_open())
        {
            std::stringstream sstr;
            sstr << FragmentShaderStream.rdbuf();
            FragmentShaderCode = sstr.str();
            FragmentShaderStream.close();
        }

        GLint Result = GL_FALSE;
        int InfoLogLength;

        // Compile Vertex Shader
        std::cout << "Compiling shader: " << vertexShaderPath << std::endl;
        char const *VertexSourcePointer = VertexShaderCode.c_str();
        glShaderSource(vertexShader, 1, &VertexSourcePointer, NULL);
        glCompileShader(vertexShader);

        // Check Vertex Shader
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength > 0)
        {
            std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
            glGetShaderInfoLog(vertexShader, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
            printf("%s\n", &VertexShaderErrorMessage[0]);
            return false;
        }

        // Compile Fragment Shader
        std::cout << "Compiling shader: " << fragmentShaderPath << std::endl;
        char const *FragmentSourcePointer = FragmentShaderCode.c_str();
        glShaderSource(fragmentShader, 1, &FragmentSourcePointer, NULL);
        glCompileShader(fragmentShader);

        // Check Fragment Shader
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &Result);
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength > 0)
        {
            std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
            glGetShaderInfoLog(fragmentShader, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
            printf("%s\n", &FragmentShaderErrorMessage[0]);
            return false;
        }

        // Link the program
        printf("Linking program\n");
        programID = glCreateProgram();
        glAttachShader(programID, vertexShader);
        glAttachShader(programID, fragmentShader);
        glLinkProgram(programID);

        // Check the program
        glGetProgramiv(programID, GL_LINK_STATUS, &Result);
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength > 0)
        {
            std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
            glGetProgramInfoLog(programID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
            printf("%s\n", &ProgramErrorMessage[0]);
            return false;
        }

        glDetachShader(programID, vertexShader);
        glDetachShader(programID, fragmentShader);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return true;
    }
} // namespace msi_vr