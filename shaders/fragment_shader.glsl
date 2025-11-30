#version 330 core

in vec3 TexCoord;

out vec4 FragColor;

uniform sampler2DArray u_Textures;

void main()
{
    FragColor = texture(u_Textures, TexCoord);
}