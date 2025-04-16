#pragma once

#include <cassert>
#include <raylib.h>
#include <raymath.h>

class sprite_t;

class action_t {
public:
    virtual bool is_finished() = 0;
    virtual void step(void* anim_ent, float dt) = 0;
    virtual void finish(void* anim_ent) = 0;
};

class sprite_t {
public:
    action_t* action = nullptr;
    int atlas_idx;
    Vector3 pos;
    bool flip;
    bool draw_z;

    ~sprite_t();

    sprite_t()
    : pos((Vector3){.x = 0, .y = 0, .z = 0}), atlas_idx(0), flip(false), draw_z(false) {}
    
    sprite_t(const Vector2& pos, const float& z, const int& atlas_idx, const bool& flip = false, const bool& draw_z = false)
    : pos((Vector3){.x = pos.x, .y = pos.y, .z = z}), atlas_idx(atlas_idx), flip(flip), draw_z(draw_z) {}

    sprite_t(const Vector3& pos, const int& atlas_idx, const bool& flip = false, const bool& draw_z = false)
    : pos(pos), atlas_idx(atlas_idx), flip(flip), draw_z(draw_z) {}

    void set_action(void* new_action, bool forced);

    void stop_action(bool forced);

    virtual void update(float dt);

    virtual void draw();
};

class animatable_t {
public:
    action_t* anim = nullptr;

    ~animatable_t();

    void set_animation(void* new_anim, bool forced);

    void stop_animation(bool forced);
};