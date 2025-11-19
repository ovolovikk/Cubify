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
    void use();
private:
    GLuint vertex_shader_ID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader_ID = glCreateShader(GL_FRAGMENT_SHADER);
    GLuint program_ID = glCreateProgram(); 
};

#endif // SHADER_HPP