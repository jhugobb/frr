#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;
uniform sampler2D xOcc;
const int MAX_KERNEL_SIZE = 512;
uniform vec3 samples[MAX_KERNEL_SIZE];

uniform int kernelSize;
uniform float radius;
uniform float bias;

const vec2 noiseScale = vec2(1280.0/4.0, 720.0/4.0); 

uniform mat4 projection;

void main()
{
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for(int i = kernelSize/2; i < kernelSize; ++i)
    {
        vec3 sample = TBN * vec3(0, samples[i].y, samples[i].z); // from tangent to view-space
        sample = fragPos + sample * radius; 
        
        vec4 offset = vec4(sample, 1.0);
        offset = projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        float sampleDepth = texture(gPosition, offset.xy).z; // get depth value of kernel sample
        
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / (kernelSize/2));

    float xocc = texture(xOcc, TexCoords).r;
    FragColor = (occlusion+xocc)/2;
}

