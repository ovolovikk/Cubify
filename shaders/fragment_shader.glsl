#version 430 core

in vec3 TexCoord;
flat in vec3 Normal;
in float Visibility;

out vec4 FragColor;

uniform sampler2DArray u_Textures;

const vec3 SUN_DIRECTION = normalize(vec3(0.5, 0.8, 0.4)); 
const vec3 SKY_COLOR = vec3(0.28, 0.66, 1.0);
const float AMBIENT_INTENSITY = 0.2;
const float SUN_INTENSITY = 0.8;

void main()
{
    vec4 texColor = texture(u_Textures, TexCoord);

    float diff = max(dot(Normal, SUN_DIRECTION), 0.0);
    vec3 lightning = (diff * SUN_INTENSITY + AMBIENT_INTENSITY) * texColor.rgb;
    vec3 finalColor = mix(SKY_COLOR, lightning, Visibility);

    FragColor = vec4(finalColor, texColor.a);
}