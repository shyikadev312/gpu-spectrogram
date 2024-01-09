#version 330 core
  
in vec2 worldPosition;

out vec4 finalFragmentColor;

void main()
{
    vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
    vec4 black = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 red = vec4(1.0, 0.0, 0.0, 1.0);

    int x = int(worldPosition.x) + int(sign(worldPosition.x));
    int y = int(worldPosition.y) + int(sign(worldPosition.y));

    int diag = int(x * y > 0);

    vec4 color1 = mix(red, white, diag);
    vec4 color2 = mix(white, black, diag);

    int even = int((x % 2) == (y % 2));
    vec4 color = mix(color1, color2, even);
    finalFragmentColor = color;
}
