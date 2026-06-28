#include "Graphics/Shader.hpp"

#include "Core/Logging/Log.hpp"

Shader::Shader() : vertex_shader_ID(0), fragment_shader_ID(0), program_ID(0)
{
}

Shader::~Shader()
{
    glDetachShader(program_ID, vertex_shader_ID);
    glDetachShader(program_ID, fragment_shader_ID);

    glDeleteShader(vertex_shader_ID);
    glDeleteShader(fragment_shader_ID);
    glDeleteProgram(program_ID);
}

bool Shader::load(string vertex_shader_path, string fragment_shader_path)
{
    vertex_shader_ID = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader_ID = glCreateShader(GL_FRAGMENT_SHADER);
    program_ID = glCreateProgram();

    // vertex shader
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
        LOGE("[Shader] Vertex shader couldn't be opened: %s", vertex_shader_path.c_str());
        return false;
    }
    
    GLint result = GL_FALSE;
    int info_log_length;

    char const * vertex_shader_pointer = vertex_shader_code.c_str();
    glShaderSource(vertex_shader_ID, 1, &vertex_shader_pointer, NULL);
    glCompileShader(vertex_shader_ID);

    glGetShaderiv(vertex_shader_ID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertex_shader_ID, GL_INFO_LOG_LENGTH, &info_log_length);
    if(info_log_length > 0)
    {
        std::vector<char> vertex_shader_error_message(info_log_length + 1);
        glGetShaderInfoLog(vertex_shader_ID, info_log_length, NULL, &vertex_shader_error_message[0]);
        LOGE("%s", &vertex_shader_error_message[0]);
    }
    
    // fragment Shader
    string fragment_shader_code;
    std::ifstream fragment_shader_stream(fragment_shader_path, std::ios::in);
    if(fragment_shader_stream.is_open())
    {
        std::stringstream sstr;
        sstr << fragment_shader_stream.rdbuf();
        fragment_shader_code = sstr.str();
        fragment_shader_stream.close();
    }
    else 
    {
        LOGE("[Shader] Fragment shader couldn't be opened: %s", fragment_shader_path.c_str());
        return false;
    }

    char const * fragment_shader_pointer = fragment_shader_code.c_str();
    glShaderSource(fragment_shader_ID, 1, &fragment_shader_pointer, NULL);
    glCompileShader(fragment_shader_ID);

    glGetShaderiv(fragment_shader_ID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragment_shader_ID, GL_INFO_LOG_LENGTH, &info_log_length);
    if(info_log_length > 0)
    {
        std::vector<char> fragment_shader_error_message(info_log_length + 1);
        glGetShaderInfoLog(fragment_shader_ID, info_log_length, NULL, &fragment_shader_error_message[0]);
        LOGE("%s", &fragment_shader_error_message[0]);
    }

    // assigning shaders to program
    glAttachShader(program_ID, vertex_shader_ID);
    glAttachShader(program_ID, fragment_shader_ID);
    glLinkProgram(program_ID);

    glGetProgramiv(program_ID, GL_LINK_STATUS, &result);
    glGetProgramiv(program_ID, GL_INFO_LOG_LENGTH, &info_log_length);
    if (info_log_length > 0)
    {
        std::vector<char> program_error_message(info_log_length + 1);
        glGetProgramInfoLog(program_ID, info_log_length, NULL, &program_error_message[0]);
        LOGE("%s", &program_error_message[0]);
    }

    return true;
}

void Shader::use()
{
    glUseProgram(program_ID);
}

void Shader::setMat4(const string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(program_ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setVec3(const string& name, const glm::vec3& vec) const
{
    glUniform3fv(glGetUniformLocation(program_ID, name.c_str()), 1, &vec[0]);
}

void Shader::setFloat(const string& name, const float value) const
{
    glUniform1f(glGetUniformLocation(program_ID, name.c_str()), value);
}

void Shader::setInt(const string& name, const int value) const
{
    glUniform1i(glGetUniformLocation(program_ID, name.c_str()), value);
}
