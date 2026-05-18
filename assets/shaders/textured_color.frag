
#version 410 core
in vec2 vUV;
uniform vec3 uColor;
uniform sampler2D uAlbedoMap;
uniform int uUseTexture;
out vec4 FragColor;
void main() {
    vec3 albedo = uColor;
    if (uUseTexture != 0) {
        albedo *= texture(uAlbedoMap, vUV).rgb;
    }
    FragColor = vec4(albedo, 1.0);
}
