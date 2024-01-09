#version 330 core

in vec2 pos;

uniform mat3 mvp;

void main()
{
    vec3 pos = mvp * vec3(pos, 1.0);
    gl_Position = vec4(pos, 1.0);
}
