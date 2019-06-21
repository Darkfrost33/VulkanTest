#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject{
	mat4 view;
	mat4 proj;
} ubo;

layout(set = 0, binding = 1) uniform UboInstance{
	mat4 model;
}uboInstance;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 4) in vec3 inInstancePos;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * uboInstance.model * vec4(inInstancePos+inPosition, 1.0);
	//gl_Position = uboInstance.model * vec4(inInstancePos+inPosition, 1.0);
	fragColor = vec3(1.0,1.0,1.0);
	fragTexCoord = inTexCoord;
}