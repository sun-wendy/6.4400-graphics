#version 330 core

out vec4 frag_color;

struct AmbientLight {
    bool enabled;
    vec3 ambient;
};

struct PointLight {
    bool enabled;
    vec3 position;
    vec3 diffuse;
    vec3 specular;
    vec3 attenuation;
};

struct DirectionalLight {
    bool enabled;
    vec3 direction;
    vec3 diffuse;
    vec3 specular;
};
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

in vec3 world_position;
in vec3 world_normal;
in vec2 tex_coord;

uniform vec3 camera_position;
uniform mat4 world_to_light_ndc_matrix;
uniform sampler2D shadow_texture;

uniform Material material; // material properties of the object
uniform AmbientLight ambient_light;
uniform PointLight point_light; 
uniform DirectionalLight directional_light;
vec3 CalcAmbientLight();
vec3 CalcPointLight(vec3 normal, vec3 view_dir);
vec3 CalcDirectionalLight(vec3 normal, vec3 view_dir);
float CalcShadow(vec3 world_position);

uniform sampler2D ambient_sampler;
uniform sampler2D diffuse_sampler;
uniform sampler2D specular_sampler;
uniform bool ambient_is_texture;
uniform bool diffuse_is_texture;
uniform bool specular_is_texture;

void main() {
    vec3 normal = normalize(world_normal);
    vec3 view_dir = normalize(camera_position - world_position);

    frag_color = vec4(0.0);

    if (ambient_light.enabled) {
        frag_color += vec4(CalcAmbientLight(), 1.0);
    }
    
    if (point_light.enabled) {
        frag_color += vec4(CalcPointLight(normal, view_dir), 1.0);
    }

    if (directional_light.enabled) {
        frag_color += vec4(CalcDirectionalLight(normal, view_dir), 1.0);
    }
}

float CalcShadow(vec3 world_position) {
    vec3 proj_coords = vec3(world_to_light_ndc_matrix * vec4(world_position, 1.0));
    proj_coords = proj_coords * 0.5 + 0.5;
    float closest_depth = texture(shadow_texture, proj_coords.xy).r;
    float cur_depth = proj_coords.z;
    float bias = 0.005;
    float shadow = cur_depth - bias > closest_depth ? 1.0 : 0.0;
    return shadow;
}

vec3 GetAmbientColor() {
    if (ambient_is_texture) {
        return vec3(texture(ambient_sampler, tex_coord));
    }
    return material.ambient;
}

vec3 GetDiffuseColor() {
    if (diffuse_is_texture) {
        return vec3(texture(diffuse_sampler, tex_coord));
    }
    return material.diffuse;
}

vec3 GetSpecularColor() {
    if (specular_is_texture) {
        return vec3(texture(specular_sampler, tex_coord));
    }
    return material.specular;
}

vec3 CalcAmbientLight() {
    return ambient_light.ambient * GetAmbientColor();
}

vec3 CalcPointLight(vec3 normal, vec3 view_dir) {
    PointLight light = point_light;
    vec3 light_dir = normalize(light.position - world_position);

    float diffuse_intensity = max(dot(normal, light_dir), 0.0);
    vec3 diffuse_color = diffuse_intensity * light.diffuse * GetDiffuseColor();

    vec3 reflect_dir = reflect(-light_dir, normal);
    float specular_intensity = pow(
        max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular_color = specular_intensity * 
        light.specular * GetSpecularColor();

    float distance = length(light.position - world_position);
    float attenuation = 1.0 / (light.attenuation.x + 
        light.attenuation.y * distance + 
        light.attenuation.z * (distance * distance));

    return attenuation * (diffuse_color + specular_color);
}

vec3 CalcDirectionalLight(vec3 normal, vec3 view_dir) {
    DirectionalLight light = directional_light;
    vec3 light_dir = normalize(-light.direction);
    float diffuse_intensity = max(dot(normal, light_dir), 0.0);
    vec3 diffuse_color = diffuse_intensity * light.diffuse * GetDiffuseColor();

    vec3 reflect_dir = reflect(-light_dir, normal);
    float specular_intensity = pow(
        max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular_color = specular_intensity * 
        light.specular * GetSpecularColor();

    vec3 final_color = diffuse_color + specular_color;

    if (CalcShadow(world_position) > 0.0) {
        return vec3(0.0);
    }

    return final_color;
}

