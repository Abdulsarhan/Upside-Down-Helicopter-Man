#include <stdio.h>
#include <assert.h>

#include <Windows.h>

#include <glad/glad.h>
#include "application.h"
#include "pal.h"
#include "stb_image.h"
#include <stdint.h>
#include <cglm/cglm.h>

// Signed ints

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

const i8  I8_MAX = INT8_MAX;	/**< Maximum value for a signed 8 bit integer */
const i16 I16_MAX = INT16_MAX;	/**< Maximum value for a signed 16 bit integer */
const i32 I32_MAX = INT32_MAX;	/**< Maximum value for a signed 32 bit integer */
const i64 I64_MAX = INT64_MAX;	/**< Maximum value for a signed 64 bit integer */

// Unsigned ints
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

const u8  U8_MAX = UINT8_MAX;   /**< Maximum value for an unsigned 8 bit integer */
const u16 U16_MAX = UINT16_MAX;	/**< Maximum value for an unsigned 16 bit integer */
const u32 U32_MAX = UINT32_MAX;	/**< Maximum value for an unsigned 32 bit integer */
const u64 U64_MAX = UINT64_MAX;	/**< Maximum value for an unsigned 64 bit integer */

// Floats
typedef float f32;
typedef double f64;


void check_compile_errors(u32 object, const char* type) {
    i32 success;
    char infoLog[1024];
    
    if (strcmp(type, "PROGRAM") != 0) {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, 1024, NULL, infoLog);
            fprintf(stderr, "| ERROR::SHADER: Compile-time error: Type: %s\n%s\n -- --------------------------------------------------- --\n", 
                    type, infoLog);
        }
    }
    else {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(object, 1024, NULL, infoLog);
            fprintf(stderr, "| ERROR::Shader: Link-time error: Type: %s\n%s\n -- --------------------------------------------------- --\n", 
                    type, infoLog);
        }
    }
}

u32 load_shader(const char* v_shader_file, const char* f_shader_file) {
    /* Read shader source code from files */
    size_t vshader_file_size = pal_get_file_size(v_shader_file);
    size_t fshader_file_size = pal_get_file_size(f_shader_file);
    char* vertexCode = (char*)malloc(vshader_file_size + 1);
    char* fragmentCode = (char*)malloc(fshader_file_size + 1);

    // null checking.
    if (!vertexCode || !fragmentCode) {
        fprintf(stderr, "ERROR::SHADER: Failed to read shader files\n");
        if (vertexCode) free(vertexCode);
        if (fragmentCode) free(fragmentCode);
        return 0;
    }

    pal_read_file(v_shader_file, vertexCode);
    vertexCode[vshader_file_size] = '\0';
    pal_read_file(f_shader_file, fragmentCode);
    fragmentCode[fshader_file_size] = '\0';
    
    
    const char* vShaderCode = vertexCode;
    const char* fShaderCode = fragmentCode;
    
    u32 vertex_shader, fragment_shader;
    
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vShaderCode, NULL);
    glCompileShader(vertex_shader);
    check_compile_errors(vertex_shader, "VERTEX");
    
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fShaderCode, NULL);
    glCompileShader(fragment_shader);
    check_compile_errors(fragment_shader, "FRAGMENT");
    
    u32 shader = glCreateProgram();
    glAttachShader(shader, vertex_shader);
    glAttachShader(shader, fragment_shader);
    glLinkProgram(shader);
    check_compile_errors(shader, "PROGRAM");
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    free(vertexCode);
    free(fragmentCode);
    
    return shader;
}

GLuint load_texture(const char* file) {
	i32 width = 0, height = 0, nrChannels = 0;
    u32 format = 0;

    glActiveTexture(GL_TEXTURE1);
    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    stbi_set_flip_vertically_on_load(pal_true);
    unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);
    assert(data != NULL);

    switch (nrChannels) {
    case 4:
        format = GL_RGBA;
        break;
    case 3:
        format = GL_RGB;
        break;
    case 2:
        format = GL_RG;
        break;
    case 1:
        format = GL_R8;
        break;

    }
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    data = NULL;
    return texture;
}

static void set_float(u32 shaderid, const char* uniformname, f32 value) {
    glUniform1f(glGetUniformLocation(shaderid, uniformname), value);
}

static void set_integer(u32 shaderid, const char* uniformname, i32 value) {
    glUniform1i(glGetUniformLocation(shaderid, uniformname), value);
}

static void set_unsigned_integer(u32 shaderid, const char* uniformname, u32 value) {
    glUniform1ui(glGetUniformLocation(shaderid, uniformname), value);
}

static void set_vector2f(u32 shaderid, const char* uniformname, f32 x, f32 y) {
    glUniform2f(glGetUniformLocation(shaderid, uniformname), x, y);
}

static void set_vector2f_vec(u32 shaderid, const char* uniformname, vec2 value) {
    glUniform2f(glGetUniformLocation(shaderid, uniformname), value[0], value[1]);
}

static void set_vector3f(u32 shaderid, const char* uniformname, f32 x, f32 y, f32 z) {
    glUniform3f(glGetUniformLocation(shaderid, uniformname), x, y, z);
}

static void set_vector3f_vec(u32 shaderid, const char* uniformname, vec3 value) {
    glUniform3f(glGetUniformLocation(shaderid, uniformname), value[0], value[1], value[2]);
}

static void set_vector4f(u32 shaderid, const char* uniformname, f32 x, f32 y, f32 z, f32 w) {
    glUniform4f(glGetUniformLocation(shaderid, uniformname), x, y, z, w);
}

static void set_vector4f_vec(u32 shaderid, const char* uniformname, vec4 value) {
    glUniform4f(glGetUniformLocation(shaderid, uniformname), value[0], value[1], value[2], value[3]);
}

static void set_matrix4(u32 shaderid, const char* uniformname, mat4 matrix) {
    glUniformMatrix4fv(glGetUniformLocation(shaderid, uniformname), 1, GL_FALSE, (float*)matrix);
}

static void set_float_array(u32 shaderid, const char* uniformname, const f32* values, u32 count) {
    glUniform1fv(glGetUniformLocation(shaderid, uniformname), count, values);
}

static void set_integer_array(u32 shaderid, const char* uniformname, const i32* values, u32 count) {
    glUniform1iv(glGetUniformLocation(shaderid, uniformname), count, values);
}

static void set_unsigned_integer_array(u32 shaderid, const char* uniformname, const u32* values, u32 count) {
    glUniform1uiv(glGetUniformLocation(shaderid, uniformname), count, values);
}

static void set_vector2f_array(u32 shaderid, const char* uniformname, const vec2* values, u32 count) {
    glUniform2fv(glGetUniformLocation(shaderid, uniformname), count, (float*)values);
}

static void set_vector3f_array(u32 shaderid, const char* uniformname, const vec3* values, u32 count) {
    glUniform3fv(glGetUniformLocation(shaderid, uniformname), count, (float*)values);
}

static void set_vector4f_array(u32 shaderid, const char* uniformname, const vec4* values, u32 count) {
    glUniform4fv(glGetUniformLocation(shaderid, uniformname), count, (float*)values);
}

static void set_matrix4_array(u32 shaderid, const char* uniformname, const mat4* matrices, u32 count) {
    glUniformMatrix4fv(glGetUniformLocation(shaderid, uniformname), count, GL_FALSE, (float*)matrices);
}

 static f32 quadVertices[] = {
    // positions (x, y, z)   // texture coords (u, v)
     1.0f,  1.0f, 0.0f,      1.0f, 1.0f,   // top right
     1.0f, -1.0f, 0.0f,      1.0f, 0.0f,   // bottom right
    -1.0f, -1.0f, 0.0f,      0.0f, 0.0f,   // bottom left
    -1.0f,  1.0f, 0.0f,      0.0f, 1.0f    // top left
};

static u32 quadIndices[] = {
    0, 1, 2,  /* first triangle */
    2, 3, 0   /* second triangle */
};

// globals
static mat4 pv = {0};
static GLuint VAO = 0, VBO = 0, EBO = 0;
static GLuint sprite_shader = 0;
static GLuint player_texture, chopper_texture;
static GLuint rocket_guy_left_texture, chaingun_guy_left_texture, knife_guy_left_texture, rifle_guy_left_texture, pistol_guy_left_texture;
static GLuint rocket_guy_right_texture, chaingun_guy_right_texture, knife_guy_right_texture, rifle_guy_right_texture, pistol_guy_right_texture;
static GLuint health_pack_texture;
static GLuint ground_texture, scrolling_clouds_texture, scrolling_trees_texture, scrolling_mountains_texture, sun_texture, sky_texture;
static GLuint rifle_bullet_texture, pistol_bullet_texture, knife_bullet_texture, chaingun_bullet_texture, rocket_bullet_texture;
static GLuint rifle_right_texture, pistol_right_texture, rocket_launcher_right_texture, chaingun_right_texture, knife_right_texture;
static GLuint rifle_left_texture, pistol_left_texture, rocket_launcher_left_texture, chaingun_left_texture, knife_left_texture;

#define ASCII_MAX 128
static GLuint bitmap_font[ASCII_MAX]; // indices 0 - 9 corresponds to numbers 0 - 9 everything after that is lowercase letters.
static i32 first_run = 1;
static vec2 mouse_position = {0, 0};
static f32 current_time = 0;
static f32 last_frame_time = 0;
static f32 delta_time = 0;

static pal_sound* explosion_sound;
static pal_sound* rocket_firing;
static pal_sound* chaingun_revving;
static pal_sound* chaingun_revving_two;
static pal_sound* gun_shooting;

// fire times
f32 knife_firetime = 0;
f32 pistol_firetime = 0;
f32 rifle_firetime = 0;
f32 rocket_firetime = 0;
f32 chaingun_firetime = 0;
f32 chaingun_revtime = 0;

// fire times for enemies
f32 knife_guys_timer = 0;
f32 pistol_guys_timer = 0;
f32 rifle_guys_timer = 0;
f32 rocket_guys_timer = 0;
f32 chaingun_guys_timer = 0;
f32 spawn_timer = 0;

// score tracking.
i32 score;
// Gun sizes.
static const vec2 rifle_size = {9.0f, 3.0f};
static const vec2 rocket_launcher_size = {12.0f, 3.0f};
static const vec2 pistol_size = {9.0f, 3.0f};
static const vec2 chaingun_size = {18.0f, 9.0f};
static const vec2 knife_size = {9.0f, 3.0f};

// Bullet sizes.
static const vec2 knife_bullet_size = {9.0f, 9.0f};
static const vec2 rifle_bullet_size = {6.0f, 2.0f};
static const vec2 chaingun_bullet_size = {9.0f, 3.0f};
static const vec2 rocket_bullet_size = {24.0f, 24.0f};
static const vec2 pistol_bullet_size = {3.0f, 3.0f};

// Bullet speeds.
static f32 knife_bullet_speed = 300;
static f32 pistol_bullet_speed = 500;
static f32 rifle_bullet_speed = 600;
static f32 chaingun_bullet_speed = 600;
static f32 rocket_bullet_speed = 200;

// entity sizes.
static const vec2 player_size = {16.0f, 32.0f};
static const vec2 guy_size = {12.0f, 12.0f};
static vec2 chaingun_guy_size = {14.0f, 14.0f};
static const vec2 chopper_size = {59.0f, 35.0f};

static int player_health = {100};
static int currently_held_weapon = 0;

static vec2 rifle_guy_pos = {0, -110};
static vec2 chaingun_guy_pos = {150, -109};
static vec2 rocket_guy_pos = {100, -110};
static vec2 knife_guy_pos = {200, -110};

static const f32 no_rotation = 0.0f;

static vec4 mouse_pos_world = {0,0,0,0};
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080


// sizes for all entities are stored
// out of band.

enum _entitytype {

    // player

    // enemies
    ENTITY_TYPE_KNIFE_GUY,
    ENTITY_TYPE_PISTOL_GUY,
    ENTITY_TYPE_RIFLE_GUY,
    ENTITY_TYPE_ROCKET_GUY,
    ENTITY_TYPE_CHAINGUN_GUY,

    ENTITY_TYPE_PLAYER,

    // bullet types
    ENTITY_TYPE_KNIFE_BULLET, // what the fuck?
    ENTITY_TYPE_PISTOL_BULLET,
    ENTITY_TYPE_RIFLE_BULLET,
    ENTITY_TYPE_CHAINGUN_BULLET,
    ENTITY_TYPE_ROCKET_BULLET,

    // pickups
    ENTITY_TYPE_HEALTH_PACK,
    ENTITY_TYPE_COUNT,
};

typedef struct Entities {
    i32 entity_count;
    u64 next_entity_id;
    u64* id; // unique identifier of the entity.
    u32* entity_type; // is it a rifle guy? pistol guy? friendly bullet?  enemy bullet?
    i32* health; // health of the enemy or player. Does not apply to bullets since they die immediately.
	vec2* positions; // position of the guy or the bullet
    vec2* velocities;
    f32* rotations; // rotation of the bullet. Should be 0 for the guys.
    i32* bullet_owned_by; // is the bullet owned by the player or an enemy?
    pal_bool* facing_left; // Specifies which way the guy facing. Left or right?
}Entities;

Entities entities;
#define ENTITY_LIMIT 2000

mat4 view;
mat4 proj;

static void draw_sprite(GLuint texture, vec2 position, vec2 size, f32 rotation) {
    // Model matrix
    mat4 model;
    glm_mat4_identity(model);
    vec3 pos = {position[0], position[1], 0.0f};
    glm_translate(model, pos);
    
    vec3 z_axis = {0.0f, 0.0f, 1.0f};
    glm_rotate(model, rotation, z_axis);
    
    vec3 siz = {size[0], size[1], 1.0f};
    glm_scale(model, siz);

    // Set uniforms using your uniform setters

    glUseProgram(sprite_shader);

    set_matrix4(sprite_shader, "model", model);
    if (texture == sun_texture || texture == sky_texture || texture == ground_texture) { // render the sky and the sun in screen-space
		set_matrix4(sprite_shader, "pv", proj);
    } else {
		set_matrix4(sprite_shader, "pv", pv);
    }
    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    set_integer(sprite_shader, "ourTexture", 0); // Texture unit 0
    
    // Set color (white tint)
    set_vector4f(sprite_shader, "color", 1.0f, 1.0f, 1.0f, 1.0f);
    
    // Bind VAO and draw
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

}

static float lerp(float a, float b, float t) {
    return a * (1 - t) + b * t;
}

static void load_font() {
    char string[256];

    // Load digits '0' to '9'
    for (char c = '0'; c <= '9'; c++) {
        snprintf(string, sizeof(string), "textures/num_%c.png", c);
        bitmap_font[(int)c] = load_texture(string);
    }

    #if 0
    // Load lowercase 'a' to 'z'
    for (char c = 'a'; c <= 'z'; c++) {
        snprintf(string, sizeof(string), "textures/lower_%c.png", c);
        bitmap_font[(int)c] = load_texture(string);
    }
#endif
    // Load uppercase 'A' to 'Z'
    for (char c = 'A'; c <= 'Z'; c++) {
        snprintf(string, sizeof(string), "textures/upper_%c.png", c);
        bitmap_font[(int)c] = load_texture(string);
    }

    snprintf(string, sizeof(string), "textures/colon.png");
    bitmap_font[(int)':'] = load_texture(string);
}

static void draw_text(const char* text, f32 x, f32 y, f32 font_size, f32 spacing, vec3 color) {
    mat4 model;
    glm_mat4_identity(model);

    vec3 base_pos = {x, y, 0.0f};

    glUseProgram(sprite_shader);
    set_matrix4(sprite_shader, "pv", proj); // Screen-space projection

    for (int i = 0; text[i] != '\0'; i++) {
        char c = text[i];
        if ((int)c < 0 || bitmap_font[(int)c] == 0)
            continue;

        // Calculate character position with spacing
        f32 advance = i * (font_size + spacing);
        vec3 pos = { base_pos[0] + advance, base_pos[1], 0.0f };

        glm_mat4_identity(model);
        glm_translate(model, pos);

        vec3 siz = {font_size, font_size, 1.0f};
        glm_scale(model, siz);

        set_matrix4(sprite_shader, "model", model);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, bitmap_font[(int)c]);
        set_integer(sprite_shader, "ourTexture", 0);

        set_vector4f(sprite_shader, "color", color[0], color[1], color[2], 1.0f);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}
i32 check_aabb_collision(vec2 one_pos, vec2 one_size, vec2 two_pos, vec2 two_size) {
    vec2 half_one, half_two;
    glm_vec2_scale(one_size, 0.5f, half_one);
    glm_vec2_scale(two_size, 0.5f, half_two);

    vec2 one_min, one_max, two_min, two_max;
    glm_vec2_sub(one_pos, half_one, one_min);
    glm_vec2_add(one_pos, half_one, one_max);
    glm_vec2_sub(two_pos, half_two, two_min);
    glm_vec2_add(two_pos, half_two, two_max);

    i32 collision_x = one_max[0] >= two_min[0] && two_max[0] >= one_min[0];
    i32 collision_y = one_max[1] >= two_min[1] && two_max[1] >= one_min[1];

    return collision_x && collision_y;
}
void get_mouse_pos_world() {
    vec4 target_pos_clip_space = {0, 0, 0, 0};
    target_pos_clip_space[0] = (mouse_position[0] / SCREEN_WIDTH) * 2.0f - 1.0f;
    target_pos_clip_space[1] = -((mouse_position[1] / SCREEN_HEIGHT) * 2.0f - 1.0f);
    target_pos_clip_space[2] = 0.0f;
    target_pos_clip_space[3] = 1.0f;

    mat4 inverse_pv = {0};
    glm_mat4_inv(pv, inverse_pv);
    glm_mat4_mulv(inverse_pv,  target_pos_clip_space, mouse_pos_world);
}

void adjust_gun_rotations(Entities* entities) {
    vec2 direction = {0, 0};
    f32 rotation = 0;
    for (int i = 0; i < entities->entity_count; i++) {
        switch (entities->entity_type[i]) {
            case ENTITY_TYPE_PLAYER:
				vec2 player_gun_pos;
				glm_vec2_add(entities->positions[i], (vec2){15, -36}, player_gun_pos);
				glm_vec2_sub(mouse_pos_world, player_gun_pos, direction);
				glm_vec2_normalize(direction);

				rotation = atan2f(direction[1], direction[0]); // y, x
				entities->rotations[i] = rotation - PI; // the player is upside-down.
                break;
            case ENTITY_TYPE_CHAINGUN_GUY:
            case ENTITY_TYPE_PISTOL_GUY:
            case ENTITY_TYPE_KNIFE_GUY:
            case ENTITY_TYPE_ROCKET_GUY:
            case ENTITY_TYPE_RIFLE_GUY:
				for (int j = 0; j < entities->entity_count; j++) {
				
					if (entities->entity_type[j] == ENTITY_TYPE_PLAYER) {
					
						glm_vec2_sub(entities->positions[j], entities->positions[i], direction);
                        if (direction[0] < 0) {
						rotation = atan2f(direction[1], direction[0]); // y, x
                            entities->facing_left[i] = pal_true;
                        } else {
						rotation = atan2f(direction[1], direction[0]); // y, x
                            entities->facing_left[i] = pal_false;
                        }
                        if (entities->facing_left[i] == pal_true) {
							entities->rotations[i] = rotation - PI; // the player is upside-down.
                        } else {
							entities->rotations[i] = rotation; // the player is upside-down.
                        }
					}
				}
                break;

        }
    }
}

typedef struct rgb_color {
    float r;
    float g;
    float b;
}rgb_color;

typedef struct rgba_color {
    float r;
    float g;
    float b;
    float a;
}rgba_color;

static inline rgb_color lerp_red_green_100(int value) {
    // Clamp value to [0, 100]
    if (value < 0) value = 0;
    if (value > 100) value = 100;

    float t = (float)value / 100.0f;

    rgb_color color = {
        0.95f - t,          // Red increases with t
        t,   // Green decreases with t
        0.0f        // Blue stays zero
    };

    return color;
}

i32 myitoa(i32 value, char* string)
{
    char tmp[16];
    char* tp = tmp;
    u32 absolute_value;

    u32 is_negative = ((u32)(value >> 31)) & 1;

    absolute_value = is_negative ? (u32)(-value) : (u32)value;

	do {
		*tp++ = (absolute_value % 10) + '0';
		absolute_value /= 10;
	} while (absolute_value);

    i32 len = tp - tmp;

    if (is_negative) {
        *string++ = '-';
        len++;
    }

    while (tp > tmp)
        *string++ = *--tp;

    return len;
}

int create_entity(vec2 position, u32 entity_type, pal_bool facing_left, pal_bool bullet_owner) {
    int entity = entities.entity_count;
    entities.entity_count++;
    entities.id[entity] = entities.next_entity_id;
    entities.next_entity_id++;
    entities.entity_type[entity] = entity_type;
    memset(entities.velocities[entity],0, sizeof(vec2));
    assert(entities.entity_type[entity] >= 0 && entities.entity_type[entity] < ENTITY_TYPE_COUNT);

    glm_vec2_add((vec2){position[0], position[1]}, (vec2){0, 0}, entities.positions[entity]);
    entities.rotations[entity] = 0;
    entities.facing_left[entity] = facing_left;
    if (entity_type == ENTITY_TYPE_CHAINGUN_GUY) {
    
		entities.health[entity] = 200;
    } else if (entity_type == ENTITY_TYPE_ROCKET_GUY) {
        entities.health[entity] = 150;
    } else if (entity_type == ENTITY_TYPE_RIFLE_GUY) {
        entities.health[entity] = 100;
    } else if (entity_type == ENTITY_TYPE_PISTOL_GUY) {
        entities.health[entity] = 50;
    } else if (entity_type == ENTITY_TYPE_KNIFE_GUY) {
        entities.health[entity] = 100;
    }
    entities.bullet_owned_by[entity] = bullet_owner;
    return entity;
}

static void destroy_entity(int entity_idx) {
    i32 last_entity = entities.entity_count - 1;

    if (entity_idx != last_entity) {
        entities.id[entity_idx] = entities.id[last_entity];
        entities.entity_type[entity_idx] = entities.entity_type[last_entity];
        entities.health[entity_idx] = entities.health[last_entity];
        entities.positions[entity_idx][0] = entities.positions[last_entity][0];
        entities.positions[entity_idx][1] = entities.positions[last_entity][1];
        entities.rotations[entity_idx] = entities.rotations[last_entity];
        entities.bullet_owned_by[entity_idx] = entities.bullet_owned_by[last_entity];
        entities.facing_left[entity_idx] = entities.facing_left[last_entity];
		entities.velocities[entity_idx][0] = entities.velocities[last_entity][0];
		entities.velocities[entity_idx][1] = entities.velocities[last_entity][1];
    }

    entities.entity_count--;
}
static void destroy_all_entities() {
    i32 entity_count = entities.entity_count;
    entities.entity_count = 0;
    entities.next_entity_id = 0;
    memset(entities.id,0,sizeof(u64) * entity_count);
    memset(entities.entity_type,0,sizeof(u32) * entity_count);
    memset(entities.health,0,sizeof(i32) * entity_count);
    memset(entities.positions,0,sizeof(vec2) * entity_count);
    memset(entities.rotations,0,sizeof(f32) * entity_count);
    memset(entities.bullet_owned_by,0,sizeof(i32) * entity_count);
    memset(entities.facing_left,0,sizeof(i32) * entity_count);
}

static void game_update_and_render(pal_window* window) {
    // Initialize OpenGL objects on first run
    if (first_run) {
        // load sounds
		explosion_sound = pal_load_sound("sounds/explosion.ogg");
		gun_shooting = pal_load_sound("sounds/gun_firing.ogg");
		rocket_firing = pal_load_sound("sounds/rocket_firing.ogg");
		chaingun_revving = pal_load_sound("sounds/chaingun_revving.ogg");
		chaingun_revving_two = pal_load_sound("sounds/chaingun_revving_2.ogg");

        // sprite shader
        sprite_shader = load_shader("shaders/sprite.vert", "shaders/sprite.frag");
        // Player
        player_texture = load_texture("textures/player.png");
        // Chopper
        chopper_texture = load_texture("textures/chopper.png");
        // health pack
        health_pack_texture = load_texture("textures/health_pack.png");

        // Enemies facing left
        rocket_guy_left_texture = load_texture("textures/rocket_guy_left.png");
        chaingun_guy_left_texture = load_texture("textures/chaingun_guy_left.png");
        rifle_guy_left_texture = load_texture("textures/rifle_guy_left.png");
        knife_guy_left_texture = load_texture("textures/knife_guy_left.png");
        pistol_guy_left_texture = load_texture("textures/pistol_guy_left.png");

        // Enemies facing right
        rocket_guy_right_texture = load_texture("textures/rocket_guy_right.png");
        chaingun_guy_right_texture = load_texture("textures/chaingun_guy_right.png");
        rifle_guy_right_texture = load_texture("textures/rifle_guy_right.png");
        knife_guy_right_texture = load_texture("textures/knife_guy_right.png");
        pistol_guy_right_texture = load_texture("textures/pistol_guy_right.png");
        // Bullets
        rifle_bullet_texture = load_texture("textures/rifle_bullet.png"); // used also for chaingun
        chaingun_bullet_texture = rifle_bullet_texture;                       // used also for chaingun
        pistol_bullet_texture = load_texture("textures/pistol_bullet.png");
        rocket_bullet_texture = load_texture("textures/rocket_bullet.png");
        knife_bullet_texture = load_texture("textures/knife_bullet.png");

        // Weapons facing right
        rifle_right_texture = load_texture("textures/rifle_right.png");
		pistol_right_texture = load_texture("textures/pistol_right.png");
        rocket_launcher_right_texture = load_texture("textures/rocket_launcher_right.png");
        chaingun_right_texture = load_texture("textures/chaingun_right.png");
        knife_right_texture = load_texture("textures/knife_right.png");
        // Weapons facing left
        rifle_left_texture = load_texture("textures/rifle_left.png");
		pistol_left_texture = load_texture("textures/pistol_left.png");
        rocket_launcher_left_texture = load_texture("textures/rocket_launcher_left.png");
        chaingun_left_texture = load_texture("textures/chaingun_left.png");
        knife_left_texture = load_texture("textures/knife_left.png");
        // level
        ground_texture = load_texture("textures/ground.png");
        scrolling_clouds_texture = load_texture("textures/scrolling_clouds.png");
        scrolling_mountains_texture = load_texture("textures/scrolling_mountains.png");
        sun_texture = load_texture("textures/sun.png");
        sky_texture = load_texture("textures/sky.png");
        scrolling_trees_texture = load_texture("textures/scrolling_trees.png");
        load_font();
        // Generate and bind VAO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // Bind and fill VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

        // Bind and fill EBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

        // Position attribute (location 0) - 3 floats for x,y,z
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Texture coordinate attribute (location 1) - 2 floats for u,v
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        // Unbind VAO
        glBindVertexArray(0);

        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        first_run = 0;

        // this will be wrong when debugging, but when we fullscreen
        // the game, it will be fine.
        glViewport(0, 0, (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT);
        entities.id = malloc(sizeof(u64) * ENTITY_LIMIT);
        entities.entity_type = malloc(sizeof(u32) * ENTITY_LIMIT);
        entities.facing_left = malloc(sizeof(pal_bool) * ENTITY_LIMIT);
        entities.rotations = malloc(sizeof(f32) * ENTITY_LIMIT);
        entities.health = malloc(sizeof(i32) * ENTITY_LIMIT);
        entities.bullet_owned_by = malloc(sizeof(i32) * ENTITY_LIMIT);
        entities.positions = malloc(sizeof(vec2) * ENTITY_LIMIT);
        entities.velocities = malloc(sizeof(vec2) * ENTITY_LIMIT);
		create_entity((vec2){0,-110}, ENTITY_TYPE_PLAYER, 0, 0);
    }
    //--------------------------------------------------
    // Randomly Spawning Entities
    //--------------------------------------------------
		uint64_t state = 0;
		pal_srand(&state, current_time);
		uint32_t location = pal_rand(&state) % 100;
		pal_srand(&state, current_time);
		uint32_t enemy_type = pal_rand(&state) % 4;
        if (spawn_timer < (current_time - 0.5) || spawn_timer == 0) {
            create_entity((vec2){entities.positions[0][0] + abs(location) + 200, -110}, enemy_type, 0, 0);
            spawn_timer = current_time;
        }

    //--------------------------------------------------
    // Renderer Stuff that Gamplay Code Relies on
    //--------------------------------------------------
    // Set up matrices

    // View matrix
    glm_mat4_identity(view);
    f32 camera_x = entities.positions[0][0] + 200;
    f32 camera_y = 0;
    vec3 camera_translation = {-camera_x, -camera_y, 0.0f};
    glm_translate(view, camera_translation);

// Projection matrix (orthographic)
#define WORLD_WIDTH 640
#define WORLD_HEIGHT 360
    // right-handed. Y up.
    glm_ortho(-WORLD_WIDTH / 2.0f, WORLD_WIDTH / 2.0f, -WORLD_HEIGHT / 2.0f, WORLD_HEIGHT / 2.0f, -1.0f, 1.0f, proj);
    // combined proj + view matrix
    glm_mul(proj, view, pv);

    //--------------------------------------------------
    // Player movement
    //--------------------------------------------------
    const f32 player_speed = 400.0f;
    const f32 half_of_sqrt_of_two = 0.707107f;
    vec2 move_delta = {0};
    pal_bool get_back = 0;
    move_delta[1] += player_speed * is_key_down(PAL_W);
    move_delta[1] -= player_speed * is_key_down(PAL_S);
    move_delta[0] -= player_speed * is_key_down(PAL_A);
    move_delta[0] += player_speed * is_key_down(PAL_D);

    if (move_delta[0] != 0 && move_delta[1] != 0) {
        move_delta[0] *= half_of_sqrt_of_two;
        move_delta[1] *= half_of_sqrt_of_two;
    }
	entities.positions[0][0] += move_delta[0] * delta_time;
	entities.positions[0][1] += move_delta[1] * delta_time;

    // This ensures that the player cannot go too far back.
    if (entities.positions[0][0] <= -750) {
        get_back = 1;
    }

    // This ensures that the player cannot go too high off the screen.
    if (entities.positions[0][1] >= 160) {
        entities.positions[0][1] = 160.0f;
    }

    // This ensures that the player cannot go through the ground.
    if (entities.positions[0][1] <= -90.0f) {
        entities.positions[0][1] = -90.0f;
    }
   //--------------------------------------------------
   // Player weapons & shooting
   //--------------------------------------------------

    get_mouse_pos_world();
    adjust_gun_rotations(&entities);
    enum {
        WEAPON_RIFLE,
        WEAPON_KNIFE,
        WEAPON_PISTOL,
        WEAPON_CHAINGUN,
        WEAPON_ROCKET_LAUNCHER,
        WEAPON_COUNT
    };

    if (is_key_pressed(PAL_1)) {
        currently_held_weapon = WEAPON_KNIFE;
    }
    if (is_key_pressed(PAL_2)) {
        currently_held_weapon = WEAPON_PISTOL;
    }
    if (is_key_pressed(PAL_3)) {
        currently_held_weapon = WEAPON_RIFLE;
    }
    if (is_key_pressed(PAL_4)) {
        currently_held_weapon = WEAPON_CHAINGUN;
    }
    if (is_key_pressed(PAL_5)) {
        currently_held_weapon = WEAPON_ROCKET_LAUNCHER;
    }
    if (is_mouse_down(PAL_MOUSE_LEFT)) {

        assert(currently_held_weapon >= 0 && currently_held_weapon < WEAPON_COUNT);
		vec2 direction = {0, 0};
        int bullet;
        switch (currently_held_weapon) {

            case WEAPON_KNIFE:
                if (knife_firetime < (current_time - 1.5) || knife_firetime == 0) {
					bullet = create_entity(entities.positions[0], ENTITY_TYPE_KNIFE_BULLET, 0, 0);
					entities.bullet_owned_by[bullet] = 0;
					glm_vec2_sub((vec2){mouse_pos_world[0], mouse_pos_world[1]}, entities.positions[0], direction);
					glm_vec2_normalize(direction);
					entities.rotations[bullet] = atan2f(direction[1], direction[0]);
					entities.velocities[bullet][0] = direction[0] * knife_bullet_speed;
					entities.velocities[bullet][1] = direction[1] * knife_bullet_speed;
                    knife_firetime = current_time;
                }
                break;
            case WEAPON_PISTOL:
                if (pistol_firetime < (current_time - 0.5) || pistol_firetime == 0) {
                    pal_play_sound(gun_shooting, 0.6);
                    bullet = create_entity(entities.positions[0], ENTITY_TYPE_PISTOL_BULLET, 0, 0);
                    entities.bullet_owned_by[bullet] = 0;
                    glm_vec2_sub((vec2){mouse_pos_world[0], mouse_pos_world[1]}, entities.positions[0], direction);
                    glm_vec2_normalize(direction);
                    entities.rotations[bullet] = atan2f(direction[1], direction[0]);
                    entities.velocities[bullet][0] = direction[0] * pistol_bullet_speed;
                    entities.velocities[bullet][1] = direction[1] * pistol_bullet_speed;
                    pistol_firetime = current_time;
                }
                break;
            case WEAPON_RIFLE:
                if (rifle_firetime < (current_time - 0.3) || rifle_firetime == 0) {
                    pal_play_sound(gun_shooting, 0.6);
                    bullet = create_entity(entities.positions[0], ENTITY_TYPE_RIFLE_BULLET, 0, 0);
                    entities.bullet_owned_by[bullet] = 0;
                    glm_vec2_sub((vec2){mouse_pos_world[0], mouse_pos_world[1]}, entities.positions[0], direction);
                    glm_vec2_normalize(direction);
                    entities.rotations[bullet] = atan2f(direction[1], direction[0]);
                    entities.velocities[bullet][0] = direction[0] * rifle_bullet_speed;
                    entities.velocities[bullet][1] = direction[1] * rifle_bullet_speed;
                    rifle_firetime = current_time;
                }
                break;
            case WEAPON_CHAINGUN:
                if (chaingun_firetime < (current_time - 0.05) || chaingun_firetime == 0) {
                    if (chaingun_revtime < (current_time - 0.5) || chaingun_revtime == 0) {
                        pal_play_sound(chaingun_revving_two, 0.7);
                        chaingun_revtime = current_time;
                    }
                    pal_stop_sound(gun_shooting);
                    pal_play_sound(gun_shooting, 0.6);
                    bullet = create_entity(entities.positions[0], ENTITY_TYPE_CHAINGUN_BULLET, 0, 0);
                    entities.bullet_owned_by[bullet] = 0;
                    glm_vec2_sub((vec2){mouse_pos_world[0], mouse_pos_world[1]}, entities.positions[0], direction);
                    glm_vec2_normalize(direction);
                    entities.rotations[bullet] = atan2f(direction[1], direction[0]);
                    entities.velocities[bullet][0] = direction[0] * chaingun_bullet_speed;
                    entities.velocities[bullet][1] = direction[1] * chaingun_bullet_speed;
                    chaingun_firetime = current_time;
                }
                break;
            case WEAPON_ROCKET_LAUNCHER:
                if (rocket_firetime < (current_time - 1.5) || rocket_firetime == 0) {
                    pal_play_sound(rocket_firing, 0.5);
                    bullet = create_entity(entities.positions[0], ENTITY_TYPE_ROCKET_BULLET, 0, 0);
                    entities.bullet_owned_by[bullet] = 0;
                    glm_vec2_sub((vec2){mouse_pos_world[0], mouse_pos_world[1]}, entities.positions[0], direction);
                    glm_vec2_normalize(direction);
                    entities.rotations[bullet] = atan2f(direction[1], direction[0]);
                    entities.velocities[bullet][0] = direction[0] * rocket_bullet_speed;
                    entities.velocities[bullet][1] = direction[1] * rocket_bullet_speed;
                    rocket_firetime = current_time;
                }
                break;
        }

    }

    if (!is_mouse_down(PAL_MOUSE_LEFT)) {
        pal_stop_sound(chaingun_revving_two);
    }
    //--------------------------------------------------
    // Enemy Bullets
    //--------------------------------------------------
    for (int i = 0; i < entities.entity_count; i++) {
		int bullet;
        vec2 direction = {0, 0};
        switch (entities.entity_type[i]) {
        case ENTITY_TYPE_KNIFE_GUY:
			if (knife_guys_timer < (current_time - 0.5)) {
				bullet = create_entity(entities.positions[i], ENTITY_TYPE_KNIFE_BULLET, 0, 0);
				entities.bullet_owned_by[bullet] = 1;
				glm_vec2_sub(entities.positions[0], entities.positions[i], direction);
				glm_vec2_normalize(direction);
				entities.rotations[bullet] = atan2f(direction[1], direction[0]);
				entities.velocities[bullet][0] = direction[0] * knife_bullet_speed;
				entities.velocities[bullet][1] = direction[1] * knife_bullet_speed;
                knife_guys_timer = current_time;
			}
            break;
        case ENTITY_TYPE_PISTOL_GUY:
            if (pistol_guys_timer < (current_time - 0.5)) {
                bullet = create_entity(entities.positions[i], ENTITY_TYPE_PISTOL_BULLET, 0, 0);
				pal_play_sound(gun_shooting, 0.6);
                entities.bullet_owned_by[bullet] = 1;
                glm_vec2_sub(entities.positions[0], entities.positions[i], direction);
                glm_vec2_normalize(direction);
                entities.rotations[bullet] = atan2f(direction[1], direction[0]);
                entities.velocities[bullet][0] = direction[0] * pistol_bullet_speed;
                entities.velocities[bullet][1] = direction[1] * pistol_bullet_speed;
				pistol_guys_timer = current_time;
            }
            break;
        case ENTITY_TYPE_RIFLE_GUY:
            if (rifle_guys_timer < (current_time - 0.5)) {
                bullet = create_entity(entities.positions[i], ENTITY_TYPE_RIFLE_BULLET, 0, 0);
				pal_play_sound(gun_shooting, 0.6);
                entities.bullet_owned_by[bullet] = 1;
                glm_vec2_sub(entities.positions[0], entities.positions[i], direction);
                glm_vec2_normalize(direction);
                entities.rotations[bullet] = atan2f(direction[1], direction[0]);
                entities.velocities[bullet][0] = direction[0] * rifle_bullet_speed;
                entities.velocities[bullet][1] = direction[1] * rifle_bullet_speed;
                rifle_guys_timer = current_time;
            }
            break;
        case ENTITY_TYPE_ROCKET_GUY:
            if (rocket_guys_timer < (current_time - 2.5)) {
                pal_play_sound(rocket_firing, 0.3);
                bullet = create_entity(entities.positions[i], ENTITY_TYPE_ROCKET_BULLET, 0, 0);
                entities.bullet_owned_by[bullet] = 1;
                glm_vec2_sub(entities.positions[0], entities.positions[i], direction);
                glm_vec2_normalize(direction);
                entities.rotations[bullet] = atan2f(direction[1], direction[0]);
                entities.velocities[bullet][0] = direction[0] * rocket_bullet_speed;
                entities.velocities[bullet][1] = direction[1] * rocket_bullet_speed;
				rocket_guys_timer = current_time;
            }
            break;
        case ENTITY_TYPE_CHAINGUN_GUY:
            if (chaingun_guys_timer < (current_time - 0.5)) {
				pal_play_sound(gun_shooting, 0.6);
				bullet = create_entity(entities.positions[i], ENTITY_TYPE_CHAINGUN_BULLET, 0, 0);
				entities.bullet_owned_by[bullet] = 1;
				glm_vec2_sub(entities.positions[0], entities.positions[i], direction);
				glm_vec2_normalize(direction);
				entities.rotations[bullet] = atan2f(direction[1], direction[0]);
				entities.velocities[bullet][0] = direction[0] * chaingun_bullet_speed;
				entities.velocities[bullet][1] = direction[1] * chaingun_bullet_speed;
				chaingun_guys_timer = current_time;
            }
            break;

        }
    }

    //--------------------------------------------------
    // Making Bullets Move every frame
    //--------------------------------------------------
    for (int i = 0; i < entities.entity_count; i++) {
    
        switch (entities.entity_type[i]) {
                case ENTITY_TYPE_KNIFE_BULLET:
                case ENTITY_TYPE_PISTOL_BULLET:
                case ENTITY_TYPE_RIFLE_BULLET:
                case ENTITY_TYPE_CHAINGUN_BULLET:
                case ENTITY_TYPE_ROCKET_BULLET:
                    glm_vec2_add(entities.positions[i], (vec2){entities.velocities[i][0] * delta_time, entities.velocities[i][1] * delta_time}, entities.positions[i]);
                    break;
		}
    }

    //--------------------------------------------------
    // Bullet Collisions
    //--------------------------------------------------
    vec2 collision_size = {player_size[0] / 1, player_size[1] / 1};
    pal_bool collided = 0;
    for (int i = 0; i < entities.entity_count; i++) {
        switch (entities.entity_type[i]) {
            case ENTITY_TYPE_KNIFE_BULLET:
                if (entities.bullet_owned_by[i] == 1) {
                    pal_bool collided = check_aabb_collision(entities.positions[0], collision_size, entities.positions[i], knife_bullet_size);
                    if (collided) {
                        player_health -= 1;
                        destroy_entity(i);
                    }
                } else {
                    for (int j = 0; j < entities.entity_count; j++) {
                        switch (entities.entity_type[j]) {
                            case ENTITY_TYPE_KNIFE_GUY:
                            case ENTITY_TYPE_PISTOL_GUY:
                            case ENTITY_TYPE_RIFLE_GUY:
                            case ENTITY_TYPE_ROCKET_GUY:
                                collided = check_aabb_collision(entities.positions[j], collision_size, entities.positions[i], guy_size);
								if (collided) {
									entities.health[j] -= 100;
									destroy_entity(i);
								}
                                break;
                            case ENTITY_TYPE_CHAINGUN_GUY:
                                collided = check_aabb_collision(entities.positions[j], collision_size, entities.positions[i], chaingun_guy_size);
								if (collided) {
									entities.health[j] -= 100;
									destroy_entity(i);
								}
                                break;
                        }
                    }
                }
                break;
            case ENTITY_TYPE_PISTOL_BULLET:
                if (entities.bullet_owned_by[i] == 1) {
                    pal_bool collided = check_aabb_collision(entities.positions[0], collision_size, entities.positions[i], pistol_bullet_size);
                    if (collided) {
                        player_health -= 1;
                        destroy_entity(i);
                    }
                } else {
                    for (int j = 0; j < entities.entity_count; j++) {
                        switch (entities.entity_type[j]) {
                            case ENTITY_TYPE_KNIFE_GUY:
                            case ENTITY_TYPE_PISTOL_GUY:
                            case ENTITY_TYPE_RIFLE_GUY:
                            case ENTITY_TYPE_ROCKET_GUY:
                                collided = check_aabb_collision(entities.positions[j], collision_size, entities.positions[i], guy_size);
								if (collided) {
									entities.health[j] -= 15;
									destroy_entity(i);
								}
                                break;
                            case ENTITY_TYPE_CHAINGUN_GUY:
                                collided = check_aabb_collision(entities.positions[j], collision_size, entities.positions[i], chaingun_guy_size);
								if (collided) {
									entities.health[j] -= 15;
									destroy_entity(i);
								}
                                break;
                        }
                    }

                }
                break;
            case ENTITY_TYPE_RIFLE_BULLET:
                if (entities.bullet_owned_by[i] == 1) {
                    collided = check_aabb_collision(entities.positions[0], collision_size, entities.positions[i], rifle_bullet_size);
                    if (collided) {
                        player_health -= 1;
                        destroy_entity(i);
                    }
                } else {
                    for (int j = 0; j < entities.entity_count; j++) {
                        switch (entities.entity_type[j]) {
                            case ENTITY_TYPE_KNIFE_GUY:
                            case ENTITY_TYPE_PISTOL_GUY:
                            case ENTITY_TYPE_RIFLE_GUY:
                            case ENTITY_TYPE_ROCKET_GUY:
                                collided = check_aabb_collision(entities.positions[j], collision_size, entities.positions[i], guy_size);
								if (collided) {
									entities.health[j] -= 25;
									destroy_entity(i);
								}
                                break;
                            case ENTITY_TYPE_CHAINGUN_GUY:
                                collided = check_aabb_collision(entities.positions[j], collision_size, entities.positions[i], chaingun_guy_size);
								if (collided) {
									entities.health[j] -= 25;
									destroy_entity(i);
								}
                                break;
                        }
                    }

                }
                break;
            case ENTITY_TYPE_ROCKET_BULLET:
                if (entities.bullet_owned_by[i] == 1) {
                    collided = check_aabb_collision(entities.positions[0], collision_size, entities.positions[i], rocket_bullet_size);
                    if (collided) {
                        player_health -= 1;
                        destroy_entity(i);
                        pal_play_sound(explosion_sound, 1);
                    }
                } else {
                    for (int j = 0; j < entities.entity_count; j++) {
                        switch (entities.entity_type[j]) {
                            case ENTITY_TYPE_KNIFE_GUY:
                            case ENTITY_TYPE_PISTOL_GUY:
                            case ENTITY_TYPE_RIFLE_GUY:
                            case ENTITY_TYPE_ROCKET_GUY:
                                collided = check_aabb_collision(entities.positions[j], collision_size, entities.positions[i], guy_size);
								if (collided) {
									entities.health[j] -= 200;
									destroy_entity(i);
                                    pal_play_sound(explosion_sound, 1);
								}
                                break;
                            case ENTITY_TYPE_CHAINGUN_GUY:
                                collided = check_aabb_collision(entities.positions[j], collision_size, entities.positions[i], chaingun_guy_size);
								if (collided) {
									entities.health[j] -= 200;
									destroy_entity(i);
                                    pal_play_sound(explosion_sound, 1);
								}
                                break;
                        }
                    }

                }
                break;
            case ENTITY_TYPE_CHAINGUN_BULLET:
                if (entities.bullet_owned_by[i] == 1) {
                    collided = check_aabb_collision(entities.positions[0], collision_size, entities.positions[i], chaingun_bullet_size);
                    if (collided) {
                        player_health -= 1;
                        destroy_entity(i);
                    }
                } else {
                    for (int j = 0; j < entities.entity_count; j++) {
                        switch (entities.entity_type[j]) {
                            case ENTITY_TYPE_KNIFE_GUY:
								collided = check_aabb_collision(entities.positions[j], collision_size, entities.positions[i], guy_size);
								if (collided) {
									entities.health[j] -= 25;
									destroy_entity(i);
								}

                                break;
                            case ENTITY_TYPE_PISTOL_GUY:
								collided = check_aabb_collision(entities.positions[j], collision_size, entities.positions[i], guy_size);
								if (collided) {
									entities.health[j] -= 25;
									destroy_entity(i);
								}
                                break;
                            case ENTITY_TYPE_RIFLE_GUY:
								collided = check_aabb_collision(entities.positions[j], collision_size, entities.positions[i], guy_size);
								if (collided) {
									entities.health[j] -= 25;
									destroy_entity(i);
								}
                                break;
                            case ENTITY_TYPE_ROCKET_GUY:
								collided = check_aabb_collision(entities.positions[j], collision_size, entities.positions[i], guy_size);
								if (collided) {
									entities.health[j] -= 25;
									destroy_entity(i);
								}
                                break;
                            case ENTITY_TYPE_CHAINGUN_GUY:
								collided = check_aabb_collision(entities.positions[j], collision_size, entities.positions[i], guy_size);
								if (collided) {
									entities.health[j] -= 25;
									destroy_entity(i);
								}
                                break;
                        }
                    }

                }
                break;
        }
    }

    for (int i = 0; i < entities.entity_count; i++) {
        switch (entities.entity_type[i]) {
			case ENTITY_TYPE_KNIFE_GUY:
			case ENTITY_TYPE_PISTOL_GUY:
			case ENTITY_TYPE_RIFLE_GUY:
			case ENTITY_TYPE_ROCKET_GUY:
			case ENTITY_TYPE_CHAINGUN_GUY:
                if (entities.health[i] <= 0) {
                    destroy_entity(i);
                    score += 1 + entities.entity_type[i];
                }
				break;
        }
    }
    //--------------------------------------------------
    // Rendering
    //--------------------------------------------------

    // Clear screen first
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw guys
    float cloud_scrl_fact = 0.20;
    float mountain_scrl_fact = 0.3;
    float tree_scrl_fact = 0.4;
    float ground_scrl_fact = 0.3; 
	// TODO @HACK: we make the scrolling factor of the ground match
    // that of the mountain as a hack to get scrolling to look good.
    // fix this by making the ground longer.

    // level shit
    vec2 cloud_pos_1 = {- entities.positions[0][0]  * cloud_scrl_fact , 0};
    vec2 mountain_pos_1 = {- entities.positions[0][0] * mountain_scrl_fact, 0};
    vec2 tree_pos_1 = {-entities.positions[0][0] * tree_scrl_fact , 0};
    // draw sky and sun first.
    draw_sprite(sky_texture, (vec2){0,0}, (vec2){640, 180}, no_rotation);
    draw_sprite(sun_texture, (vec2){-320 / 6, 20}, (vec2){320 / 6.5,320/ 6.5}, no_rotation);
    draw_sprite(scrolling_clouds_texture, cloud_pos_1, (vec2){640, 180}, no_rotation);
    draw_sprite(scrolling_mountains_texture, mountain_pos_1, (vec2){640,180}, no_rotation);
    draw_sprite(scrolling_trees_texture, tree_pos_1, (vec2){640,180}, no_rotation);
    
    vec2 cloud_pos_2;
    glm_vec2_add((vec2){1280, 0}, cloud_pos_1, cloud_pos_2);
    vec2 tree_pos_2;
    glm_vec2_add((vec2){1280, 0}, tree_pos_1, tree_pos_2);
    vec2 mountain_pos_2;
    glm_vec2_add((vec2){1280, 0}, mountain_pos_1, mountain_pos_2);

    draw_sprite(scrolling_clouds_texture, cloud_pos_2, (vec2){640, 180}, no_rotation);
    draw_sprite(scrolling_mountains_texture, mountain_pos_2, (vec2){640,180}, no_rotation);
    draw_sprite(scrolling_trees_texture, tree_pos_2, (vec2){640,180}, no_rotation);
   
    vec2 cloud_pos_3;
    glm_vec2_add((vec2){1280, 0}, cloud_pos_2, cloud_pos_3);
    vec2 tree_pos_3;
    glm_vec2_add((vec2){1280, 0}, tree_pos_2, tree_pos_3);
    vec2 mountain_pos_3;
    glm_vec2_add((vec2){1280, 0}, mountain_pos_2, mountain_pos_3);

    draw_sprite(scrolling_clouds_texture, cloud_pos_3, (vec2){640, 180}, no_rotation);
    draw_sprite(scrolling_mountains_texture, mountain_pos_3, (vec2){640,180}, no_rotation);
    draw_sprite(scrolling_trees_texture, tree_pos_3, (vec2){640,180}, no_rotation);

    vec3 cloud_pos_4;
    glm_vec3_add((vec3){1280, 0}, cloud_pos_3, cloud_pos_4);
    vec3 tree_pos_4;
    glm_vec3_add((vec3){1280, 0}, tree_pos_3, tree_pos_4);
    vec3 mountain_pos_4;
    glm_vec3_add((vec3){1280, 0}, mountain_pos_3, mountain_pos_4);

    draw_sprite(scrolling_clouds_texture, cloud_pos_4, (vec2){640, 180}, no_rotation);
    draw_sprite(scrolling_mountains_texture, mountain_pos_4, (vec2){640,180}, no_rotation);
    draw_sprite(scrolling_trees_texture, tree_pos_4, (vec2){640,180}, no_rotation);
    // draw ground last
    draw_sprite(ground_texture, (vec2){0,0}, (vec2){640, 180}, no_rotation);

    //player
    for (int i = 0; i < entities.entity_count; i++) {
        vec2 gun_pos = {0, 0};
        switch (entities.entity_type[i]) {

            case ENTITY_TYPE_PLAYER:
                vec2 chopper_pos = {entities.positions[i][0] - 19, entities.positions[i][1] + 62.5 };
				draw_sprite(player_texture, entities.positions[i], player_size, no_rotation);
				draw_sprite(chopper_texture, chopper_pos, chopper_size, no_rotation);
				if (currently_held_weapon == WEAPON_KNIFE) {
					glm_vec2_add(entities.positions[i], (vec2){-2, -10}, gun_pos);
					draw_sprite(knife_left_texture, gun_pos, knife_size, entities.rotations[i]);
				} else if (currently_held_weapon == WEAPON_PISTOL) {
					glm_vec2_add(entities.positions[i], (vec2){-2, -10}, gun_pos);
					draw_sprite(pistol_left_texture, gun_pos, pistol_size, entities.rotations[i]);
				
				} else if (currently_held_weapon == WEAPON_RIFLE) {
					glm_vec2_add(entities.positions[i], (vec2){-2, -10}, gun_pos);
					draw_sprite(rifle_left_texture, gun_pos, rifle_size, entities.rotations[i]);

				} else if (currently_held_weapon == WEAPON_CHAINGUN) {
					glm_vec2_add(entities.positions[i], (vec2){-2, -10}, gun_pos);
					draw_sprite(chaingun_left_texture, gun_pos, chaingun_size, entities.rotations[i]);
				
				} else if (currently_held_weapon == WEAPON_ROCKET_LAUNCHER) {
					glm_vec2_add(entities.positions[i], (vec2){-2, -10}, gun_pos);
					draw_sprite(rocket_launcher_left_texture, gun_pos, rocket_launcher_size, entities.rotations[i]);

				}
            break;

            case ENTITY_TYPE_KNIFE_GUY:
				glm_vec2_add(entities.positions[i], (vec2){-5, 2}, gun_pos);

				if (entities.facing_left[i]) {
				draw_sprite(knife_guy_left_texture, entities.positions[i], guy_size, no_rotation);
				draw_sprite(knife_left_texture, gun_pos, knife_size, entities.rotations[i]);
				} else {
				draw_sprite(knife_guy_right_texture, entities.positions[i], guy_size, no_rotation);
				draw_sprite(knife_right_texture, gun_pos, knife_size, entities.rotations[i]);
				
				}
                break;

            case ENTITY_TYPE_PISTOL_GUY:
				glm_vec2_add(entities.positions[i], (vec2){-8, -1}, gun_pos);

				if (entities.facing_left[i]) {
					draw_sprite(pistol_guy_left_texture, entities.positions[i], guy_size, no_rotation);
					draw_sprite(pistol_left_texture, gun_pos, pistol_size, entities.rotations[i]);
				} else {
					draw_sprite(pistol_guy_right_texture, entities.positions[i], guy_size, no_rotation);
					draw_sprite(pistol_right_texture, gun_pos, pistol_size, entities.rotations[i]);
				
				}
                break;

            case ENTITY_TYPE_RIFLE_GUY:
				glm_vec2_add(entities.positions[i], (vec2){-5, 1}, gun_pos);
				if (entities.facing_left[i]) {
					draw_sprite(rifle_guy_left_texture, entities.positions[i], guy_size, no_rotation);
					draw_sprite(rifle_left_texture, gun_pos, rifle_size, entities.rotations[i]);
				} else {
					draw_sprite(rifle_guy_right_texture, entities.positions[i], guy_size, no_rotation);
					draw_sprite(rifle_right_texture, gun_pos, rifle_size, entities.rotations[i]);
				
				}
                break;

            case ENTITY_TYPE_ROCKET_GUY:
				glm_vec2_add(entities.positions[i], (vec2){-3, 1}, gun_pos);

				if (entities.facing_left[i]) {
				draw_sprite(rocket_guy_left_texture, entities.positions[i], guy_size, no_rotation);
				draw_sprite(rocket_launcher_left_texture, gun_pos, rocket_launcher_size, entities.rotations[i]);
				} else {
				draw_sprite(rocket_guy_right_texture, entities.positions[i], guy_size, no_rotation);
				draw_sprite(rocket_launcher_right_texture, gun_pos, rocket_launcher_size, entities.rotations[i]);
				
				}
                break;

            case ENTITY_TYPE_CHAINGUN_GUY:
				glm_vec2_add(entities.positions[i], (vec2){0, -3}, gun_pos);

				if (entities.facing_left[i]) {
					draw_sprite(chaingun_guy_left_texture, entities.positions[i], chaingun_guy_size, no_rotation);
					draw_sprite(chaingun_left_texture, gun_pos, chaingun_size, entities.rotations[i]);
				} else {
					draw_sprite(chaingun_guy_right_texture, entities.positions[i], chaingun_guy_size, no_rotation);
					draw_sprite(chaingun_right_texture, gun_pos, chaingun_size, entities.rotations[i]);

				}
                break;

            case ENTITY_TYPE_KNIFE_BULLET:
                draw_sprite(knife_bullet_texture, entities.positions[i], knife_bullet_size, entities.rotations[i]);
                break;
            case ENTITY_TYPE_PISTOL_BULLET:
					draw_sprite(pistol_bullet_texture, entities.positions[i], pistol_bullet_size, entities.rotations[i]);
                break;
            case ENTITY_TYPE_RIFLE_BULLET:
					draw_sprite(rifle_bullet_texture, entities.positions[i], rifle_bullet_size, entities.rotations[i]);
                break;
            case ENTITY_TYPE_CHAINGUN_BULLET:
					draw_sprite(chaingun_bullet_texture, entities.positions[i], chaingun_bullet_size, entities.rotations[i]);
                break;
            case ENTITY_TYPE_ROCKET_BULLET:
					draw_sprite(rocket_bullet_texture, entities.positions[i], rocket_bullet_size, entities.rotations[i]);
                break;
            case ENTITY_TYPE_HEALTH_PACK:
				vec2 health_pack_size = {10,10};
                draw_sprite(health_pack_texture, entities.positions[i], health_pack_size, no_rotation);
                break;
        }
    }

    // text
    rgb_color player_health_color = lerp_red_green_100(player_health);
    const char player_health_string[24] = "HP:";
    const char player_health_number_string[24];

    myitoa(player_health, player_health_number_string);
    strcat(player_health_string, player_health_number_string);
    draw_text(player_health_string, -300, 140, 16, 6, (vec3){player_health_color.r, player_health_color.g, 0});
    if (get_back) {
    
    draw_text("GET BACK TO THE FRONT LINES", -300, 0, 16, 6, (vec3){1, 1, 1});
    }
    const char score_string[24] = "SCORE:";
    const char score_number_string[8];
    myitoa(score, score_number_string);
    strcat(score_string, score_number_string);
    draw_text(score_string, -300, -155, 15, 6, (vec3){1, 1, 1});
    // Swap buffers
    pal_swap_buffers(window);
}

int main() {

    pal_init();

    pal_monitor* monitor = pal_get_primary_monitor();
    pal_video_mode* mode = pal_get_video_mode(monitor);
    pal_bool is_fullscreen = pal_false;
    pal_window* window = pal_create_window(SCREEN_WIDTH,SCREEN_HEIGHT, "Window Title", 0);
    pal_make_context_current(window);
    if (!gladLoadGLLoader((GLADloadproc)pal_gl_get_proc_address)) {
        fprintf(stderr, "ERROR: Failed to initialize glad!\n");
    }


    pal_set_window_icon_legacy(window, "textures/icon.png");
    pal_set_taskbar_icon(window, "textures/icon.png");
    pal_set_cursor(window, "textures/crosshair_32_by_32.png", 32, 16, 16);
    pal_gamepad_state state;
    uint8_t running = pal_true;
    pal_event event;
    pal_swap_interval(1);
    while (running) {
        current_time = pal_get_time_since_pal_started();
        delta_time = current_time - last_frame_time;
        while (pal_poll_events(&event)) {

            switch (event.type) {

                case PAL_EVENT_MOUSE_BUTTON_DOWN:
                    break;
                case PAL_EVENT_MOUSE_BUTTON_UP:
                    break;
                case PAL_EVENT_KEY_DOWN:
                    break;
                case PAL_EVENT_KEY_UP:
                    break;
                case PAL_EVENT_QUIT:
                    printf("Window closed");
                    running = pal_false;
                    break;
                case PAL_EVENT_WINDOW_CLOSE_REQUESTED:
                    printf("Window closed");
                    running = pal_false;
                    break;
                case PAL_EVENT_MOUSE_MOTION:
                    //fix event.motion.x not returning in the library.
                    if (event.motion.x > 0 && event.motion.y > 0) {
						mouse_position[0] = event.motion.x;
						mouse_position[1] = event.motion.y;
                    }
                    break;
                case PAL_EVENT_WINDOW_LOST_FOCUS:
                    if (is_fullscreen) {
						pal_set_video_mode(NULL);
						pal_minimize_window(window);
                    }
                    break;
                case PAL_EVENT_WINDOW_GAINED_FOCUS:
                    if (is_fullscreen) {
						pal_make_window_fullscreen(window);
                    }
                    break;
                default:
                    // printf("%d\n", event.type);
                    break;
            }
        }
        
        game_update_and_render(window);
        last_frame_time = current_time;
    }
    pal_shutdown();
    return 0;
}
