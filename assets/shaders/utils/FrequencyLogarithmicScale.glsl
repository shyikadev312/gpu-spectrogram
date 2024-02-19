#ifndef GUARD_LOG10
#define GUARD_LOG10
float log10(float x)
{
    // log10(x) = log(x) / log(10) = (1 / log(10)) * log(x)
    return (1 / log(10)) * log(x);
}
#endif

uint getElementIndexY(float y, float elementHeight, uint elementCountInColumn)
{
    float minY = log10(0 + 1);
    float maxHertz = elementHeight * elementCountInColumn;
    float maxY = log10(maxHertz + 1);
    float coeff = maxHertz / maxY;
    float mappedY = y / coeff;
    float frequencyValue = pow(10.0, mappedY) - 1.0;
    float elementIndex = frequencyValue / elementHeight;
    uint valueIndexY = uint(elementIndex);
    return valueIndexY;
}
