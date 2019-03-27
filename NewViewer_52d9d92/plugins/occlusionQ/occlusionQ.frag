#version 330 core

out vec4 fragColor;
in vec4 col;
uniform int n;

void main() {
  fragColor=col;
};