#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float radius;
uniform vec3 viewPos;

float linear = 0.7;
float quadratic = 1.8;
float constant = 1;
void main()
{             
    // retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);
    // calculate distance between light source and current fragment
    float distance = length(lightPosition - FragPos);
    if(distance < radius)
    {
        // diffuse
        vec3 lightDir = normalize(lightPosition - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lightColor;
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = lightColor * spec * Specular;
        // attenuation
        float attenuation = 1.0 / (1.0 + linear * distance + quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
    }
    FragColor = vec4(Normal, 1.0);
}

