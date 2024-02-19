float getMagnitudeValueRatio(float magnitudeValue, float magnitudeMin, float magnitudeMax)
{
	float valueRatio = (magnitudeValue - magnitudeMin) / (magnitudeMax - magnitudeMin);
	return valueRatio;
}
