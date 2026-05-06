#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

struct Material{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
/*
struct Light{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
*/
struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHTS 2
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform Material material;
//uniform vec3 lightColor;

uniform bool switch0;
uniform bool switch1;

uniform vec3 viewPos;
uniform vec3 lightPos;
//uniform Light light;
uniform vec3 objectColor;
uniform vec3 lightColor;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main(){
	/*vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 result;
	float ambientStrength =0.1;
	vec3 ambient = ambientStrength * lightColor;
	result+=ambient;
	if(switch0) result+=CalcPointLight(pointLights[0],norm,FragPos,viewDir);
	if(switch1) result += CalcPointLight(pointLights[1],norm,FragPos,viewDir);
	FragColor = vec4(result,1.0);*/

        float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    //vec3 specular = specularStrength * spec * lightColor;  
      //vec3 specular = (0.0f,0,0);  
    vec3 result = (ambient + diffuse ) * objectColor;
    FragColor = vec4(result, 0.5);

}
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    //ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    //diffuse = light.diffuse;
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    //specular = light.specular * spec;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}