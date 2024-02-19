vec4 getScaleColor(float ratio)
{
    vec4 startColor = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 endColor = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 color = mix(startColor, endColor, ratio);
    return color;
}
