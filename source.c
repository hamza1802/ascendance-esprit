#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

/*------------------------------------*
 *    FONCTIONS D'INITIALISATION     *
 *------------------------------------*/

void initialiser_SDL() {
    // Initialisation de SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("Erreur d'initialisation de la SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    
    // Initialisation de SDL_ttf
    if (TTF_Init() == -1) {
        printf("Erreur d'initialisation de TTF_Init: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    
    // Pour SDL_image 1.2, pas besoin d'initialisation explicite
    // avec IMG_Init, c'est fait automatiquement lors du premier appel
    // à une fonction de SDL_image
}

void charger_animations_personnage(Personnage *pers) {
    const char *anim_types[] = {"idle", "run", "jump", "hurt", "death", "attack", "crouch", "dash", "slide"};
    const int frame_counts[] = {6, 8, 3, 4, 11, 12, 4, 6, 6}; // Frame counts for all animations
    
    // Use the chemin_initial path to detect if this is player 2
    int is_player2 = strstr(pers->chemin_initial, "p2") != NULL;
    
    for (int i = 0; i < 9; i++) {
        char path[100];
        if (is_player2) {
            // For P2, use the directional suffixes in the filenames
            sprintf(path, "resources/p2/spritesheetmc_%s_%s.png", anim_types[i], 
                    strcmp(pers->direction, "right") == 0 ? "R" : "L");
            pers->anim.sprites_R[i] = IMG_Load(path);
            
            sprintf(path, "resources/p2/spritesheetmc_%s_%s.png", anim_types[i],
                    strcmp(pers->direction, "right") == 0 ? "L" : "R");
            pers->anim.sprites_L[i] = IMG_Load(path);
        } else {
            sprintf(path, "resources/player/spritesheetmc_%s_R.png", anim_types[i]);
            pers->anim.sprites_R[i] = IMG_Load(path);
            
            sprintf(path, "resources/player/spritesheetmc_%s_L.png", anim_types[i]);
            pers->anim.sprites_L[i] = IMG_Load(path);
        }
        
        if (!pers->anim.sprites_R[i] || !pers->anim.sprites_L[i]) {
            printf("Failed to load animation: %s\n", path);
            // Only error out for essential animations (idle, run, jump, hurt, death, attack)
            if (i < 6) exit(1);
        }
        pers->anim.frame_counts[i] = frame_counts[i];
    }
    
    pers->anim.current_frame = 0;
    pers->anim.animation_speed = 2;  // Changed from 4 to 2 for even faster animations
    pers->anim.state = PLAYER_IDLE;
    pers->frame_timer = 0;
}

void initialiser_personnage(Personnage *pers, const char *chemin_image) {
    pers->x = 100;
    pers->y = GROUND_LEVEL - 100;
    pers->velocity_y = 0;
    pers->is_jumping = 0;
    pers->vie = 100;
    pers->score = 0;
    pers->vitesse = 12;  // Increased from 8 to 12 for even faster movement
    pers->moving_left = 0;
    pers->moving_right = 0;
    strcpy(pers->direction, "right");
    strcpy(pers->chemin_initial, chemin_image);
    
    // Initialize new fields
    pers->is_attacking = 0;
    pers->is_crouching = 0;
    pers->is_dashing = 0;
    pers->is_sliding = 0;
    pers->dash_cooldown = 0;
    pers->slide_cooldown = 0;
    pers->slide_velocity = 0;
    pers->can_double_jump = 1;
    pers->has_double_jumped = 0;
    pers->combo_multiplier = 1;
    pers->knockback_timer = 0;
    pers->knockback_velocity_x = 0;
    
    charger_animations_personnage(pers);
}

void liberer_animations_personnage(Personnage *pers) {
    for (int i = 0; i < 9; i++) {  // Free all 9 animation states
        if (pers->anim.sprites_R[i]) SDL_FreeSurface(pers->anim.sprites_R[i]);
        if (pers->anim.sprites_L[i]) SDL_FreeSurface(pers->anim.sprites_L[i]);
    }
}

/*------------------------------------*
 *      FONCTIONS D'AFFICHAGE        *
 *------------------------------------*/


/*------------------------------------*
 *    FONCTIONS MANQUANTES           *
 *------------------------------------*/


void update_animation_personnage(Personnage *pers) {
    pers->frame_timer++;
    
    // Handle special states first
    if (pers->anim.state == PLAYER_ATTACK) {
        if (pers->frame_timer >= pers->anim.animation_speed) {
            pers->frame_timer = 0;
            pers->anim.current_frame++;
            pers->is_attacking = (pers->anim.current_frame >= 5 && pers->anim.current_frame <= 8);
            if (pers->anim.current_frame >= pers->anim.frame_counts[PLAYER_ATTACK]) {
                pers->anim.current_frame = 0;
                pers->anim.state = PLAYER_IDLE;
                pers->is_attacking = 0;
            }
        }
        return;
    }
    
    if (pers->anim.state == PLAYER_DASH) {
        if (pers->frame_timer >= pers->anim.animation_speed) {
            pers->frame_timer = 0;
            pers->anim.current_frame++;
            if (pers->anim.current_frame >= pers->anim.frame_counts[PLAYER_DASH]) {
                pers->anim.current_frame = 0;
                pers->anim.state = PLAYER_IDLE;
                pers->is_dashing = 0;
            }
        }
        return;
    }

    if (pers->anim.state == PLAYER_CROUCH) {
        if (pers->frame_timer >= pers->anim.animation_speed) {
            pers->frame_timer = 0;
            pers->anim.current_frame = (pers->anim.current_frame + 1) % 
                pers->anim.frame_counts[PLAYER_CROUCH];
        }
        if (!pers->is_crouching) {
            pers->anim.state = PLAYER_IDLE;
            pers->anim.current_frame = 0;
        }
        return;
    }

    if (pers->anim.state == PLAYER_HURT || pers->anim.state == PLAYER_DEATH) {
        if (pers->anim.state == PLAYER_HURT) {
            if (pers->frame_timer >= pers->anim.animation_speed) {
                pers->frame_timer = 0;
                pers->anim.current_frame++;
                
                if (pers->anim.current_frame >= pers->anim.frame_counts[PLAYER_HURT]) {
                    pers->anim.current_frame = 0;
                    pers->anim.state = PLAYER_IDLE;
                    pers->is_attacking = 0; // Reset attack state after hurt
                }
            }
        } else if (pers->anim.state == PLAYER_DEATH) {
            if (pers->frame_timer >= pers->anim.animation_speed) {
                pers->frame_timer = 0;
                if (pers->anim.current_frame < pers->anim.frame_counts[PLAYER_DEATH] - 1) {
                    pers->anim.current_frame++;
                }
            }
        }
        return;
    }

    // Update state based on movement and actions
    PlayerState new_state = PLAYER_IDLE;
    
    if (pers->is_crouching) {
        new_state = PLAYER_CROUCH;
    } else if (pers->is_dashing) {
        new_state = PLAYER_DASH;
    } else if (pers->is_sliding) {
        new_state = PLAYER_SLIDE;
    } else if (pers->is_jumping) {
        new_state = PLAYER_JUMP;
    } else if (pers->moving_left || pers->moving_right) {
        new_state = PLAYER_RUN;
    }

    // Reset frame counter if state changed
    if (new_state != pers->anim.state) {
        pers->anim.state = new_state;
        pers->anim.current_frame = 0;
        pers->frame_timer = 0;
    }
    
    // Update animation frame
    if (pers->frame_timer >= pers->anim.animation_speed) {
        pers->frame_timer = 0;
        pers->anim.current_frame = (pers->anim.current_frame + 1) % 
            pers->anim.frame_counts[pers->anim.state];
    }
}

void afficher_personnage(Personnage *pers) {
    SDL_Rect src, dest;
    SDL_Surface *current_spritesheet;
    
    // Select the appropriate spritesheet based on direction and state
    if (strcmp(pers->direction, "right") == 0) {
        current_spritesheet = pers->anim.sprites_R[pers->anim.state];
    } else {
        current_spritesheet = pers->anim.sprites_L[pers->anim.state];
    }
    
    // Attack frames are 66px wide, others are 64px
    int frame_width = (pers->anim.state == PLAYER_ATTACK) ? 145 : 145; // All frames now 145px wide
    
    // Calculate source rectangle from spritesheet
    src.x = pers->anim.current_frame * frame_width;
    src.y = 0;
    src.w = frame_width;
    src.h = 100;  // Updated height to 100px
    
    // Set destination rectangle
    dest.x = pers->x;
    dest.y = pers->y;
    dest.w = frame_width;
    dest.h = 100;  // Updated height to 100px
    
    SDL_BlitSurface(current_spritesheet, &src, SDL_GetVideoSurface(), &dest);
}

void afficher_interface_jeu(int vie, int score) {
    TTF_Font *police = NULL;
    
    // Add local font path first, then system fonts
    const char *chemins_polices[] = {
        "resources/font.ttf",  // Local font file
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf"
    };
    
    int i;
    for (i = 0; i < 5; i++) {
        police = TTF_OpenFont(chemins_polices[i], 16);
        if (police) break;
    }
    
    if (!police) {
        printf("Erreur: Impossible de charger une police. Le texte ne sera pas affiché.\n");
        return;
    }
    
    // Création d'une chaîne de caractères pour afficher la vie et le score
    char texte_vie[50];
    char texte_score[50];
    sprintf(texte_vie, "Vie: %d", vie);
    sprintf(texte_score, "Score: %d", score);
    
    // Couleur du texte (blanc)
    SDL_Color couleur_texte = {255, 255, 255};
    
    // Rendu du texte
    SDL_Surface *surface_vie = TTF_RenderText_Solid(police, texte_vie, couleur_texte);
    SDL_Surface *surface_score = TTF_RenderText_Solid(police, texte_score, couleur_texte);
    
    if (!surface_vie || !surface_score) {
        printf("Erreur lors du rendu du texte: %s\n", TTF_GetError());
        TTF_CloseFont(police);
        return;
    }
    
    // Position du texte (en haut à gauche)
    SDL_Rect position_vie = {10, 10, 0, 0};
    SDL_Rect position_score = {10, 30, 0, 0};
    
    // Affichage du texte
    SDL_BlitSurface(surface_vie, NULL, SDL_GetVideoSurface(), &position_vie);
    SDL_BlitSurface(surface_score, NULL, SDL_GetVideoSurface(), &position_score);
    
    // Libération des ressources
    SDL_FreeSurface(surface_vie);
    SDL_FreeSurface(surface_score);
    TTF_CloseFont(police);
}
