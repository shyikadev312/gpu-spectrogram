uint getElementIndexY(float y, float elementHeight, uint elementCountInColumn)
{
    uint valueIndexY = uint(y / elementHeight);
    return valueIndexY;
}
