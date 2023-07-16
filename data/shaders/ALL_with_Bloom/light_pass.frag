#version 330 core
precision highp float;
out vec4 FragColor;

in vec2 TexCoords;

//color buffers 0, 1, 2, 3
uniform sampler2D gPosition;
uniform sampler2D gBaseColor;
uniform sampler2D gNormal;
uniform sampler2D gARM;

//SSAO 4
uniform sampler2D gSSAO;

//IBL 5, 6, 7
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

//ShadowMap 8
uniform sampler2D shadowMap;

uniform mat4 lightSpaceMatrix;
uniform vec3 directionnalLightDir;
uniform vec3 directionnalLightColor;

struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};
uniform int NR_LIGHTS;
uniform Light lights[128];
uniform vec3 camPos;

const float PI = 3.14159265359;
//------------------------------------------------
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 lightDir = normalize(-directionnalLightDir);
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.002);
    //float bias = Bias;
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0; 
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

//----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   
// ----------------------------------------------------------------------------
void main()
{		
    vec3 WorldPos = texture(gPosition,TexCoords).rgb;
    vec3 N = texture(gNormal, TexCoords).rgb;
    vec3 V = normalize(camPos - WorldPos);
    vec3 R = reflect(-V, N);

    vec3 baseColor = pow(texture(gBaseColor, TexCoords).rgb, vec3(2.2));

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, baseColor, texture(gARM,TexCoords).b);

    //Shadow
    vec4 FragPosLightSpace = lightSpaceMatrix * vec4(texture(gPosition, TexCoords).rgb, 1.0f);
    float shadow = ShadowCalculation(FragPosLightSpace);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    //IRRADIANCE for point light
    for(int i = 0; i < NR_LIGHTS; ++i) 
    {
        // calculate per-light radiance
        vec3 L = normalize(lights[i].Position - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(lights[i].Position -  WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lights[i].Color * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, texture(gARM, TexCoords).g);   
        float G   = GeometrySmith(N, V, L, texture(gARM, TexCoords).g);
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
           
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - texture(gARM, TexCoords).g;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * baseColor / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }  
    //IRRADIANCE for DirLight
    {
        // calculate per-light radiance
        vec3 L = normalize(-directionnalLightDir);
        vec3 H = normalize(V + L);
        //float distance = length(directionnalLightDir - WorldPos);
        //float attenuation = 1.0 / (distance * distance);
        //TODO: UNIFORM FOR DIRLIGHT COLOR
        vec3 radiance = vec3(directionnalLightColor) /* attenuation*/;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, texture(gARM, TexCoords).g);   
        float G   = GeometrySmith(N, V, L, texture(gARM, TexCoords).g);
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
           
        vec3 numerator    = NDF * G * F; 
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
        vec3 specular = numerator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - texture(gARM, TexCoords).g;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * (baseColor / PI + specular)) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again

    }

    //IBL ambient lighting
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, texture(gARM, TexCoords).g);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - texture(gARM, TexCoords).b;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * baseColor;

     const float MAX_REFLECTION_LOD = 4.0;

     vec3 prefilteredColor = textureLod(prefilterMap, R, texture(gARM, TexCoords).g * MAX_REFLECTION_LOD).rgb;
     vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), texture(gARM, TexCoords).g)).rg;
     vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    
    //IBL ambient
    vec3 ambient = (kD * diffuse + specular) * texture(gARM, TexCoords).r;
    //Add SSAO
    ambient *= texture(gSSAO, TexCoords).r;  
    
    ambient *= (1.0 - shadow);

    vec3 color = ambient + Lo;

//    // HDR tonemapping
//    color = color / (color + vec3(1.0));
//    // gamma correct
//    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}