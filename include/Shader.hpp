#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>

using std::string;

class Shader
{
public:
    Shader();
    ~Shader();

    bool load(string vertex_shader_path, string fragment_shader_path);

    void use();

    void setMat4(const string& name, const glm::mat4& mat) const;
    void setVec3(const string& name, const glm::vec3& vec) const;
    void setFloat(const string& name, const float value) const;
    void setInt(const string& name, const int value) const;

private:
    GLuint vertex_shader_ID = 0;
    GLuint fragment_shader_ID = 0;
    GLuint program_ID = 0; 
};

#endif // SHADER_HPP