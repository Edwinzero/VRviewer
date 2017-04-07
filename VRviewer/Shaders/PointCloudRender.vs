#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

uniform mat4 model_to_world;
uniform mat4 world_to_vive;

out vec3 inColor;
out vec3 inNormal;

void main(){
    vec4 world = model_to_world * vec4(pos, 1.0);
    gl_Position = world_to_vive * world;
    inColor = color;
    gl_PointSize = 3.0;
}