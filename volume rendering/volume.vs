#version 330 core

layout(location = 0) in vec3 aPos;           // ³»ÂI¦ì¸m
layout(location = 1) in vec3 aNormal;      // 3D texture ®y¼Ð
//layout(location = 2) in vec3 aTexCoord;

out vec3 texCoord;
out vec3 worldPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 world = model * vec4(aPos, 1.0);
    texCoord = aPos + vec3(0.5,0.5,0.5);
    worldPos = world.xyz;


    gl_Position = projection * view * model * vec4(aPos,1.0);
}

	