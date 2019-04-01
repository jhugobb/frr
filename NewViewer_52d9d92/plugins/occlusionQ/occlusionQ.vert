#version 330 core

uniform mat4 modelViewProjectionMatrix;
uniform vec3 bboxMax;
uniform vec3 bboxMin;
uniform bool bbox;
uniform bool useVFC;
layout (location=0) in vec3 vertex;
layout (location=1) in vec3 normals;
layout (location=2) in vec3 color;
layout (location=4) in vec3 vertexBox;
out vec4 col;

void main() {
  if (bbox){
    mat4 scale=mat4(vec4(bboxMax.x-bboxMin.x, 0, 0, 0),
                    vec4(0, bboxMax.y-bboxMin.y, 0, 0),
                    vec4(0, 0, bboxMax.z-bboxMin.z, 0),
                    vec4(0, 0, 0, 1));
    vec4 BC=vec4((bboxMax+bboxMin)/2, 0);
    vec4 V=scale*vec4(vertexBox-vec3(0.5), 1);
    gl_Position=modelViewProjectionMatrix*(BC+V);
    col = vec4(color*normals, 1.0);
  } else {
    gl_Position = modelViewProjectionMatrix * vec4(vertex,1.0);
    col=vec4(color,1.0);
  }

};