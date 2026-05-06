#version 330 core
out vec4 FragColor;

uniform vec3 color;

void main()
{
    // 輸出指定的顏色，alpha 設為 1.0
    FragColor = vec4(color, 1.0);
}