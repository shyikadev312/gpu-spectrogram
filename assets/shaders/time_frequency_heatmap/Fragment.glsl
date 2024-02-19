in vec2 heatmapPosition;

out vec4 finalFragmentColor;

uniform vec2 lowerLeft; // TODO move to vertex?
uniform float columnWidthUnits;
uniform float valueHeightUnits; // TODO elementHeight
uniform uint columnHeightValues; // TODO elementCountInColumn
uniform float minValue;
uniform float maxValue;

layout(std430, binding = 1) readonly buffer HeatmapBuffer
{
    float values[];
} heatmapBuffer;

void main()
{
    vec2 bufferRelativePos = heatmapPosition - lowerLeft;
    uint valueIndexY = getElementIndexY(bufferRelativePos.y, valueHeightUnits, columnHeightValues);
    uint valueIndexX = uint(bufferRelativePos.x / columnWidthUnits);
    uint valueIndex = valueIndexX * columnHeightValues + valueIndexY;
    float value = heatmapBuffer.values[valueIndex];

    float magnitudeValueRatio = getMagnitudeValueRatio(value, minValue, maxValue);
    magnitudeValueRatio = clamp(magnitudeValueRatio, 0.0, 1.0);
    
    finalFragmentColor = getScaleColor(magnitudeValueRatio);
}
