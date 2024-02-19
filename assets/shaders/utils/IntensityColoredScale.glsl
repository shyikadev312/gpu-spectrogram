#define SCALE_COLOR_COUNT 6

vec4 getScaleColor(float ratio)
{
    vec4 GradientColors[SCALE_COLOR_COUNT] = vec4 [SCALE_COLOR_COUNT] (
        vec4(0.0, 0.0, 0.0, 1.0), // black
        vec4(0.0, 0.0, 1.0, 1.0), // blue
        vec4(0.0, 1.0, 1.0, 1.0), // cyan
        vec4(0.0, 1.0, 0.0, 1.0), // green
        vec4(1.0, 1.0, 0.0, 1.0), // yellow
        vec4(1.0, 0.0, 0.0, 1.0) // red
    );

    float colorStep = 1.0 / (SCALE_COLOR_COUNT - 1);
    int colorIndex = int(ratio / colorStep);
    colorIndex = clamp(colorIndex, 0, SCALE_COLOR_COUNT - 2);
    float colorProgress = (ratio - (colorIndex * colorStep)) / colorStep;
    vec4 colorFrom = GradientColors[colorIndex];
    vec4 colorTo = GradientColors[colorIndex + 1];
    vec4 color = mix(colorFrom, colorTo, colorProgress);
    return color;
}
