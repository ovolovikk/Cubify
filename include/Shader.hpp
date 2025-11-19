#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>

#include <GL/glew.h>

using std::string;

class Shader
{
public:
    Shader(string vertex_shader_path, string fragment_shader_path);
    ~Shader();
private:
    GLuint vertex_shader_ID;
    GLuint fragment_shader_ID;
    GLuint program_ID; 

    void readShaderCode();
};

#endif // SHADER_HPP