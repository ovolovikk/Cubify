#version 430 core

in vec3 TexCoord;
flat in vec3 Normal;
in float Visibility;

out vec4 FragColor;

uniform sampler2DArray u_Textures;

const vec3 SUN_DIRECTION = normalize(vec3(0.5, 0.8, 0.4)); 
const vec3 SKY_COLOR = vec3(0.28, 0.66, 1.0);
const float AMBIENT_INTENSITY = 0.3;
const float SUN_INTENSITY = 0.6;

void main()
{
    vec4 texColor = texture(u_Textures, TexCoord);

    // Wrap Lightning (light all sides of a block)
    float NdotL = dot(Normal, SUN_DIRECTION);
    float wrapDiffuse = (NdotL + 1.0) * 0.5; // Remap from [-1,1] to [0,1]
    
    // Lightning based on sides
    float faceBrightness = 1.0;
    if (abs(Normal.y) > 0.5) {
        faceBrightness = Normal.y > 0.0 ? 1.0 : 0.5; // Top 1.0 , Bottom 0.5
    } else if (abs(Normal.x) > 0.5) {
        faceBrightness = 0.8; // X 0.8
    } else {
        faceBrightness = 0.7; // Z 0.7
    }
    
    float lighting = wrapDiffuse * SUN_INTENSITY * faceBrightness + AMBIENT_INTENSITY;
    vec3 litColor = lighting * texColor.rgb;
    vec3 finalColor = mix(SKY_COLOR, litColor, Visibility);

    FragColor = vec4(finalColor, texColor.a);
}