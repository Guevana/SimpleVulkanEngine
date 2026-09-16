#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUv;
layout(set = 1, binding = 0) uniform sampler2D baseColorTexture;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} pushData;

void main(){
    outColor = vec4(fragColor * texture(baseColorTexture, fragUv).rgb, 1.0);
}