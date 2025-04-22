#include "header.h"

void handle_player_mechanics(Personnage *pers, Uint8 *keystate) {
    // Double jump
    if (keystate[SDLK_SPACE] && !pers->is_jumping && pers->can_double_jump && !pers->has_double_jumped) {
        pers->velocity_y = -10;
        pers->has_double_jumped = 1;
    }
    
    // Dash
    if (keystate[SDLK_LSHIFT] && pers->dash_cooldown <= 0) {
        pers->is_dashing = 1;
        pers->dash_cooldown = 60; // 1 second cooldown
        float dash_speed = 15.0f;
        if (strcmp(pers->direction, "right") == 0) {
            pers->knockback_velocity_x = dash_speed;
        } else {
            pers->knockback_velocity_x = -dash_speed;
        }
    }
    
    // Update dash
    if (pers->is_dashing) {
        pers->x += pers->knockback_velocity_x;
        pers->is_dashing = pers->dash_cooldown > 50; // Dash for 10 frames
    }
    
    // Reset double jump when touching ground
    if (pers->y >= GROUND_LEVEL - 64) {
        pers->has_double_jumped = 0;
        pers->can_double_jump = 1;
    }
    
    // Handle knockback
    if (pers->knockback_timer > 0) {
        pers->x += pers->knockback_velocity_x;
        pers->knockback_timer--;
    }
    
    // Update cooldowns
    if (pers->dash_cooldown > 0) {
        pers->dash_cooldown--;
    }
}

void handle_player_hit(Personnage *pers, int direction) {
    pers->vie -= 5;
    pers->knockback_timer = 20;
    pers->knockback_velocity_x = direction * 5.0f;
    pers->combo_multiplier = 1; // Reset combo
}

void add_score(Personnage *pers, int base_points) {
    pers->score += base_points * pers->combo_multiplier;
    pers->combo_multiplier = MIN(pers->combo_multiplier + 1, 8); // Max 8x multiplier
}
