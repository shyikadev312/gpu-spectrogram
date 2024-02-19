in vec2 heatmapPosition;

out vec4 finalFragmentColor;

uniform uint columnSize;
uniform float dbfsToMagnitudeCellCoeff;

layout(std430, binding = 1) readonly buffer RtsaBuffer
{
    vec2 values[];
} rtsaBuffer;

void main()
{
    uint columnIndex = uint(heatmapPosition.x);
    uint rowIndex = uint(-heatmapPosition.y * dbfsToMagnitudeCellCoeff);
    uint bufferElementIndex = columnIndex * columnSize + rowIndex;

    //float value = rtsaBuffer.values[bufferElementIndex];

    float valueIntensity = rtsaBuffer.values[bufferElementIndex].x;
    valueIntensity = clamp(valueIntensity, 0.0, 1.0);

    float valueAge = rtsaBuffer.values[bufferElementIndex].y;
    valueAge = clamp(valueAge, 0.0, 1.0);

    vec4 valueIntensityColor = getScaleColor(valueIntensity);

    //vec4 color1 = vec4(0.0, 0.0, 1.0, 1.0);
    //vec4 color2 = vec4(1.0, 0.0, 0.0, 1.0);
    //vec4 color =  mix(color1, color2, valueIntensity);
    
    vec4 color = valueIntensityColor * (1.0 - valueAge);

    finalFragmentColor = color;
}
