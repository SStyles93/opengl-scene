#version 330 core
precision highp float;
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
//aModel covers location 3 to 6 (4 times mat4)
layout (location = 3) in mat4 aModel;
//aNormalMatrix covers location 7 to 10 (4 times mat4)
layout (location = 7) in mat4 aNormalMatrix;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;

    vec4 worldPos = aModel * vec4(aPos, 1.0);
    WorldPos = worldPos.xyz; 
    
    Normal = mat3(aNormalMatrix) * aNormal;

    gl_Position = projection * view *  worldPos;
}