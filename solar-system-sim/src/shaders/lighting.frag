#version 330 core

in vec3 fragPos;
in vec3 n;
in vec2 tc;

uniform sampler2D sampler;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float diffuse;
uniform float specular;
uniform float shininess;

out vec4 color;

void main() {
	vec3 normalVector = normalize(n);
	vec3 lightVector = normalize(lightPos - fragPos);
	vec3 viewVector = normalize(viewPos - fragPos);

	float ambientLight = 0.05;
	float diffuseReflection = diffuse * max(dot(normalVector, lightVector), 0.0);
	float specularReflection = specular * pow(max(dot(reflect(-lightVector, normalVector), viewVector), 0.0), shininess);

	color = (diffuseReflection + specularReflection + ambientLight) * texture(sampler, tc);
}
