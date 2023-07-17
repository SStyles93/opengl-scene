#version 330 core
precision highp float;
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gBaseColor;
layout (location = 2) out vec3 gNormal;
layout (location = 3) out vec3 gARM;
layout (location = 4) out vec3 gSSAO;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_arm1;

void main()
{    
    ///World Pos
    gPosition = WorldPos;
    //Base color
    gBaseColor.rgb = texture(texture_diffuse1, TexCoords).rgb;

    //Normal
    vec3 tangentNormal = texture(texture_normal1, TexCoords).rgb * 2.0 - 1.0;
    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N = normalize(Normal);
    vec3 T = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    gNormal.rgb = TBN * tangentNormal;

   gARM.rgb = texture(texture_arm1, TexCoords).rgb;

    //SSAO ViewPos
    gSSAO = FragPos;
}