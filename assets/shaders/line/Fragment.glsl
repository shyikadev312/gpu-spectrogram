#version 330 core

out vec4 finalFragmentColor;

uniform vec4 lineColor;

void main()
{
    finalFragmentColor = lineColor;
}
