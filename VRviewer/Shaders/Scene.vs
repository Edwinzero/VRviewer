#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 uvs;
layout(location = 2) in vec3 normals;

uniform mat4 matrix;

out vec2 UVs;

void main(){
    UVs = uvs;
    gl_Position = matrix * position;
}