#version 410 core
in vec3 vWorldPos;
in vec3 vNormal;

uniform vec3 uColor;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform float uLightIntensity;
uniform vec3 uFogColor;
uniform float uFogDensity;
uniform vec3 uCameraPos;

out vec4 FragColor;

void main() {
    vec3 n = normalize(vNormal);
    float ndl = max(dot(n, normalize(uLightDir)), 0.0);
    // Toon bands for cartoon readability
    float band = ndl > 0.66 ? 1.0 : (ndl > 0.33 ? 0.65 : 0.35);
    vec3 lit = uColor * (0.25 + band * uLightIntensity) * uLightColor;

    float dist = length(uCameraPos - vWorldPos);
    float fog = 1.0 - exp(-uFogDensity * dist);
    vec3 color = mix(lit, uFogColor, clamp(fog, 0.0, 0.85));
    FragColor = vec4(color, 1.0);
}
