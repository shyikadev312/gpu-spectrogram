#version 330 core

in vec2 ndcPosition;

out vec2 worldPosition;

uniform mat3 mvpInv;

void main()
{
    vec3 pos = mvpInv * vec3(ndcPosition, 1.0);
    worldPosition = pos.xy;
    gl_Position = vec4(ndcPosition, 0.0, 1.0);
}
