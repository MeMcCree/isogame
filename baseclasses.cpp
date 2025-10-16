#include "baseclasses.h"

extern Texture2D atlas_texture;
extern int num_tiles_x;
extern int num_tiles_y;

constexpr int SPRITE_WIDTH = 64;
constexpr int SPRITE_HEIGHT = 64;

Vector2 to_screen(Vector3 pos) {
    return (Vector2){
        .x = pos.x * SPRITE_WIDTH / 2.0f - pos.y * SPRITE_WIDTH / 2.0f,
        .y = pos.x * SPRITE_HEIGHT / 4.0f + pos.y * SPRITE_HEIGHT / 4.0f + pos.z
    };
}

sprite_t::~sprite_t() {
    if (action != nullptr) {
        delete action;
    }
}

void sprite_t::set_action(void* new_action, bool forced = false) {
    assert(new_action != nullptr && "Nullptr provided, expected a valid pointer to action_t");
    if (forced || (action != nullptr && action->is_finished())) {
        if (action != nullptr) {
            delete action;
        }
        action = (action_t*)new_action;
    }
}

void sprite_t::stop_action(bool forced = true) {
    if (action == nullptr) {
        return;
    }

    if (forced || action->is_finished()) {
        delete action;
        action = nullptr;
    }
}

void sprite_t::update(float dt) {
    if (action != nullptr) {
        action->step((void*)this, dt);
    }
}

void sprite_t::draw() {
    Rectangle src = (Rectangle){
        .x = (this->atlas_idx % num_tiles_x) * SPRITE_WIDTH,
        .y = (this->atlas_idx / num_tiles_x) * SPRITE_HEIGHT,
        .width = (this->flip ? -1 : 1) * SPRITE_WIDTH,
        .height = SPRITE_HEIGHT
    };
    DrawTextureRec(atlas_texture, src, to_screen((Vector3){this->pos.x, this->pos.y, this->pos.z}), WHITE);
}

animatable_t::~animatable_t() {
    if (anim != nullptr) {
        delete anim;
    }
}

void animatable_t::set_animation(void* new_anim, bool forced = false) {
    assert(new_anim != nullptr && "Nullptr provided, expected a valid pointer to action_t");
    if (forced || (anim != nullptr && anim->is_finished())) {
        if (anim != nullptr) {
            delete anim;
        }
        anim = (action_t*)new_anim;
    }
}

void animatable_t::stop_animation(bool forced = true) {
    if (anim == nullptr) {
        return;
    }

    if (forced || anim->is_finished()) {
        delete anim;
        anim = nullptr;
    }
}