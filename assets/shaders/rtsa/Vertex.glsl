#version 330 core

in vec3 localPosition;

out vec2 heatmapPosition;

uniform mat3 localToClip;
uniform mat3 localToWorld;

void main()
{
    vec3 worldPosition = localToWorld * vec3(localPosition.xy, 1.0);
    heatmapPosition = worldPosition.xy;

    vec3 clipSpacePosition = localToClip * vec3(localPosition.xy, 1.0);
    gl_Position = vec4(clipSpacePosition.xy, 0.0, 1.0);
}
