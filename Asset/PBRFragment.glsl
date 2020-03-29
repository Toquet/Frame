#version 330 core

layout(location = 0) out vec4 frag_color;

in vec3 out_normal;
in vec3 out_world;
in vec2 out_texcoord;

uniform sampler2D Color;
uniform sampler2D Normal;
uniform sampler2D Metallic;
uniform sampler2D Roughness;
uniform sampler2D AmbientOcclusion;

uniform vec3 camera_position;
uniform vec3 light_position;
uniform vec3 light_color;

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(Normal, out_texcoord).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(out_world);
    vec3 Q2  = dFdy(out_world);
    vec2 st1 = dFdx(out_texcoord);
    vec2 st2 = dFdy(out_texcoord);

    vec3 N   = normalize(out_normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
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
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// ----------------------------------------------------------------------------
void main()
{		
    vec3 albedo     = pow(texture(Color, out_texcoord).rgb, vec3(2.2));
    float metallic  = texture(Metallic, out_texcoord).r;
    float roughness = texture(Roughness, out_texcoord).r;
    float ao        = texture(AmbientOcclusion, out_texcoord).r;
//    vec3 albedo = pow(vec3(0.5, 0.0, 0.0), vec3(2.2));
//    float metallic = 1.0;
//    float roughness = 0.0;
//    float ao = 1.0;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(camera_position - out_world);

    // calculate reflectance at normal incidence; if dia-electric (like plastic)
    // use F0 of 0.04 and if it's a metal, use the albedo color as F0 (metallic
    // workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    
    // calculate per-light radiance
    vec3 L = normalize(light_position - out_world);
    vec3 H = normalize(V + L);
    float distance = length(light_position - out_world);
    float attenuation = 1.0; // 1.0 / (distance * distance);
    vec3 radiance = light_color * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);      
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
           
    vec3 nominator = NDF * G * F; 
    // 0.001 to prevent divide by zero.
    float denominator = 
        4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular = nominator / denominator;
        
    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals 
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;	  

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);        

    // add to outgoing radiance Lo
    // note that we already multiplied the BRDF by the Fresnel (kS) so we
    // won't multiply by kS again
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;    
    
    // ambient lighting (note that the next IBL tutorial will replace 
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * albedo * ao;
    
    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    frag_color = vec4(color, 1.0);
}