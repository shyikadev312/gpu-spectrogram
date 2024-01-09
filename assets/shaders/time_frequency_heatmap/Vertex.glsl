#version 330 core

in vec3 localPosition;

out vec2 heatmapPosition;

uniform mat3 localToWorld;
uniform mat3 localToClip;

void main()
{
    vec3 worldPosition = localToWorld * vec3(localPosition.xy, 1.0);
    heatmapPosition = worldPosition.xy;

    vec3 clipPos = localToClip * vec3(localPosition.xy, 1.0);
    gl_Position = vec4(clipPos.xy, 0.0, 1.0);
}
