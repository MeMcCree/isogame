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

enum movedir_e {
    MOVE_SOUTH, MOVE_WEST, MOVE_NORTH, MOVE_EAST
};

class player_t : public sprite_t, public animatable_t {
public:
    bool is_moving = false;
    bool is_falling = false;

    void update(float dt) override {
        if (action != nullptr) {
            action->step((void*)this, dt);
        }

        if (anim != nullptr) {
            anim->step((void*)this, dt);
        }
    }
};

class tile_t : public sprite_t, public animatable_t {
public:
    bool* is_falling = nullptr;

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

    void finish(void* anim_ent) override {
    }

    void step(void* anim_ent, float dt) override {
        if (is_finished()) {
            return;
        }

        sprite_t* sprite = (sprite_t*)anim_ent;

        accum += dt;
        sprite->atlas_idx = start_idx + (int)((end_idx - start_idx) * fminf(1.0f, fmaxf(0.0f, accum - delay) / time));

        if (is_finished()) {
            finish(anim_ent);
        }
    }
};

float linear_move_func(float x) {
    return x;
}

class linear_move : action_t {
public:
    Vector3 start, end;
    float accum = 0.0f;
    float delay = 0.0f;
    float time = 1.0f;
    function<float(float)> f;

    linear_move(Vector3 start, Vector3 end, float time, float delay = 0.0f, function<float(float)> f = [](float x) { return x; })
    : start(start), end(end), time(time), delay(delay), f(f) {}

    bool is_acting() {
        return accum >= delay;
    }

    bool is_finished() override {
        return accum - delay >= time;
    }

    void step(void* ent, float dt) override {
        if (is_finished()) {
            return;
        }

        sprite_t* sprite = (sprite_t*)ent;
        accum += dt;
        float t = fminf(1.0f, fmaxf(0.0f, accum - delay) / time);
        t = f(t);
        sprite->pos = Vector3Lerp(start, end, t);

        if (is_finished()) {
            finish(ent);
        }
    }
};

class player_move : public linear_move {
public:
    player_move(Vector3 start, Vector3 end, float time, float delay = 0.0f, function<float(float)> f = [](float x) { return x; })
    : linear_move(start, end, time, delay, f) {}

    void finish(void* ent) override {
        player_t* ply = (player_t*)ent;
        ply->is_moving = false;
    }
};

class tile_move : public linear_move {
public:
    tile_move(Vector3 start, Vector3 end, float time, float delay = 0.0f, function<float(float)> f = [](float x) { return x; })
    : linear_move(start, end, time, delay, f) {}

    void finish(void* ent) override {
        tile_t* tile = (tile_t*)ent;
        if (tile->is_falling != nullptr) {
            *tile->is_falling = false;
        }
    }
};

float nearness(sprite_t* sprite) {
    return sprite->pos.x + sprite->pos.y - sprite->pos.z;
}

int main(int argc, char* argv[]) {
    const int screen_width = 600;
    const int screen_height = 600;
    const char* title = "Iso";
    InitWindow(screen_width, screen_height, title);
    SetTargetFPS(30);

    atlas_texture = LoadTexture("resource/atlas.png");
    if (atlas_texture.width == 0) {
        TraceLog(LOG_ERROR, "Failed to load atlas.");
        return 1;
    }

    num_tiles_x = atlas_texture.width / SPRITE_WIDTH;
    num_tiles_y = atlas_texture.height / SPRITE_HEIGHT;

    Camera2D camera = {0};
    camera.offset = (Vector2){.x = -1.5*SPRITE_WIDTH + screen_width / 2, .y = 0};
    camera.zoom = 2.0f;

    player_t player;
    player.atlas_idx = 2;
    player.pos.z = -16.0f;
    player.draw_z = false;
    movedir_e movedir = MOVE_SOUTH;

    bool is_tile_falling = 0;
    tile_t map[MAP_HEIGHT*MAP_WIDTH];
    for (int i = 0; i < MAP_HEIGHT*MAP_WIDTH; i++) {
        map[i].pos = (Vector3){.x = i % MAP_WIDTH, .y = i / MAP_WIDTH, .z = 0.0f};
        map[i].atlas_idx = 1;
        map[i].is_falling = &is_tile_falling;
    }

    vector<sprite_t*> sprites;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if ((IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_RIGHT)) && !player.is_moving) {
            player.set_animation(new player_move_anim(2, 10, 1.5f), true);

            Vector3 end = player.pos;
            if (IsKeyPressed(KEY_DOWN)) {
                end.x += 1;
                movedir = MOVE_SOUTH;
            } else if (IsKeyPressed(KEY_LEFT)) {
                end.y += 1;
                movedir = MOVE_WEST;
            } else if (IsKeyPressed(KEY_UP)) {
                end.x -= 1;
                movedir = MOVE_NORTH;
            } else if (IsKeyPressed(KEY_RIGHT)) {
                end.y -= 1;
                movedir = MOVE_EAST;
            }

            player.set_action(new player_move(player.pos, end, 0.8f, 0.5f), true);

            player.is_moving = true;
        }

        // player_fall: z 0 -> 512, 2.5s, 0.15s delay
        // tile_fall:   z 0 -> 512, 2.0s, 1.0s delay
        player.update(dt);

        for (int i = 0; i < MAP_HEIGHT*MAP_WIDTH; i++) {
            map[i].update(dt);
        }

        if (!is_tile_falling) {
            int tile_idx = rand() % (MAP_HEIGHT*MAP_WIDTH);
            if (map[tile_idx].action == nullptr) {
                is_tile_falling = 1;
                map[tile_idx].atlas_idx = 0;
                map[tile_idx].draw_z = true;
                Vector3 end = map[tile_idx].pos;
                end.z += 512.0f;
                map[tile_idx].set_action(new tile_move(map[tile_idx].pos, end, 2.0f, 1.0f, [](float x) { return x*x; }), true);
            }
        }

        int player_idx = (int)player.pos.y * MAP_HEIGHT + (int)player.pos.x;
        if (player.pos.x < 0 || player.pos.x > MAP_WIDTH || player.pos.y < 0 || player.pos.y > MAP_HEIGHT) {
            player_idx = -1;
        }

        if (!player.is_falling && !player.is_moving && (player_idx == -1 || map[player_idx].action != nullptr && ((tile_move*)&map[player_idx].action)->is_acting())) {
            player.is_moving = 1;
            player.is_falling = 1;

            player.pos.z = 0.0f;
            player.draw_z = 1;
            player.atlas_idx = 5;

            Vector3 end = player.pos;
            end.z += 512.0f;
            player.set_action(new player_move(player.pos, end, 2.5f, 0.15f, [](float x) { return x*x; }), true);
        }

        sprites.clear();
        for (int i = 0; i < MAP_HEIGHT*MAP_WIDTH; i++) {
            sprites.push_back((sprite_t*)&map[i]);
        }

        sprites.push_back((sprite_t*)&player);
        sprite_t eyes;
        eyes.pos = player.pos;
        eyes.draw_z = player.draw_z;
        eyes.atlas_idx = player.atlas_idx + 9;
        switch (movedir) {
            case MOVE_SOUTH: {
                eyes.flip = false;
                sprites.push_back((sprite_t*)&eyes);
            } break;
            case MOVE_WEST: {
                eyes.flip = true;
                sprites.push_back((sprite_t*)&eyes);
            } break;
            default: break;
        }

        for (int i = 0; i < (int)sprites.size() - 1; i++) {
            for (int j = i + 1; j < sprites.size(); j++) {
                if (nearness(sprites[i]) > nearness(sprites[j])) {
                    sprite_t* temp = sprites[i];
                    sprites[i] = sprites[j];
                    sprites[j] = temp;
                }
            }
        }

        BeginDrawing();
            ClearBackground(BLACK);
            BeginMode2D(camera);
                for (int i = 0; i < sprites.size(); i++) {
                    sprites[i]->draw();
                }
            // player.draw();
            // switch (movedir) {
            //     case MOVE_SOUTH: case MOVE_WEST: eyes.draw(); break;
            //     default: break;
            // }
            EndMode2D();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}