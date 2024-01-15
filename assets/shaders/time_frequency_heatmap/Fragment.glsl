#version 430 core
  
in vec2 heatmapPosition;

out vec4 finalFragmentColor;

uniform vec2 lowerLeft; // TODO move to vertex?
uniform float columnWidthUnits;
uniform float valueHeightUnits;
uniform uint columnHeightValues;
uniform float minValue;
uniform float maxValue;

layout(std430, binding = 1) readonly buffer HeatmapBuffer
{
    float values[];
} heatmapBuffer;

void main()
{
    vec2 bufferRelativePos = heatmapPosition - lowerLeft;
    uint valueIndexX = uint(bufferRelativePos.x / columnWidthUnits);
    uint valueIndexY = uint(bufferRelativePos.y / valueHeightUnits);
    uint valueIndex = valueIndexX * columnHeightValues + valueIndexY;
    float value = heatmapBuffer.values[valueIndex];
    float valueRatio = (value - minValue) / (maxValue - minValue);
    valueRatio = clamp(valueRatio, 0.0, 1.0);

    // grayscale:
    // vec4 startColor = vec4(0.0, 0.0, 0.0, 1.0); // TODO gradient texture
    // vec4 endColor = vec4(1.0, 1.0, 1.0, 1.0);
    // finalFragmentColor = mix(startColor, endColor, valueRatio);
    // return;

    // colored scale

#define HEATMAP_COLOR_COUNT 6

    vec4 HeatmapGradientColors[HEATMAP_COLOR_COUNT] = vec4 [HEATMAP_COLOR_COUNT] (
        vec4(0.0, 0.0, 0.0, 1.0), // black
        vec4(0.0, 0.0, 1.0, 1.0), // blue
        vec4(0.0, 1.0, 1.0, 1.0), // cyan
        vec4(0.0, 1.0, 0.0, 1.0), // green
        vec4(1.0, 1.0, 0.0, 1.0), // yellow
        vec4(1.0, 0.0, 0.0, 1.0) // red
    );

    float colorStep = 1.0 / (HEATMAP_COLOR_COUNT - 1);
    int colorIndex = int(valueRatio / colorStep);
    colorIndex = clamp(colorIndex, 0, HEATMAP_COLOR_COUNT - 2);
    float colorProgress = (valueRatio - (colorIndex * colorStep)) / colorStep;
    vec4 color1 = HeatmapGradientColors[colorIndex];
    vec4 color2 = HeatmapGradientColors[colorIndex + 1];
    finalFragmentColor = mix(color1, color2, colorProgress);
}
