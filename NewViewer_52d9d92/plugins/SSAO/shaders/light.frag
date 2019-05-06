#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssao;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float radius;
uniform vec3 viewPos;

float linear = 0.09;
float quadratic = 0.032;
float constant = 1;
void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float AmbientOcclusion = texture(ssao, TexCoords).r;

    vec3 ambient = vec3(0.3 * Diffuse * AmbientOcclusion);
    vec3 lighting = ambient; 
    vec3 viewDir = normalize(- FragPos);
    // diffuse
    vec3 lightDir = normalize(lightPosition - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lightColor;
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
    vec3 specular = lightColor * spec;
    // attenuation
    float distance = length(lightPosition - FragPos);
    float attenuation = 1.0 / (1.0 + linear * distance + quadratic * distance * distance);
    diffuse *= attenuation;
    specular *= attenuation;
    lighting += diffuse + specular;
    FragColor = vec4(lighting, 1.0);
}


