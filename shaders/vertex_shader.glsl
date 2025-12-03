#version 430 core

struct Quad
{
    float x, y, z; // bottom left vertex
    float w, h;
    float layer;
    int perpendicular_axis; // 0-x, 1-y, 2-z
    int back_face;
};

layout (std430, binding = 0) buffer QuadBuffer
{
    Quad quads[];
};

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 TexCoord;

void main()
{
    Quad q = quads[gl_InstanceID];
    int index = gl_VertexID % 6;
    float u_local = 0.0;
    float v_local = 0.0;

    /* */if (index == 0) { u_local = 0.0; v_local = 0.0; } // bottom left(1)
    else if (index == 1) { u_local = 1.0; v_local = 0.0; } // bottom right(1)
    else if (index == 2) { u_local = 1.0; v_local = 1.0; } // top right(1)
    else if (index == 3) { u_local = 1.0; v_local = 1.0; } // top right(2)
    else if (index == 4) { u_local = 0.0; v_local = 1.0; } // top left(2)
    else if (index == 5) { u_local = 0.0; v_local = 0.0; } // bottom left(2)

    // calc position
    vec3 pos = vec3(q.x, q.y, q.z);
    int u_axis = (q.perpendicular_axis + 1) % 3;
    int v_axis = (q.perpendicular_axis + 2) % 3;

    pos[u_axis] += u_local * q.w;
    pos[v_axis] += v_local * q.h;

    gl_Position = projection * view * model * vec4(pos, 1.0);
    bool swapUV = (q.perpendicular_axis == 0 || q.perpendicular_axis == 1);
    if (swapUV) TexCoord = vec3(v_local * q.h, u_local * q.w, q.layer);
        else TexCoord = vec3(u_local * q.w, v_local * q.h, q.layer);
}
