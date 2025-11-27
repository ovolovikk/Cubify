#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D u_Atlas;

void main()
{
    FragColor = texture(u_Atlas, TexCoord);
}