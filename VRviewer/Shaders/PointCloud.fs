#version 330 core

in vec3 inColor;
in vec3 inNormal;

out vec4 outColor;

void main(){
    outColor = vec4(inColor, 1.0);
}