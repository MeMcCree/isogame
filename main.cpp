#include <iostream>
#include <vector>
#include <functional>
#include <memory>
#include <cassert>
#include <raylib.h>
#include <raymath.h>
#include "baseclasses.h"

using namespace std;

constexpr int SPRITE_WIDTH = 64;
constexpr int SPRITE_HEIGHT = 64;

Texture2D atlas_texture = {0};
int num_tiles_x = 0;
int num_tiles_y = 0;

constexpr int MAP_WIDTH = 5;
constexpr int MAP_HEIGHT = 5;

enum move_dir_e {
    MOVE_SOUTH, MOVE_WEST, MOVE_NORTH, MOVE_EAST
};

class player_t : public sprite_t, public animatable_t {
public:
    bool is_moving = false;

    void update(float dt) override {
        if (action != nullptr) {
            action->step((void*)this, dt);
        }

        if (anim != nullptr) {
            anim->step((void*)this, dt);
        }
    }
};

class player_move_anim : action_t {
private:
    int start_idx = 0, end_idx = 1;
    float accum = 0.0f;
    float delay = 0.0f;
    float time = 1.0f;
public:
    player_move_anim(int start_idx, int end_idx, float time, float delay = 0.0f)
    : start_idx(start_idx), end_idx(end_idx), time(time), delay(delay) {}

    bool is_finished() override {
        return accum - delay >= time;
    }

    void step(void* anim_ent, float dt) override {
        if (is_finished()) {
            return;
        }

        sprite_t* sprite = (sprite_t*)anim_ent;

        accum += dt;
        sprite->atlas_idx = start_idx + (int)((end_idx - start_idx) * fminf(1.0f, fmaxf(0.0f, accum - delay) / time));
    }

    void finish(void* anim_ent) override {
        player_t* ply = (player_t*)anim_ent;
        ply->is_moving = false;
    }
};

/*
typedef struct {
    int atlas_idx;
    int flip, draw_z;
    int is_animating;
    Vector2 pos;
    float z;
} sprite_t;

float nearness(sprite_t sprite) {
    return sprite.pos.x + sprite.pos.y - sprite.z;
}

void DrawSprite(sprite_t sprite) {
    Rectangle src = (Rectangle){
        .x = (sprite.atlas_idx % num_tiles_x) * SPRITE_WIDTH,
        .y = (sprite.atlas_idx / num_tiles_x) * SPRITE_HEIGHT,
        .width = (sprite.flip ? -1 : 1) * SPRITE_WIDTH,
        .height = SPRITE_HEIGHT
    };
    DrawTextureRec(atlas_texture, src, to_screen(sprite.pos, (sprite.draw_z ? sprite.z : 0.0f)), WHITE);
}

float fall_func(float x) {
    return x*x;
}

typedef float(*func_t)(float);

typedef struct {
    Vector2 start, end;
    float start_z, end_z;
    float accum, delay, time;
    int* is_moving;
    Vector2* pos;
    float* z;
    func_t f;
} move_t;

typedef struct {
    int start_idx, end_idx;
    int* idx;
    int* is_animating;
    float accum, delay, time;
} anim_t;

int move_object(move_t* move) {
    if (!move->is_moving) return 0;

    move->accum += GetFrameTime();
    float t = fminf(1.0f, fmaxf(0.0f, move->accum - move->delay) / move->time);
    if (move->f != NULL) {
        t = move->f(t);
    }
    *move->pos = Vector2Lerp(move->start, move->end, t);
    *move->z = move->start_z + (move->end_z - move->start_z) * t;
    if (move->accum > move->time + move->delay) {
        *move->is_moving = 0;
        return 1;
    }
    return 0;
}

int play_anim(anim_t* anim) {
    if (!anim->is_animating) return 0;

    anim->accum += GetFrameTime();
    *anim->idx = anim->start_idx + (int)((anim->end_idx - anim->start_idx) * fminf(1.0f, fmaxf(0.0f, anim->accum - anim->delay) / anim->time));
    if (anim->accum > anim->time + anim->delay) {
        return 1;
    }

    return 0;
}

typedef struct {
    sprite_t sprite;
    move_t move;
    anim_t anim;
    int is_moving, is_falling;
} player_t;

typedef struct {
    sprite_t sprite;
    int is_falling;
    float fall_time;
} floor_t;

struct {
    move_t* items;
    int count;
    int capacity;
} moves = {0};

struct {
    anim_t* items;
    int count;
    int capacity;
} anims = {0};

struct {
    sprite_t* items;
    int count;
    int capacity;
} sprites = {0};
*/

int main(int argc, char* argv[]) {
    const int screen_width = 600;
    const int screen_height = 600;
    const char* title = "Iso";
    InitWindow(screen_width, screen_height, title);
    SetTargetFPS(30);

    player_t player;
    player.atlas_idx = 2;

    atlas_texture = LoadTexture("resource/atlas.png");
    if (atlas_texture.width == 0) {
        TraceLog(LOG_ERROR, "Failed to load atlas.");
        return 1;
    }

    num_tiles_x = atlas_texture.width / SPRITE_WIDTH;
    num_tiles_y = atlas_texture.height / SPRITE_HEIGHT;

    /*Camera2D camera = {0};
    camera.offset = (Vector2){.x = -1.5*SPRITE_WIDTH + screen_width / 2, .y = 0};
    camera.zoom = 2.0f;

    player_t player = {0};
    player.sprite.atlas_idx = 2;
    player.sprite.z = -16.0f;
    movedir_e movedir = MOVE_SOUTH;

    anim_t jump_anim = {0};
    jump_anim.start_idx = 2;
    jump_anim.end_idx = 10;
    jump_anim.time = 1.5f;
    jump_anim.is_animating = &player.sprite.is_animating;
    jump_anim.idx = &player.sprite.atlas_idx;

    move_t player_move = {0};
    player_move.start_z = -16.0f;
    player_move.end_z = -16.0f;
    player_move.delay = 0.5f;
    player_move.time = 0.8f;
    player_move.is_moving = &player.is_moving;
    player_move.pos = &player.sprite.pos;
    player_move.z = &player.sprite.z;

    int dummy;
    move_t player_fall = {0};
    player_fall.start_z = 0.0f;
    player_fall.end_z = 512.0f;
    player_fall.time = 2.5f;
    player_fall.delay = 0.15f;
    player_fall.is_moving = &dummy;
    player_fall.pos = &player.sprite.pos;
    player_fall.z = &player.sprite.z;
    player_fall.f = &fall_func;

    move_t tile_fall = {0};
    tile_fall.start_z = 0.0f;
    tile_fall.end_z = 512.0f;
    tile_fall.delay = 1.0f;
    tile_fall.time = 2.0f;
    tile_fall.is_moving = &dummy;

    floor_t map[MAP_HEIGHT*MAP_WIDTH] = {0};
    for (int i = 0; i < MAP_HEIGHT*MAP_WIDTH; i++) {
        map[i].sprite = (sprite_t){
            .atlas_idx = 1,
            .pos = (Vector2){.x = i % MAP_WIDTH, .y = i / MAP_WIDTH},
            .flip = 0
        };
        map[i].is_falling = 0;
    }

    int is_tile_falling = 0;*/

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (IsKeyPressed(KEY_DOWN) && !player.is_moving) {
            player.set_animation(new player_move_anim(2, 10, 1.5f), true);
        }

        player.update(dt);
        /*if ((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_RIGHT)) && !player.is_moving) {
            player_move.start = player.sprite.pos;
            if (IsKeyPressed(KEY_DOWN)) {
                player_move.end = (Vector2){player.sprite.pos.x + 1, player.sprite.pos.y};
                movedir = MOVE_SOUTH;
            } else if (IsKeyPressed(KEY_LEFT)) {
                player_move.end = (Vector2){player.sprite.pos.x, player.sprite.pos.y + 1};
                movedir = MOVE_WEST;
            } else if (IsKeyPressed(KEY_UP)) {
                player_move.end = (Vector2){player.sprite.pos.x - 1, player.sprite.pos.y};
                movedir = MOVE_NORTH;
            } else if (IsKeyPressed(KEY_RIGHT)) {
                player_move.end = (Vector2){player.sprite.pos.x, player.sprite.pos.y - 1};
                movedir = MOVE_EAST;
            }

            player_move.accum = 0.0f;
            *player_move.is_moving = 1;
            player.move = player_move;
            jump_anim.accum = 0.0f;
            *jump_anim.is_animating = 1;
            *jump_anim.idx = jump_anim.start_idx;
            player.anim = jump_anim;
        }

        move_object(&player.move);
        play_anim(&player.anim);

        for (int i = 0; i < moves.count; i++) {
            if (move_object(&moves.items[i])) {
                for (int j = i; j < moves.count - 1; j++) {
                    moves.items[j] = moves.items[j + 1];
                }
                moves.count--;
                i--;
            }
        }

        for (int i = 0; i < anims.count; i++) {
            if (play_anim(&anims.items[i])) {
                for (int j = i; j < anims.count - 1; j++) {
                    anims.items[j] = anims.items[j + 1];
                }
                anims.count--;
                i--;
            }
        }

        if (!is_tile_falling) {
            int tile_idx = rand() % (MAP_HEIGHT*MAP_WIDTH);
            if (!map[tile_idx].is_falling) {
                is_tile_falling = 1;
                tile_fall.start = map[tile_idx].sprite.pos;
                tile_fall.end = map[tile_idx].sprite.pos;
                tile_fall.pos = &map[tile_idx].sprite.pos;
                tile_fall.z = &map[tile_idx].sprite.z;
                tile_fall.is_moving = &is_tile_falling;
                map[tile_idx].is_falling = 1;
                map[tile_idx].sprite.atlas_idx = 0;
                map[tile_idx].fall_time = GetTime();
                *tile_fall.is_moving = 1;
                map[tile_idx].sprite.draw_z = 1;
                tile_fall.f = &fall_func;
                da_append(&moves, tile_fall);
            }
        }

        int player_idx = (int)player.sprite.pos.y * MAP_HEIGHT + (int)player.sprite.pos.x;
        if (player.sprite.pos.x < 0 || player.sprite.pos.x > MAP_WIDTH || player.sprite.pos.y < 0 || player.sprite.pos.y > MAP_HEIGHT) {
            player_idx = -1;
        }

        if (!player.is_falling && (player_idx == -1 || map[player_idx].is_falling && GetTime() - map[player_idx].fall_time > tile_fall.delay)) {
            player.is_moving = 1;
            player.is_falling = 1;

            player.sprite.draw_z = 1;
            player.sprite.z = 0.0f;
            player.sprite.atlas_idx = 5;
            
            player_fall.accum = 0.0f;
            player_fall.start = player.sprite.pos;
            player_fall.end = player.sprite.pos;
            player.move = player_fall;
        }

        sprites.count = 0;
        for (int i = 0; i < MAP_HEIGHT*MAP_WIDTH; i++) {
            da_append(&sprites, map[i].sprite);
        }

        da_append(&sprites, player.sprite);
        sprite_t eyes = player.sprite;
        eyes.atlas_idx += 9;
        switch (movedir) {
            case MOVE_SOUTH: {
                eyes.flip = 0;
                da_append(&sprites, eyes);
            } break;
            case MOVE_WEST: {
                eyes.flip = 1;
                da_append(&sprites, eyes);
            } break;
            default: break;
        }

        for (int i = 0; i < sprites.count - 1; i++) {
            for (int j = i + 1; j < sprites.count; j++) {
                if (nearness(sprites.items[i]) > nearness(sprites.items[j])) {
                    sprite_t temp = sprites.items[i];
                    sprites.items[i] = sprites.items[j];
                    sprites.items[j] = temp;
                }
            }
        }*/

        BeginDrawing();
            ClearBackground(BLACK);
            /*BeginMode2D(camera);
                for (int i = 0; i < sprites.count; i++) {
                    DrawSprite(sprites.items[i]);
                }
            EndMode2D();*/
            player.draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}