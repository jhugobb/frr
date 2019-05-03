#version 330 core

uniform mat4 modelViewProjectionMatrix;
uniform vec3 bboxMax;
uniform vec3 bboxMin;
uniform mat4 scale;
layout (location=0) in vec3 vertex;
layout (location=1) in vec3 normals;
layout (location=2) in vec3 color;
out vec4 col;

void main() {

  gl_Position = scale * modelViewProjectionMatrix * vec4(vertex,1.0);
  col=vec4(color,1.0);

};