#version 450

layout(location =0) out vec4 outColor;

layout(push_constant) uniform Push {
    mat2 transform;
    vec2 offset;
    vec3 color;
} pushData;

void main(){
    outColor = vec4(pushData.color, 1.0);
}