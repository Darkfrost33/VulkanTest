#version 450

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(push_constant) uniform pusBlock{
	uint debugLine;
}constant;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	if(constant.debugLine == 0){
		outColor = vec4(fragColor*texture(texSampler,fragTexCoord).rgb ,1.0);
	}else if(constant.debugLine == 1)
	{
		outColor = vec4(0.0,1.0,0.0,1.0);
	}else if(constant.debugLine == 2)
	{
		outColor = vec4(1.0,0.0,0.0,1.0);
	}
}