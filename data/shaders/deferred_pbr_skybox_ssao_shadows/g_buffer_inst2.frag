#version 330 core
precision highp float;
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gARM;
layout (location = 4) out vec3 gShadow;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_ao;
uniform sampler2D texture_roughness;
uniform sampler2D texture_metallic;

void main()
{    
   // store the fragment position vector in the first gbuffer texture
    gPosition = WorldPos;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    // and the diffuse per-fragment color
     gAlbedo.rgb = texture(texture_diffuse, TexCoords).rgb;
    // Ao Roughness Metallic
    gARM.r = texture(texture_ao, TexCoords).r; 
    gARM.g = texture(texture_roughness, TexCoords).r; 
    gARM.b = texture(texture_metallic, TexCoords).r; 
    
}