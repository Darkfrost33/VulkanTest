#version 450

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform pusBlock{
	float offset;
}constant;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor*texture(texSampler,fragTexCoord).rgb + vec3(0.0f, 1.0f * constant.offset, 0.0f),1.0);
	//outColor = vec4(fragColor,1.0);
}