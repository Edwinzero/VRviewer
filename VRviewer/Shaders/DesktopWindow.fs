#version 330 core

noperspective in vec2 UVs;
out vec4 outColor;

uniform sampler2D tex;

void main(){
    outColor = texture(tex, UVs);
}