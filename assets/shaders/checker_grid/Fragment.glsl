#version 330 core
  
in vec2 worldPosition;

out vec4 finalFragmentColor;

uniform vec2 cellSizeScale;

void main()
{
    vec4 gray = vec4(0.8);
    vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 black = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 red = vec4(1.0, 0.0, 0.0, 1.0);

    vec2 worldPositionScaled = worldPosition / cellSizeScale;

    int x = int(worldPositionScaled.x);
    int y = int(worldPositionScaled.y);

    int diag = int(worldPositionScaled.x * worldPositionScaled.y > 0);

    vec4 color1 = mix(red, gray , diag);
    vec4 color2 = mix(gray , black, diag);

    int even = int((x % 2) == (y % 2));
    vec4 color = mix(color1, color2, even);
    finalFragmentColor = color;
}
