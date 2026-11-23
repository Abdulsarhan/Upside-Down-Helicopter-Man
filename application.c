#include <stdio.h>
#include <assert.h>

#include <Windows.h>
#include <stdint.h>
#include "glad/glad.h"
#include "cglm/cglm.h"
#include "application.h"
#include "pal.h"
#include "stb_image.h"
#define ARENA_IMPLEMENTATION
#include "arena.h"
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

#define PI 3.1415f

void check_compile_errors(u32 object, const char* type) {
    i32 success;
    char info_log[1024];
    
    if (strcmp(type, "PROGRAM") != 0) {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, 1024, NULL, info_log);
            fprintf(stderr, "| ERROR::SHADER: Compile-time error: Type: %s\n%s\n -- --------------------------------------------------- --\n", 
                    type, info_log);
        }
    }
    else {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(object, 1024, NULL, info_log);
            fprintf(stderr, "| ERROR::Shader: Link-time error: Type: %s\n%s\n -- --------------------------------------------------- --\n", 
                    type, info_log);
        }
    }
}

u32 load_shader(const char* v_shader_file, const char* f_shader_file) {
    /* Read shader source code from files */
    size_t vshader_file_size = 0;
    size_t fshader_file_size = 0;
    char* vert_file = pal_read_entire_file(v_shader_file, &vshader_file_size);
    char* frag_file = pal_read_entire_file(f_shader_file, &fshader_file_size);

    // null checking.
    if (!vert_file || !frag_file) {
        fprintf(stderr, "ERROR::SHADER: Failed to read shader files\n");
        if (vert_file) free(vert_file);
        if (frag_file) free(frag_file);
        return 0;
    }
    
    u32 vertex_shader = 0, fragment_shader = 0;
    
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vert_file, NULL);
    glCompileShader(vertex_shader);
    check_compile_errors(vertex_shader, "VERTEX");
    
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &frag_file, NULL);
    glCompileShader(fragment_shader);
    check_compile_errors(fragment_shader, "FRAGMENT");
    
    u32 shader = glCreateProgram();
    glAttachShader(shader, vertex_shader);
    glAttachShader(shader, fragment_shader);
    glLinkProgram(shader);
    check_compile_errors(shader, "PROGRAM");
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    free(vert_file);
    free(frag_file);
    
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
static GLuint health_pack_texture;
static GLuint ground_texture, scrolling_clouds_texture, scrolling_trees_texture, scrolling_mountains_texture, sun_texture, sky_texture;
static GLuint rifle_bullet_texture, pistol_bullet_texture, knife_bullet_texture, chaingun_bullet_texture, rocket_bullet_texture;

static GLuint weapon_left_textures[5];
static GLuint weapon_right_textures[5];

#define ASCII_MAX 128
static GLuint bitmap_font[ASCII_MAX]; // indices 0 - 9 corresponds to numbers 0 - 9 everything after that is lowercase letters.
static i32 first_run = 1;
static vec2 mouse_position = {0, 0};
static f32 current_time = 0;
static f32 last_frame_time = 0;
static f32 delta_time = 0;

static pal_sound* explosion_sound;
static pal_sound* chaingun_revving;
static pal_sound* chaingun_revving_two;

// fire times for enemies
f32 spawn_timer = 0;

// score tracking.
i32 score = 0;
// Gun sizes.

static const vec2 gun_sizes[5] = {
    {9.0f, 3.0f},
    {3.0f, 3.0f},
    {9.0f, 3.0f},
    {18.0f, 9.0f},
    {12.0f, 3.0f},
};
static const vec2 bullet_sizes[5] = {
    {9.0f, 9.0f},
    {3.0f, 3.0f},
    {6.0f, 2.0f},
    {9.0f, 3.0f},
    {24.0f, 24.0f},
};

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

static int currently_held_weapon = 0;

static const f32 no_rotation = 0.0f;

static vec4 mouse_pos_world = {0,0,0,0};
#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

u64 g_next_entt_id = 0;

typedef struct Player {
    vec2 size; // size of the entity in pixels
    u64 id; // unique identifier of the entity.
    i32 health; // health of the enemy or player. Does not apply to bullets since they die immediately.
	vec2 position; // position of the guy or the bullet
    f32 rotation; // rotation of the bullet. Should be 0 for the guys.
}Player;

Player player;

typedef struct pool_of_guys {
    i32 count;
    vec2 size; // size of the entity in pixels
    GLuint right_facing_texture;
    GLuint left_facing_texture;
    i32 starting_health;
    float timer; // the last time that an npc has fired it's gun.
    u64* id; // unique identifier of the entity.
    i32* health; // health of the enemy or player. Does not apply to bullets since they die immediately.
	vec2* positions; // position of the guy or the bullet
    f32* rotations;
    pal_bool* facing_left; // Specifies which way the guy facing. Left or right?
}pool_of_guys;

enum {
    KNIFE_GUY,
    PISTOL_GUY,
    RIFLE_GUY,
    CHAINGUN_GUY,
    ROCKET_GUY,
    ENEMY_TYPE_COUNT
};
pool_of_guys guys[ENEMY_TYPE_COUNT];

typedef struct pool_of_bullets {
    i32 count;
    vec2 size; // size of the entity in pixels
    i32 damage; // How much damage does it do?
    GLuint texture;
    u64* id; // unique identifier of the entity.
	vec2* positions; // position of the guy or the bullet
	vec2* velocities;// position of the guy or the bullet
    f32* rotations; // rotation of the bullet. Should be 0 for the guys.
}pool_of_bullets;

enum {
    KNIFE_BULLET = KNIFE_GUY,
    PISTOL_BULLET = PISTOL_GUY,
    RIFLE_BULLET = RIFLE_GUY,
    CHAINGUN_BULLET = CHAINGUN_GUY,
    ROCKET_BULLET = ROCKET_GUY,
    BULLET_TYPE_COUNT
};


pool_of_bullets p_bullets[BULLET_TYPE_COUNT];
pool_of_bullets e_bullets[BULLET_TYPE_COUNT];

typedef enum {
	WEAPON_KNIFE = KNIFE_GUY,
	WEAPON_PISTOL = PISTOL_GUY,
	WEAPON_RIFLE = RIFLE_GUY,
	WEAPON_CHAINGUN = CHAINGUN_GUY,
	WEAPON_ROCKET = ROCKET_GUY,
	WEAPON_TYPE_COUNT
}weapon_type;

typedef struct weapon_info{
    f32 fire_time;
    f32 fire_rate;
    f32 gun_volume;
    f32 bullet_speed;
    pal_sound* sound;
}weapon_info;
weapon_info p_weapons[WEAPON_TYPE_COUNT];

#define ENTITY_LIMIT 500

mat4 view;
mat4 proj;

static void draw_sprite(GLuint texture, vec2 position, const vec2 size, f32 rotation) {
    // Model matrix
    mat4 model;
    glm_mat4_identity(model);
    vec3 pos = {position[0], position[1], 0.0f};
    glm_translate(model, pos);
    
    vec3 z_axis = {0.0f, 0.0f, 1.0f};
    glm_rotate(model, rotation, z_axis);
    
    vec3 size_vec3 = {size[0], size[1], 1.0f};
    glm_scale(model, size_vec3);

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

i32 check_aabb_collision(vec2 one_pos, const vec2 one_size, vec2 two_pos, const vec2 two_size) {
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
    target_pos_clip_space[0] = (mouse_position[0] / WINDOW_WIDTH) * 2.0f - 1.0f;
    target_pos_clip_space[1] = -((mouse_position[1] / WINDOW_HEIGHT) * 2.0f - 1.0f);
    target_pos_clip_space[2] = 0.0f;
    target_pos_clip_space[3] = 1.0f;

    mat4 inverse_pv = {0};
    glm_mat4_inv(pv, inverse_pv);
    glm_mat4_mulv(inverse_pv,  target_pos_clip_space, mouse_pos_world);
}

void adjust_player_gun_rotations(Player* player) {
	vec2 direction = {0, 0};
	f32 rotation = 0;
	vec2 player_gun_pos;
	glm_vec2_add(player->position, (vec2){15, -36}, player_gun_pos);
	glm_vec2_sub(mouse_pos_world, player_gun_pos, direction);
	glm_vec2_normalize(direction);

	rotation = atan2f(direction[1], direction[0]); // y, x
	player->rotation = rotation - PI;         // the player is upside-down.
}

void adjust_npc_gun_rotations(Player player, pool_of_guys* guys) {
	vec2 direction = {0, 0};
	f32 rotation = 0;
	i32 count = guys->count;
	
    for (int i = 0; i < count; i++) {
					
		glm_vec2_sub(player.position, guys->positions[i], direction);
		if (direction[0] < 0) {
		rotation = atan2f(direction[1], direction[0]); // y, x
			guys->facing_left[i] = pal_true;
		} else {
		rotation = atan2f(direction[1], direction[0]); // y, x
			guys->facing_left[i] = pal_false;
		}
		if (guys->facing_left[i] == pal_true) {
			guys->rotations[i] = rotation - PI; // the player is upside-down.
		} else {
			guys->rotations[i] = rotation; // the player is upside-down.
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

    i32 len = (i32)(tp - tmp);

    if (is_negative) {
        *string++ = '-';
        len++;
    }

    while (tp > tmp)
        *string++ = *--tp;

    return len;
}

int create_guy(vec2 position, pal_bool facing_left, i32 health, pool_of_guys* guys) {
    int guy = guys->count;
    guys->count++;
    guys->id[guy] = g_next_entt_id;
    g_next_entt_id++;

    memcpy(guys->positions[guy], position, sizeof(vec2));
    guys->rotations[guy] = 0;
    guys->facing_left[guy] = facing_left;
	guys->health[guy] = health;
    assert(guy < 500);
    return guy;

}

int create_bullet(vec2 position, pool_of_bullets* bullets) {
    int bullet = bullets->count;
    bullets->count++;
    bullets->id[bullet] = g_next_entt_id;
    g_next_entt_id++;
    memset(bullets->velocities[bullet],0, sizeof(vec2));

    glm_vec2_add(position, (vec2){0, 0}, bullets->positions[bullet]);
    bullets->rotations[bullet] = 0;
    return bullet;
}

static void destroy_guy(int guy_index, pool_of_guys* guys) {
    i32 last_entity = guys->count - 1;

    if (guy_index != last_entity) {
        guys->id[guy_index] = guys->id[last_entity];
        guys->health[guy_index] = guys->health[last_entity];
        guys->positions[guy_index][0] = guys->positions[last_entity][0];
        guys->positions[guy_index][1] = guys->positions[last_entity][1];
        guys->rotations[guy_index] = guys->rotations[last_entity];
        guys->facing_left[guy_index] = guys->facing_left[last_entity];
    }

    guys->count--;
}

static void destroy_bullet(int bullet_index, pool_of_bullets* bullets) {
    i32 last_entity = bullets->count - 1;

    if (bullet_index != last_entity) {
        bullets->id[bullet_index] = bullets->id[last_entity];
        bullets->positions[bullet_index][0] = bullets->positions[last_entity][0];
        bullets->positions[bullet_index][1] = bullets->positions[last_entity][1];
        bullets->velocities[bullet_index][0] = bullets->velocities[last_entity][0];
        bullets->velocities[bullet_index][1] = bullets->velocities[last_entity][1];
        bullets->rotations[bullet_index] = bullets->rotations[last_entity];
    }

    bullets->count--;
}

static void reset_pool_of_guys(pool_of_guys* guys) {
    i32 entity_count = guys->count;
    guys->count = 0;
    memset(guys->id,0,sizeof(u64) * entity_count);
    memset(guys->health,0,sizeof(i32) * entity_count);
    memset(guys->positions,0,sizeof(vec2) * entity_count);
    memset(guys->rotations,0,sizeof(f32) * entity_count);
    memset(guys->facing_left,0,sizeof(i32) * entity_count);
}

static void reset_pool_of_bullets(pool_of_bullets* bullets) {
    i32 entity_count = bullets->count;
    bullets->count = 0;
    memset(bullets->id,0,sizeof(u64) * entity_count);
    memset(bullets->positions,0,sizeof(vec2) * entity_count);
    memset(bullets->rotations,0,sizeof(f32) * entity_count);
    memset(bullets->velocities,0,sizeof(vec2) * entity_count);
}
static void handle_player_bullet_collisions(pool_of_bullets* bullets, pool_of_guys* groups, size_t group_count) {
    int i = 0;

    while (i < bullets->count) {
        pal_bool destroyed = pal_false;

        for (size_t g = 0; g < group_count; g++) {
            pool_of_guys current_group = groups[g];

            for (int e = 0; e < current_group.count; e++) {
                if (check_aabb_collision(current_group.positions[e], guy_size,
                                         bullets->positions[i], bullets->size)) {
                    current_group.health[e] -= 200;
                    destroy_bullet(i, bullets);  // now modifies real data
                    destroyed = pal_true;
                    goto next_bullet;
                }
            }
        }

    next_bullet:
        if (!destroyed) {
            i++;
        }
    }
}
static pal_bool is_fullscreen = pal_false;

static void fire_bullet(pool_of_bullets* bullets, weapon_info* info) {
	vec2 direction = {0, 0};
	int bullet = 0;

	if (info->fire_time < (current_time - info->fire_rate) || info->fire_time == 0) {
		if (info->sound) {
			pal_play_sound(info->sound, info->gun_volume);
		}
		int bullet = create_bullet(player.position, bullets);
		glm_vec2_sub((vec2){mouse_pos_world[0], mouse_pos_world[1]}, player.position, direction);
		glm_vec2_normalize(direction);
		bullets->rotations[bullet] = atan2f(direction[1], direction[0]);
		bullets->velocities[bullet][0] = direction[0] * info->bullet_speed;
		bullets->velocities[bullet][1] = direction[1] * info->bullet_speed;
		info->fire_time = current_time;
	}
}

static void knife_attack(void) {
	pool_of_bullets* bullets = &p_bullets[KNIFE_BULLET];
    weapon_info* info = &p_weapons[WEAPON_KNIFE];
    fire_bullet(bullets, info);
}

static void pistol_attack(void) {
	pool_of_bullets* bullets = &p_bullets[PISTOL_BULLET];
    weapon_info* info = &p_weapons[WEAPON_PISTOL];
    fire_bullet(bullets, info);
}

static void rifle_attack(void) {
	pool_of_bullets* bullets = &p_bullets[RIFLE_BULLET];
    weapon_info* info = &p_weapons[WEAPON_RIFLE];
    fire_bullet(bullets, info);
}

static void chaingun_attack(void) {
	pool_of_bullets* bullets = &p_bullets[CHAINGUN_BULLET];
    weapon_info* info = &p_weapons[WEAPON_CHAINGUN];
    fire_bullet(bullets, info);
}

static void rocket_attack(void) {
	pool_of_bullets* bullets = &p_bullets[ROCKET_BULLET];
    weapon_info* info = &p_weapons[WEAPON_ROCKET];
    fire_bullet(bullets, info);
}

typedef void (*attack_fn)(void);
attack_fn attack[] = {
	knife_attack,
    pistol_attack,
    rifle_attack,
    chaingun_attack,
    rocket_attack
};

void init_player_weapon(weapon_type type, f32 fire_rate, f32 gun_volume, f32 bullet_speed, pal_sound* sound) {
		p_weapons[type].bullet_speed = bullet_speed;
		p_weapons[type].fire_rate = fire_rate;
		p_weapons[type].fire_time = 0.0f;
		p_weapons[type].gun_volume = gun_volume;
        p_weapons[type].sound = sound;
}

static void game_update_and_render(pal_window* window) {
    // Initialize OpenGL objects on first run
    if (first_run) {
        // load sounds
		explosion_sound = pal_load_sound("sounds/explosion.ogg");
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
        guys[KNIFE_GUY].left_facing_texture = load_texture("textures/knife_guy_left.png");
        guys[PISTOL_GUY].left_facing_texture = load_texture("textures/pistol_guy_left.png");
        guys[RIFLE_GUY].left_facing_texture = load_texture("textures/rifle_guy_left.png");
        guys[CHAINGUN_GUY].left_facing_texture = load_texture("textures/chaingun_guy_left.png");
        guys[ROCKET_GUY].left_facing_texture = load_texture("textures/rocket_guy_left.png");

        // Enemies facing right
        guys[KNIFE_GUY].right_facing_texture = load_texture("textures/knife_guy_right.png");
        guys[PISTOL_GUY].right_facing_texture = load_texture("textures/pistol_guy_right.png");
        guys[RIFLE_GUY].right_facing_texture = load_texture("textures/rifle_guy_right.png");
        guys[CHAINGUN_GUY].right_facing_texture = load_texture("textures/chaingun_guy_right.png");
        guys[ROCKET_GUY].right_facing_texture = load_texture("textures/rocket_guy_right.png");
        // Bullets
        e_bullets[KNIFE_BULLET].texture = load_texture("textures/knife_bullet.png"); 
        e_bullets[PISTOL_BULLET].texture = load_texture("textures/rifle_bullet.png"); 
        e_bullets[RIFLE_BULLET].texture = e_bullets[PISTOL_BULLET].texture;                               
        e_bullets[CHAINGUN_BULLET].texture = load_texture("textures/chaingun_bullet.png");                               
        e_bullets[ROCKET_BULLET].texture = load_texture("textures/rocket_bullet.png");                               

        p_bullets[KNIFE_BULLET].texture = e_bullets[KNIFE_BULLET].texture; 
        p_bullets[PISTOL_BULLET].texture = e_bullets[PISTOL_BULLET].texture;
        p_bullets[RIFLE_BULLET].texture = e_bullets[RIFLE_BULLET].texture;
        p_bullets[CHAINGUN_BULLET].texture = e_bullets[CHAINGUN_BULLET].texture;
        p_bullets[ROCKET_BULLET].texture = e_bullets[ROCKET_BULLET].texture;

        // Weapons facing right
        weapon_right_textures[0] = load_texture("textures/knife_right.png");
        weapon_right_textures[1] = load_texture("textures/pistol_right.png");
        weapon_right_textures[2] = load_texture("textures/rifle_right.png");
        weapon_right_textures[3] = load_texture("textures/chaingun_right.png");
        weapon_right_textures[4] = load_texture("textures/rocket_launcher_right.png");
        // Weapons facing left
        weapon_left_textures[0] = load_texture("textures/knife_left.png");
        weapon_left_textures[1] = load_texture("textures/pistol_left.png");
        weapon_left_textures[2] = load_texture("textures/rifle_left.png");
        weapon_left_textures[3] = load_texture("textures/chaingun_left.png");
        weapon_left_textures[4] = load_texture("textures/rocket_launcher_left.png");
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


        // player
        player.health = 100;
        player.id = g_next_entt_id;
        g_next_entt_id++;
        player.position[0] = 0;
        player.position[1] = -110;
        player.rotation = 0;
        Arena arena = init_arena(1024 * 1024 * 64);

        // enemies
        for (int i = 0; i < ENEMY_TYPE_COUNT; i++) {
			guys[i].id = arena_alloc(&arena, sizeof(u64) * ENTITY_LIMIT);
			guys[i].positions = arena_alloc(&arena, sizeof(vec2) * ENTITY_LIMIT);
			guys[i].facing_left = arena_alloc(&arena, sizeof(pal_bool) * ENTITY_LIMIT);
			guys[i].rotations = arena_alloc(&arena, sizeof(f32) * ENTITY_LIMIT);
			guys[i].health = arena_alloc(&arena, sizeof(i32) * ENTITY_LIMIT);
			guys[i].size[0] = guy_size[0];
			guys[i].size[1] = guy_size[1];
            guys[i].timer = 0;
            guys[i].starting_health = 100;
        }

        // bullets
        for (int i = 0; i < BULLET_TYPE_COUNT; i++) {
            p_bullets[i].id = arena_alloc(&arena, sizeof(u64) * ENTITY_LIMIT);
            p_bullets[i].rotations = arena_alloc(&arena, sizeof(f32) * ENTITY_LIMIT);
            p_bullets[i].positions = arena_alloc(&arena, sizeof(vec2) * ENTITY_LIMIT);
            p_bullets[i].velocities = arena_alloc(&arena, sizeof(vec2) * ENTITY_LIMIT);
            p_bullets[i].size[0] = bullet_sizes[i][0];
            p_bullets[i].size[1] = bullet_sizes[i][1];
        }
        for (int i = 0; i < BULLET_TYPE_COUNT; i++) {
            e_bullets[i].id = arena_alloc(&arena, sizeof(u64) * ENTITY_LIMIT);
            e_bullets[i].rotations = arena_alloc(&arena, sizeof(f32) * ENTITY_LIMIT);
            e_bullets[i].positions = arena_alloc(&arena, sizeof(vec2) * ENTITY_LIMIT);
            e_bullets[i].velocities = arena_alloc(&arena, sizeof(vec2) * ENTITY_LIMIT);
            e_bullets[i].size[0] = bullet_sizes[i][0];
            e_bullets[i].size[1] = bullet_sizes[i][1];
        }

        pal_sound* gun_firing = pal_load_sound("sounds/gun_firing.ogg");
        pal_sound* rocket_firing = pal_load_sound("sounds/rocket_firing.ogg");
        init_player_weapon(WEAPON_KNIFE, 0.5f, 1.0f , knife_bullet_speed, NULL);
        init_player_weapon(WEAPON_PISTOL, 0.3f, 1.0f , pistol_bullet_speed, gun_firing);
        init_player_weapon(WEAPON_RIFLE, 0.2f, 1.0f , rifle_bullet_speed, gun_firing);
        init_player_weapon(WEAPON_CHAINGUN, 0.05f, 1.0f , chaingun_bullet_speed, gun_firing);
        init_player_weapon(WEAPON_ROCKET, 0.5f, 0.3f, rocket_bullet_speed, rocket_firing);

        first_run = 0;
    }
    //--------------------------------------------------
    // Renderer Stuff that Gamplay Code Relies on
    //--------------------------------------------------
    // Set up matrices

    // View matrix
    glm_mat4_identity(view);
    f32 camera_x = player.position[0] + 50; 
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
    // Randomly Spawning Entities
    //--------------------------------------------------
	uint64_t state = 0;
	pal_srand(&state, (uint64_t)current_time);
	uint32_t location = pal_rand(&state) % 100;
	uint32_t enemy_type = pal_rand(&state) % 5; 
	assert(enemy_type <= ENEMY_TYPE_COUNT - 1);

	if (spawn_timer < (current_time - 0.5)) {
		vec2 spawn_pos = {
			player.position[0] + abs(location) + 200,
			-110
    };

    switch (enemy_type) {
        case 0:
            create_guy(spawn_pos, 0, guys[KNIFE_GUY].starting_health, &guys[KNIFE_GUY]);
            break;
        case 1:
            create_guy(spawn_pos, 0, guys[PISTOL_GUY].starting_health, &guys[PISTOL_GUY]);
            break;
        case 2:
            create_guy(spawn_pos, 0, guys[RIFLE_GUY].starting_health, &guys[RIFLE_GUY]);
            break;
        case 3:
            create_guy(spawn_pos, 0, guys[CHAINGUN_GUY].starting_health, &guys[CHAINGUN_GUY]);
            break;
        case 4:
            create_guy(spawn_pos, 0, guys[ROCKET_GUY].starting_health, &guys[ROCKET_GUY]);
            break;
		}

		spawn_timer = current_time;
	}

    //--------------------------------------------------
    // Player movement
    //--------------------------------------------------
    const f32 player_speed = 400.0f;
    const f32 half_of_sqrt_of_two = 0.707107f;
    vec2 move_delta = {0};
    pal_bool get_back = 0;
    move_delta[1] += player_speed * pal_is_key_down(-1, PAL_W);
    move_delta[1] -= player_speed * pal_is_key_down(-1, PAL_S);
    move_delta[0] -= player_speed * pal_is_key_down(-1, PAL_A);
    move_delta[0] += player_speed * pal_is_key_down(-1, PAL_D);

    if (move_delta[0] != 0 && move_delta[1] != 0) {
        move_delta[0] *= half_of_sqrt_of_two;
        move_delta[1] *= half_of_sqrt_of_two;
    }
	player.position[0] += move_delta[0] * delta_time;
	player.position[1] += move_delta[1] * delta_time;

    // This ensures that the player cannot go too far back.
    if (player.position[0] <= -750) {
        get_back = 1;
    }

    // This ensures that the player cannot go too high off the screen.
    if (player.position[1] >= 160) {
        player.position[1] = 160.0f;
    }

    // This ensures that the player cannot go through the ground.
    if (player.position[1] <= -90.0f) {
        player.position[1] = -90.0f;
    }
   //--------------------------------------------------
   // Player weapons & shooting
   //--------------------------------------------------

    get_mouse_pos_world();
    adjust_player_gun_rotations(&player);
    adjust_npc_gun_rotations(player, &guys[KNIFE_GUY]);
    adjust_npc_gun_rotations(player, &guys[PISTOL_GUY]);
    adjust_npc_gun_rotations(player, &guys[RIFLE_GUY]);
    adjust_npc_gun_rotations(player, &guys[CHAINGUN_GUY]);
    adjust_npc_gun_rotations(player, &guys[ROCKET_GUY]);

    if (pal_is_key_pressed(-1, PAL_1)) {
        currently_held_weapon = WEAPON_KNIFE;
    }
    if (pal_is_key_pressed(-1, PAL_2)) {
        currently_held_weapon = WEAPON_PISTOL;
    }
    if (pal_is_key_pressed(-1, PAL_3)) {
        currently_held_weapon = WEAPON_RIFLE;
    }
    if (pal_is_key_pressed(-1, PAL_4)) {
        currently_held_weapon = WEAPON_CHAINGUN;
    }
    if (pal_is_key_pressed(-1, PAL_5)) {
        currently_held_weapon = WEAPON_ROCKET;
    }
    if (pal_is_mouse_down(-1, PAL_MOUSE_LEFT)) {
        assert(currently_held_weapon >= 0 && currently_held_weapon < WEAPON_TYPE_COUNT);
        attack[currently_held_weapon]();
    }

    if (!pal_is_mouse_down(-1, PAL_MOUSE_LEFT)) {
        pal_stop_sound(chaingun_revving_two);
    }
    //--------------------------------------------------
    // Enemy Bullets
    //--------------------------------------------------
    {
        int bullet = 0;
        vec2 direction = {0, 0};

        for (int i = 0; i < ENEMY_TYPE_COUNT; i++) {
			if (guys[i].timer < (current_time - 0.5)) {
				for (int j = 0; j < guys[i].count; j++) {
					bullet = create_bullet(guys[i].positions[j], &e_bullets[i]);
					glm_vec2_sub(player.position, guys[i].positions[j], direction);
					glm_vec2_normalize(direction);
					e_bullets[i].rotations[bullet] = atan2f(direction[1], direction[0]);
					e_bullets[i].velocities[bullet][0] = direction[0] * knife_bullet_speed;
					e_bullets[i].velocities[bullet][1] = direction[1] * knife_bullet_speed;
					guys[i].timer = current_time;
				}
			}
        }
    }
    //--------------------------------------------------
    // Making Bullets Move every frame
    //--------------------------------------------------
    for (int i = 0; i < BULLET_TYPE_COUNT; i++) {
        for (int j = 0; j < p_bullets[i].count; j++) {
			glm_vec2_add(p_bullets[i].positions[j], (vec2){p_bullets[i].velocities[j][0] * delta_time, p_bullets[i].velocities[j][1] * delta_time}, p_bullets[i].positions[j]);
        }
    }

    for (int i = 0; i < BULLET_TYPE_COUNT; i++) {
        for (int j = 0; j < e_bullets[i].count; j++) {
			glm_vec2_add(e_bullets[i].positions[j], (vec2){e_bullets[i].velocities[j][0] * delta_time, e_bullets[i].velocities[j][1] * delta_time}, e_bullets[i].positions[j]);
        }
    }

    //--------------------------------------------------
    // Bullet Collisions
    //--------------------------------------------------
    for (int i = 0; i < BULLET_TYPE_COUNT; i++) {
        for (int j = 0; j < e_bullets[i].count; j++) {
        
        pal_bool destroyed = pal_false;
		if (check_aabb_collision(player.position, player_size, e_bullets[i].positions[j], e_bullets[i].size)) {
			player.health -= 1;
			destroy_bullet(j, &e_bullets[i]);
            destroyed = pal_true;
		}
        if(!destroyed) j++;
        }
    }

    handle_player_bullet_collisions(&p_bullets[KNIFE_BULLET], guys, 5);
    handle_player_bullet_collisions(&p_bullets[PISTOL_BULLET], guys, 5);
    handle_player_bullet_collisions(&p_bullets[RIFLE_BULLET], guys, 5);
    handle_player_bullet_collisions(&p_bullets[CHAINGUN_BULLET], guys, 5);
    handle_player_bullet_collisions(&p_bullets[ROCKET_BULLET], guys, 5);

    // -----------------------------------------------
    // Kill Enemies if health is 0 or less.
    // -----------------------------------------------
    for (int i = 0; i < ENEMY_TYPE_COUNT; i++) {
        for (int j = 0; j < guys[i].count; j++) {
        
			if (guys[i].health[j] <= 0) {
				destroy_guy(j, &guys[i]);
				score += 1;
			}
        }
    }

    //--------------------------------------------------
    // Rendering
    //--------------------------------------------------

    // Clear screen first
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float cloud_scrl_fact = 0.20f;
    float mountain_scrl_fact = 0.3f;
    float tree_scrl_fact = 0.4f;

    vec2 cloud_pos_1 = { -player.position[0]  * cloud_scrl_fact , 0};
    vec2 mountain_pos_1 = { -player.position[0] * mountain_scrl_fact, 0};
    vec2 tree_pos_1 = { -player.position[0] * tree_scrl_fact , 0};
    // draw sky and sun first.
    draw_sprite(sky_texture, (vec2){0,0}, (vec2){640.0f, 180.0f}, no_rotation);
    draw_sprite(sun_texture, (vec2){-320 / 6, 20}, (vec2){320.0f / 6.5f,320.0f/ 6.5f}, no_rotation);
    draw_sprite(scrolling_clouds_texture, cloud_pos_1, (vec2){640.0f, 180.0f}, no_rotation);
    draw_sprite(scrolling_mountains_texture, mountain_pos_1, (vec2){640.0f,180.0f}, no_rotation);
    draw_sprite(scrolling_trees_texture, tree_pos_1, (vec2){640.0f,180.0f}, no_rotation);
    
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
	vec2 gun_pos = {0, 0};
	vec2 chopper_pos = {player.position[0] - 19.0f, player.position[1] + 62.5f };
	draw_sprite(player_texture, player.position, player_size, no_rotation);
	draw_sprite(chopper_texture, chopper_pos, chopper_size, no_rotation);
	if (currently_held_weapon == WEAPON_KNIFE) {
		glm_vec2_add(player.position, (vec2){-2, -10}, gun_pos);
        draw_sprite(weapon_left_textures[0], gun_pos, gun_sizes[WEAPON_KNIFE], player.rotation);
	} else if (currently_held_weapon == WEAPON_PISTOL) {
		glm_vec2_add(player.position, (vec2){-2, -10}, gun_pos);
        draw_sprite(weapon_left_textures[1], gun_pos, gun_sizes[WEAPON_PISTOL], player.rotation);
	
	} else if (currently_held_weapon == WEAPON_RIFLE) {
		glm_vec2_add(player.position, (vec2){-2, -10}, gun_pos);
        draw_sprite(weapon_left_textures[2], gun_pos, gun_sizes[WEAPON_RIFLE], player.rotation);

	} else if (currently_held_weapon == WEAPON_CHAINGUN) {
		glm_vec2_add(player.position, (vec2){-2, -10}, gun_pos);
        draw_sprite(weapon_left_textures[3], gun_pos, gun_sizes[WEAPON_CHAINGUN], player.rotation);
	
	} else if (currently_held_weapon == WEAPON_ROCKET) {
		glm_vec2_add(player.position, (vec2){-2, -10}, gun_pos);
        draw_sprite(weapon_left_textures[4], gun_pos, gun_sizes[WEAPON_ROCKET], player.rotation);
	}

    for (int i = 0; i < ENEMY_TYPE_COUNT; i++) {
        for (int j = 0; j < guys[i].count; j++) {

			glm_vec2_add(guys[i].positions[j], (vec2){-5, 2}, gun_pos);

			if (guys[i].facing_left[j]) {
				draw_sprite(guys[i].left_facing_texture, guys[i].positions[j], guy_size, no_rotation);
                draw_sprite(weapon_left_textures[i], gun_pos, gun_sizes[i], guys[i].rotations[j]);
			} else {
				draw_sprite(guys[i].right_facing_texture, guys[i].positions[j], guy_size, no_rotation);
                draw_sprite(weapon_right_textures[i], gun_pos, gun_sizes[i], guys[i].rotations[j]);
			
			}
        }
    }
    for (int i = 0; i < BULLET_TYPE_COUNT; i++) {
        for (int j = 0; j < p_bullets[i].count; j++) {
            draw_sprite(p_bullets[i].texture, p_bullets[i].positions[j], bullet_sizes[i], p_bullets[i].rotations[j]);
        }
    }
    for (int i = 0; i < BULLET_TYPE_COUNT; i++) {
        for (int j = 0; j < e_bullets[i].count; j++) {
            draw_sprite(e_bullets[i].texture, e_bullets[i].positions[j], bullet_sizes[i], e_bullets[i].rotations[j]);
        }
    }
    // text
    rgb_color player_health_color = lerp_red_green_100(player.health);
    char player_health_string[24] = "HP:";
    char player_health_number_string[24];

    myitoa(player.health, player_health_number_string);
    strcat(player_health_string, player_health_number_string);
    draw_text(player_health_string, -300, 140, 16, 6, (vec3){player_health_color.r, player_health_color.g, 0});
    if (get_back) {
		draw_text("GET BACK TO THE FRONT LINES", -300, 0, 16, 6, (vec3){1, 1, 1});
    }
    char score_string[24] = "SCORE:";
    char score_number_string[8];
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
    pal_window* window;
    if (is_fullscreen) {
		window = pal_create_window(WINDOW_WIDTH, WINDOW_HEIGHT, "Upside Down Helicopter Man", PAL_WINDOW_FULLSCREEN);
    }
    else {
		window = pal_create_window(WINDOW_WIDTH, WINDOW_HEIGHT, "Upside Down Helicopter Man", 0);
    }
    pal_make_context_current(window);
    if (!gladLoadGLLoader((GLADloadproc)pal_gl_get_proc_address)) {
        fprintf(stderr, "ERROR: Failed to initialize glad!\n");
    }

    pal_set_window_icon_legacy(window, "textures/icon.png");
    pal_set_taskbar_icon(window, "textures/icon.png");
    pal_set_cursor(window, "textures/crosshair_32_by_32.png", 32, 16, 16);
    uint8_t running = pal_true;
    pal_event event;
    pal_swap_interval(1);
    while (running) {
        current_time = (f32)pal_get_time_since_pal_started();
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
						mouse_position[0] = (float)event.motion.x;
                        mouse_position[1] = (float)event.motion.y;
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
