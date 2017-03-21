#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 uvs;

noperspective out vec2 UVs;

void main(){
    UVs = uvs;
    gl_Position = position;
}