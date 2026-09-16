#version 450 core

in vec3 colour;
in vec2 uv;
in vec3 pos;
in mat3 TBN;

out vec4 frag_colour;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 emissive;
    float shininess;
}; 

struct Light {
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PointLight {
  vec3 position;
  vec3 color;
  float intensity;
  float linear;
  float quadratic;
};

struct FlashLight {
  vec3 position;
  vec3 direction;
  vec3 color;
  float intensity;
  float cutoff;
  float linear;
  float quadratic;
};

/*
lightingShader.setFloat("light.constant",  1.0f);
lightingShader.setFloat("light.linear",    0.09f);
lightingShader.setFloat("light.quadratic", 0.032f);
*/

uniform sampler2D textureDiff;
uniform sampler2D textureBump;
uniform sampler2D textureEm;

uniform Material material;
uniform Light light;
uniform PointLight pointLight;
uniform FlashLight flashLight;

vec3 calculatePointLight(PointLight light, vec3 position, vec3 normal);
vec3 calculateFlashLight(FlashLight light, vec3 position, vec3 normal);
vec3 directionalLight(Light light, vec3 position, vec3 normal);

void main()
{
	vec2 textCord = vec2(uv.s, 1.0 - uv.t);
	vec4 diff_material = texture(textureDiff, textCord);
	if (diff_material.a < 0.1) 
        discard;

  vec4 bump_material = texture(textureBump, textCord);
  vec3 normal = bump_material.rgb;
  normal = normalize(TBN *(normal *2.0 - 1.0));

  vec3 finalColor = vec3(0.0, 0.0, 0.0);
  finalColor = finalColor + directionalLight(light, pos, normal);
  finalColor = finalColor + calculatePointLight(pointLight, pos, normal);
  finalColor =  finalColor + calculateFlashLight(flashLight, pos, normal);
  // emissive light
  vec4 em_material = texture(textureEm, textCord);
  vec3 emissive = vec3(em_material);
  finalColor = finalColor * diff_material.rgb + emissive;
	frag_colour = vec4(finalColor, diff_material.aaaa);
}

vec3 calclucateLightColor(vec3 light_color, vec3 light_dir, float intensity, vec3 position, vec3 normal) {
  // Light positions
  vec3 lightDir = light_dir;
  vec3 viewPos = vec3(0.0, 0.0, -10.0);

	// ambient light
  vec3 ambient = intensity * light_color * material.ambient;

  // diffuse color light
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = vec3(0.0, 0.0, 0.0);
  diffuse = diff * intensity * light_color  * material.diffuse;

  // specular light
  vec3 specular = vec3(0.0);
  if (material.shininess > 0.0) {
    bool blinn = true;
    float specularStrength = 0.6;
    vec3 viewDir = normalize(viewPos - position);
    float spec = 1.0;
    if (blinn) {
      vec3 halfwayDir = normalize(lightDir + viewDir);
      spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    } else {
      vec3 reflectDir = reflect(-lightDir, normal);  
      spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    specular = specularStrength * spec * material.specular;
  }

	// final calculation
  return ambient + diffuse + specular;
}

vec3 calculatePointLight(PointLight light, vec3 position, vec3 normal)
{ 
  vec3 light_dir = position - light.position;
  float distance = length(light_dir);
  light_dir = normalize(light_dir);
  vec3 color = calclucateLightColor(light.color, light_dir, light.intensity, position, normal);
  float attenuation = 1.0 + (light.linear * distance) + (light.quadratic * distance * distance);
  return color / attenuation;
}

vec3 calculateFlashLight(FlashLight light, vec3 position, vec3 normal)
{ 
  vec3 lightDir = normalize(light.position - pos);
  float theta = dot(lightDir, normalize(-light.direction));
  if (theta > light.cutoff) {
    PointLight pl;
    pl.position = light.position;
    pl.color = light.color;
    pl.linear = light.linear;
    pl.quadratic = light.quadratic;
    pl.intensity = light.intensity;
    return calculatePointLight(pl, position, normal);
  }
  return vec3(0.0, 0.0, 0.0);
}

vec3 directionalLight(Light light, vec3 position, vec3 normal)
{
   return calclucateLightColor(light.color, normalize(-light.direction), light.intensity, position, normal);
}
