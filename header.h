#ifndef GAME_HEADER_H
#define GAME_HEADER_H

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>

#define GROUND_LEVEL 620  // 720 - 200 from bottom
#define GRAVITY 1     // Increased from 0.5f for faster falling
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define VIEWPORT_WIDTH (SCREEN_WIDTH / 2)  // Split screen width

// P1 Controls
#define P1_LEFT SDLK_q
#define P1_RIGHT SDLK_d
#define P1_JUMP SDLK_z
#define P1_ATTACK SDLK_e
#define P1_CROUCH SDLK_s
#define P1_DASH SDLK_a
#define P1_SLIDE SDLK_w

// P2 Controls (original)
#define P2_LEFT SDLK_LEFT
#define P2_RIGHT SDLK_RIGHT
#define P2_JUMP SDLK_SPACE
#define P2_ATTACK SDLK_k
#define P2_CROUCH SDLK_DOWN
#define P2_DASH SDLK_l
#define P2_SLIDE SDLK_j

/*------------------------------------*
 *          STRUCTURES DE DONNÉES     *
 *------------------------------------*/
typedef struct {
    SDL_Surface *image;
    int x, y;
    int volume;
    int is_fullscreen;
    char niveau[20]; // "Menu", "Options", "Jouer", "Enigme"
} Background;

typedef struct {
    SDL_Surface *image_normal;
    SDL_Surface *image_actif;
    int x, y;
    int actif;
} Bouton;

typedef enum {
    PLAYER_IDLE,
    PLAYER_RUN,
    PLAYER_JUMP,
    PLAYER_HURT,
    PLAYER_DEATH,
    PLAYER_ATTACK,
    PLAYER_CROUCH,
    PLAYER_DASH,
    PLAYER_SLIDE
} PlayerState;

typedef struct {
    SDL_Surface *sprites_R[9];    // Right-facing animations [idle,run,jump,hurt,death,attack,crouch,dash,slide]
    SDL_Surface *sprites_L[9];    // Left-facing animations
    int frame_counts[9];          // Number of frames for each animation
    int current_frame;            // Current frame in animation
    int animation_speed;          // Speed of animation
    PlayerState state;            // Current animation state
} PlayerAnimation;

typedef struct {
    PlayerAnimation anim;
    int x, y;
    int vie;
    int score;
    int vitesse;
    float velocity_y;  // Vertical velocity for jumping
    int is_jumping;    // Jump state
    char direction[10]; // "left", "right", "up"
    char chemin_initial[100];  // Store the initial sprite path
    // Add movement flags
    int moving_left;
    int moving_right;
    int moving_up;
    int moving_down;
    int frame_timer;              // For controlling animation speed
    int is_attacking;  // Attack state flag
    int can_double_jump;
    int has_double_jumped;
    int dash_cooldown;
    int is_dashing;
    int combo_multiplier;
    int knockback_timer;
    float knockback_velocity_x;
    int is_crouching;
    int is_sliding;
    float slide_velocity;
    int slide_cooldown;
} Personnage;

/*------------------------------------*
 *         MODULE INITIALISATION      *
 *------------------------------------*/
void initialiser_SDL();
void initialiser_background(Background *bg, const char *chemin_image);
void initialiser_boutons(Bouton boutons[], int nb_boutons);
void initialiser_personnage(Personnage *pers, const char *chemin_image);

/*------------------------------------*
 *         MODULE AFFICHAGE           *
 *------------------------------------*/
void afficher_background(Background *bg);
void afficher_boutons_menu(Bouton boutons[], int nb_boutons);
void afficher_boutons_options(Bouton boutons[], int nb_boutons);
void afficher_personnage(Personnage *pers);
void afficher_interface_jeu(int vie, int score);

/*------------------------------------*
 *         MODULE GAMEPLAY            *
 *------------------------------------*/
void charger_animations_personnage(Personnage *pers);
void update_animation_personnage(Personnage *pers);
void liberer_animations_personnage(Personnage *pers);

#endif