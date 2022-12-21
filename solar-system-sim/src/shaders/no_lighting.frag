#version 330 core

in vec3 fragPos;
in vec3 n;
in vec2 tc;

uniform sampler2D sampler;

out vec4 color;

void main() {
	color = texture(sampler, tc);
}
