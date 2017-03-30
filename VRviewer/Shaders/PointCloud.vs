#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 inColor;
out vec3 inNormal;

void main(){
    mat4 mv = view * model;
    inNormal = transpose(inverse(mv)) * vec4(normal, 1.0);
    gl_Position = proj * mv * vec4(pos, 1.0);
    inColor = color;
    
}