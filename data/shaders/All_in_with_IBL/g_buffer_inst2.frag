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

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_metallic1;
uniform sampler2D texture_roughness1;
uniform sampler2D texture_ao1;

void main()
{    
    gPosition = WorldPos;
    gNormal = normalize(Normal);
    //Base color
    gBaseColor.rgb = texture(texture_diffuse1, TexCoords).rgb;
    //NormalMap
    gNormalMap.rgb = texture(texture_normal1, TexCoords).rgb;
    // Ambient Roughness Metallic
    gARM.r = texture(texture_ao1, TexCoords).r;
    gARM.g = texture(texture_roughness1, TexCoords).r;
    gARM.b = texture(texture_metallic1, TexCoords).r;
}