#version 330 core
precision highp float;
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gBaseColor;
layout (location = 3) out vec3 gNormalMap;
layout (location = 4) out vec3 gARM;
layout (location = 5) out vec3 gSSAO;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

uniform sampler2D baseColorMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

void main()
{    
    gPosition = WorldPos;
    gNormal = normalize(Normal);
    //Base color
    gBaseColor.rgb = texture(baseColorMap, TexCoords).rgb;
    //NormalMap
    gNormalMap.rgb = texture(normalMap, TexCoords).rgb;
    // Ambient Roughness Metallic
    gARM.r = texture(aoMap, TexCoords).r;
    gARM.g = texture(roughnessMap, TexCoords).r;
    gARM.b = texture(metallicMap, TexCoords).r;
}