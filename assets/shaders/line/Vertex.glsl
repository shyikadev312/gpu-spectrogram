#version 330 core

in vec2 pos;

uniform mat3 ctm;

void main()
{
    vec3 clipPosition = ctm * vec3(pos, 1.0);
    gl_Position = vec4(clipPosition, 1.0);
}
