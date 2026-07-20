#version 410 core
layout (location = 0) in vec3 aPos;

uniform mat4 uMVP;

out vec3 vWorldPos;
out vec3 vNormal;

void main() {
    vWorldPos = aPos;
    vNormal = normalize(aPos + vec3(0.0, 0.35, 0.0));
    gl_Position = uMVP * vec4(aPos, 1.0);
}
