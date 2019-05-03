#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform mat4 MVP;

void main()
{
    vec4 worldPos = MVP * vec4(aPos, 1.0);
    FragPos = worldPos.xyz; 
    TexCoords = aTexCoords;
    Normal = aNormal;

    gl_Position = worldPos;
}