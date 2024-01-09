#version 430 core
  
in vec2 heatmapPosition;

out vec4 finalFragmentColor;

uniform vec2 lowerLeft; // TODO move to vertex?
uniform float columnWidthUnits;
uniform float valueHeightUnits;
uniform uint columnHeightValues;

layout(std430, binding = 1) readonly buffer HeatmapBuffer
{
    float values[];
} heatmapBuffer;

void main()
{
    vec4 startColor = vec4(0.0, 0.0, 0.0, 1.0); // TODO gradient texture
    vec4 endColor = vec4(1.0, 1.0, 1.0, 1.0);

    float min = 0.0; // TODO set
    float max = 20.0;

    vec2 bufferRelativePos = heatmapPosition - lowerLeft;
    uint valueIndexX = uint(bufferRelativePos.x / columnWidthUnits);
    uint valueIndexY = uint(bufferRelativePos.y / valueHeightUnits);
    uint valueIndex = valueIndexX * columnHeightValues + valueIndexY;
    float value = heatmapBuffer.values[valueIndex];
    float valueRatio = (value - min) / (max - min);
    finalFragmentColor = mix(startColor, endColor, valueRatio);
}
