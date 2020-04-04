#version 330 core
in vec3 localPos;
out vec4 FragColor; 

uniform samplerCube envMap;
uniform vec3 upDir = vec3(0, 1, 0);
const float PI = 3.14159265359;
const float Epsilon = 0.00001; 

void main()
{
    vec3 normal = normalize(localPos);
    vec3 irradiance = vec3(0);
    vec3 up = vec3(1, 0, 0);
    vec3 right = cross(normal, up);
    //right = mix(cross(normal, vec3(1, 0, 0)), right, step(Epsilon, dot(right, right)));
    right = normalize(right);
    up = cross(normal, right);
    if(length(right) == 0 || length(up) == 0) {
        FragColor = vec4(1, 0, 0, 1);
        return;
    }
    const float sampleDeltaPhi = 0.025;
    const float sampleDeltaTheta = 0.1;
    float nrSamples = 0.0; 
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDeltaPhi) {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDeltaTheta) {
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 
            irradiance += clamp(textureLod(envMap, sampleVec, 0.f).rgb * cos(theta) * sin(theta), 0, 5);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    FragColor = vec4(irradiance, 1);
}