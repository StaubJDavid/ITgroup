#pragma once
#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    unsigned int ID;

    Shader();

    void setFromSource(const char* vertexSource, const char* fragmentSource); // Create a program from vertex, fragment source string
    void setFromFile(const char* vertexPath, const char* fragmentPath); // Create a program vertex, fragment shader files from given path

    void use(); // Active shader program

    // Utility functions to set uniform's value
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;

private:
    void checkCompileErrors(unsigned int shader, std::string type); // Utility function for checking shader compilation/linking errors
};