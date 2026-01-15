#version 430 core

const float FOG_DENSITY = 0.005;
const float FOG_POWER = 1.5;

struct Quad
{
    uint packed_position;
    uint packed_data;
};

layout (std430, binding = 0) buffer QuadBuffer
{
    Quad quads[];
};

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 TexCoord;
flat out vec3 Normal;
out float Visibility;

void main()
{
    Quad q = quads[gl_InstanceID];
    
    // unpack data
    float x = float(q.packed_position & 1023u);
    float y = float((q.packed_position >> 10) & 1023u);
    float z = float((q.packed_position >> 20) & 1023u);
    float layer = float(q.packed_data & 1023u);
    uint normal_index = (q.packed_data >> 10) & 7u;

    int perpendicular_axis = int(normal_index) / 2;
    bool is_positive_face = (normal_index % 2) == 0;
    
    int index = gl_VertexID % 6;
    float u_local = 0.0;
    float v_local = 0.0;
    if(is_positive_face)
    {
    /* */if (index == 0) { u_local = 0.0; v_local = 0.0; } // bottom left(1)
    else if (index == 1) { u_local = 1.0; v_local = 0.0; } // bottom right(1)
    else if (index == 2) { u_local = 1.0; v_local = 1.0; } // top right(1)
    else if (index == 3) { u_local = 1.0; v_local = 1.0; } // top right(2)
    else if (index == 4) { u_local = 0.0; v_local = 1.0; } // top left(2)
    else if (index == 5) { u_local = 0.0; v_local = 0.0; } // bottom left(2)
    } else
        {
            /* */if (index == 0) { u_local = 0.0; v_local = 0.0; }
            else if (index == 1) { u_local = 1.0; v_local = 1.0; } // swap )
            else if (index == 2) { u_local = 1.0; v_local = 0.0; } // swap )
            else if (index == 3) { u_local = 1.0; v_local = 1.0; }
            else if (index == 4) { u_local = 0.0; v_local = 0.0; } // swap )
            else if (index == 5) { u_local = 0.0; v_local = 1.0; } // swap )
        }


    // calc position
    vec3 pos = vec3(x, y, z);
    int u_axis = (perpendicular_axis + 1) % 3;
    int v_axis = (perpendicular_axis + 2) % 3;

    pos[u_axis] += u_local; 
    pos[v_axis] += v_local; 

    vec4 position_relative = view * model * vec4(pos, 1.0);
    gl_Position = projection * position_relative;

    vec3 computed_normal = vec3(0.0);
    
    computed_normal[perpendicular_axis] = is_positive_face ? 1.0 : -1.0;
    
    Normal = computed_normal;    
    float distance = length(position_relative.xyz);
    Visibility = exp(-pow((distance * FOG_DENSITY), FOG_POWER));
    Visibility = clamp(Visibility, 0.0, 1.0);

    bool swapUV = (perpendicular_axis == 0 || perpendicular_axis == 1);
    if (swapUV) TexCoord = vec3(v_local, u_local, layer);
        else TexCoord = vec3(u_local, v_local, layer);
}
