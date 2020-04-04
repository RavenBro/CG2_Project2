#version 330 core
in vec2 vTexCoords;
in vec3 vNormal;
in vec3 vFragPos;

out vec4 color;

const vec3 lightColor = vec3(1, 1, 1);

uniform vec3 lightPos = vec3(0, 0, 5);
uniform sampler2D diffuse_tex;

void main()
{

  float amb_str = 0.2f;
  vec4 ambient = vec4(amb_str * lightColor, 1.f);

  vec3 lightDir = normalize(lightPos - vFragPos);
  vec3 diffuse = max(dot(vNormal, lightDir), 0.f) * lightColor;

  vec3 viewDir = normalize(-vFragPos);
  vec3 reflectDir = reflect(-lightDir, vNormal);

  vec3 spec = pow(max(dot(viewDir, reflectDir), 0.f), 128) * lightColor;

  color = (ambient + vec4(diffuse, 1.f) + vec4(spec, 1.f)) * texture(diffuse_tex, vTexCoords);
}