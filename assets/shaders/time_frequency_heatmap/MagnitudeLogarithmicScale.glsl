#ifndef GUARD_LOG10
#define GUARD_LOG10
float log10(float x)
{
    // log10(x) = log(x) / log(10) = (1 / log(10)) * log(x)
    return (1 / log(10)) * log(x);
}
#endif

float toLogScale(float magnitudeValue, float magnitudeMaxValue)
{
    return 20.0 * log10(magnitudeValue / magnitudeMaxValue);
}

float getMagnitudeValueRatio(float magnitudeValue, float magnitudeMin, float magnitudeMax)
{
    float logValue = toLogScale(magnitudeValue, magnitudeMax);
    float minLogValue = -96; // TODO
    float maxLogValue = 0;
    float valueRatio = (logValue - minLogValue) / (maxLogValue - minLogValue);
    return valueRatio;
}
