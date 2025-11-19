#include "Shader.hpp"

#include <sstream>
#include <fstream>
#include <iostream>

Shader::Shader(string vertex_shader_path, string fragment_shader_path)
{
    string vertex_shader_code;
    std::ifstream vertex_shader_stream(vertex_shader_path, std::ios::in);
    if(vertex_shader_stream.is_open())
    {
        std::stringstream sstr;
        sstr << vertex_shader_stream.rdbuf();
        vertex_shader_code = sstr.str();
        vertex_shader_stream.close();
    }
    else
    {
        std::cout << "vertex shader init error.\n";
        return 0;
    }

}

Shader::~Shader()
{

}