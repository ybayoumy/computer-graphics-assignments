#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec3 normal;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec3 fragPos;
out vec3 n;
out vec2 tc;

void main() {
	tc = texCoord;

	vec4 temp = M * vec4(pos, 1.0);
	fragPos = temp.xyz;

	temp = inverse(transpose(M)) * vec4(normal, 0.0);
	n = temp.xyz;

	gl_Position = P * V * M * vec4(pos, 1.0);
}
