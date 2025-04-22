#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int main(int argc, char *argv[]) {
    // Initialisation de la SDL et ses extensions
    initialiser_SDL();
    
    // Création de la fenêtre et configuration de l'écran
    SDL_Surface *ecran = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!ecran) {
        printf("Erreur lors de la création de la fenêtre: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    SDL_WM_SetCaption("Two Player Game", NULL);
    
    // Load background
    SDL_Surface *background = IMG_Load("resources/background.png");
    if (!background) {
        printf("Erreur de chargement du background: %s\n", IMG_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    // Scale background to screen size if needed
    SDL_Surface *scaled_background = SDL_CreateRGBSurface(SDL_HWSURFACE, 
        SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0, 0, 0, 0);
    SDL_SoftStretch(background, NULL, scaled_background, NULL);
    SDL_FreeSurface(background);
    
    // Initialisation des deux personnages
    Personnage joueur1;
    Personnage joueur2;
    initialiser_personnage(&joueur1, "resources/player/spritesheetmc_idle_R.png");
    initialiser_personnage(&joueur2, "resources/p2/spritesheetmc idle.png");
    
    // Set initial positions
    joueur1.x = VIEWPORT_WIDTH / 4;  // Center in left viewport
    joueur2.x = VIEWPORT_WIDTH + VIEWPORT_WIDTH / 4;  // Center in right viewport
    
    // Variables pour la gestion des événements et la boucle de jeu
    SDL_Event evenement;
    int continuer = 1;
    int dernierTemps = 0, tempsActuel = 0;
    
    // Create viewports for split screen
    SDL_Rect viewport1 = {0, 0, VIEWPORT_WIDTH, SCREEN_HEIGHT};
    SDL_Rect viewport2 = {VIEWPORT_WIDTH, 0, VIEWPORT_WIDTH, SCREEN_HEIGHT};
    
    // Boucle principale du jeu
    while (continuer) {
        tempsActuel = SDL_GetTicks();
        
        // Limiter à 60 FPS (environ 16.7ms par frame)
        if (tempsActuel - dernierTemps < 16)
            continue;
            
        dernierTemps = tempsActuel;
        
        // Gestion des événements
        while (SDL_PollEvent(&evenement)) {
            switch (evenement.type) {
                case SDL_QUIT:
                    continuer = 0;
                    break;
                case SDL_KEYDOWN:
                    switch (evenement.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            continuer = 0;
                            break;
                        case P1_ATTACK:
                            if (!joueur1.is_attacking && joueur1.anim.state != PLAYER_ATTACK) {
                                joueur1.anim.state = PLAYER_ATTACK;
                                joueur1.anim.current_frame = 0;
                                joueur1.frame_timer = 0;
                                joueur1.is_attacking = 1;
                            }
                            break;
                        case P2_ATTACK:
                            if (!joueur2.is_attacking && joueur2.anim.state != PLAYER_ATTACK) {
                                joueur2.anim.state = PLAYER_ATTACK;
                                joueur2.anim.current_frame = 0;
                                joueur2.frame_timer = 0;
                                joueur2.is_attacking = 1;
                            }
                            break;
                    }
                    break;
            }
        }

        // Get keyboard state for continuous movement
        Uint8 *keystate = SDL_GetKeyState(NULL);
        
        // Handle P1 crouch, dash, and slide
        if (!joueur1.is_attacking) {
            if (keystate[P1_CROUCH]) {
                joueur1.is_crouching = 1;
                joueur1.anim.state = PLAYER_CROUCH;  // Set animation state
                joueur1.vitesse = 6;
            } else {
                joueur1.is_crouching = 0;
                joueur1.vitesse = 12;
            }
            
            if (keystate[P1_DASH] && joueur1.dash_cooldown <= 0 && !joueur1.is_sliding) {
                joueur1.is_dashing = 1;
                joueur1.anim.state = PLAYER_DASH;  // Set animation state
                joueur1.dash_cooldown = 30;  // Shorter cooldown
                if (strcmp(joueur1.direction, "right") == 0) {
                    joueur1.x += 30;  // Immediate dash movement
                } else {
                    joueur1.x -= 30;
                }
            }
            
            if (keystate[P1_SLIDE] && !joueur1.is_sliding && joueur1.slide_cooldown <= 0) {
                joueur1.is_sliding = 1;
                joueur1.slide_cooldown = 30;  // Reduced from 45 to 30 for faster slide recovery
                joueur1.slide_velocity = strcmp(joueur1.direction, "right") == 0 ? 20.0f : -20.0f;  // Increased from 15 to 20
            }
        }
        
        // Handle P2 crouch and dash with matching logic
        if (!joueur2.is_attacking) {
            if (keystate[P2_CROUCH]) {
                joueur2.is_crouching = 1;
                joueur2.anim.state = PLAYER_CROUCH;
                joueur2.vitesse = 6;
            } else {
                joueur2.is_crouching = 0;
                joueur2.vitesse = 12;
            }
            
            if (keystate[P2_DASH] && joueur2.dash_cooldown <= 0 && !joueur2.is_sliding) {
                joueur2.is_dashing = 1;
                joueur2.anim.state = PLAYER_DASH;
                joueur2.dash_cooldown = 30;
                if (strcmp(joueur2.direction, "right") == 0) {
                    joueur2.x += 30;
                } else {
                    joueur2.x -= 30;
                }
            }
            
            if (keystate[P2_SLIDE] && !joueur2.is_sliding && joueur2.slide_cooldown <= 0) {
                joueur2.is_sliding = 1;
                joueur2.slide_cooldown = 30;
                joueur2.slide_velocity = strcmp(joueur2.direction, "right") == 0 ? 20.0f : -20.0f;
            }
        }
        
        // Update movement for both players
        if (!joueur1.is_attacking && !joueur1.is_dashing) {  // Don't move if dashing
            if (keystate[P1_LEFT] && !joueur1.is_sliding) {
                joueur1.x -= joueur1.vitesse;
                strcpy(joueur1.direction, "left");
                joueur1.moving_left = 1;
                joueur1.moving_right = 0;
            } else if (keystate[P1_RIGHT] && !joueur1.is_sliding) {
                joueur1.x += joueur1.vitesse;
                strcpy(joueur1.direction, "right");
                joueur1.moving_right = 1;
                joueur1.moving_left = 0;
            } else if (!joueur1.is_sliding) {
                joueur1.moving_left = 0;
                joueur1.moving_right = 0;
            }
        }

        if (!joueur2.is_attacking && !joueur2.is_dashing) {  // Don't move if dashing
            if (keystate[P2_LEFT] && !joueur2.is_sliding) {
                joueur2.x -= joueur2.vitesse;
                strcpy(joueur2.direction, "left");
                joueur2.moving_left = 1;
                joueur2.moving_right = 0;
            } else if (keystate[P2_RIGHT] && !joueur2.is_sliding) {
                joueur2.x += joueur2.vitesse;
                strcpy(joueur2.direction, "right");
                joueur2.moving_right = 1;
                joueur2.moving_left = 0;
            } else if (!joueur2.is_sliding) {
                joueur2.moving_left = 0;
                joueur2.moving_right = 0;
            }
        }
        
        // Apply sliding movement
        if (joueur1.is_sliding) {
            joueur1.x += joueur1.slide_velocity;
            joueur1.slide_velocity *= 0.95f; // Gradually slow down
            if (fabs(joueur1.slide_velocity) < 0.5f) {
                joueur1.is_sliding = 0;
            }
        }
        
        if (joueur2.is_sliding) {
            joueur2.x += joueur2.slide_velocity;
            joueur2.slide_velocity *= 0.95f;
            if (fabs(joueur2.slide_velocity) < 0.5f) {
                joueur2.is_sliding = 0;
            }
        }
        
        // Update cooldowns
        if (joueur1.dash_cooldown > 0) joueur1.dash_cooldown--;
        if (joueur2.dash_cooldown > 0) joueur2.dash_cooldown--;
        if (joueur1.slide_cooldown > 0) joueur1.slide_cooldown--;
        if (joueur2.slide_cooldown > 0) joueur2.slide_cooldown--;

        // Apply physics to both players
        // Player 1
        joueur1.velocity_y += GRAVITY;
        joueur1.y += joueur1.velocity_y;
        if (joueur1.y > GROUND_LEVEL - 100) {
            joueur1.y = GROUND_LEVEL - 100;
            joueur1.velocity_y = 0;
            joueur1.is_jumping = 0;
        }
        if (keystate[P1_JUMP] && !joueur1.is_jumping) {
            joueur1.velocity_y = -15;  // Increased from -12 to compensate for higher gravity
            joueur1.is_jumping = 1;
        }

        // Player 2
        joueur2.velocity_y += GRAVITY;
        joueur2.y += joueur2.velocity_y;
        if (joueur2.y > GROUND_LEVEL - 100) {
            joueur2.y = GROUND_LEVEL - 100;
            joueur2.velocity_y = 0;
            joueur2.is_jumping = 0;
        }
        if (keystate[P2_JUMP] && !joueur2.is_jumping) {
            joueur2.velocity_y = -15;  // Increased from -12 to match P1
            joueur2.is_jumping = 1;
        }

        // Screen boundaries for both players
        // Player 1 (left viewport)
        if (joueur1.x < 0) joueur1.x = 0;
        if (joueur1.x > VIEWPORT_WIDTH - 145) joueur1.x = VIEWPORT_WIDTH - 145;
        if (joueur1.y < 0) joueur1.y = 0;
        if (joueur1.y > SCREEN_HEIGHT - 100) joueur1.y = SCREEN_HEIGHT - 100;

        // Player 2 (right viewport)
        if (joueur2.x < VIEWPORT_WIDTH) joueur2.x = VIEWPORT_WIDTH;
        if (joueur2.x > SCREEN_WIDTH - 145) joueur2.x = SCREEN_WIDTH - 145;
        if (joueur2.y < 0) joueur2.y = 0;
        if (joueur2.y > SCREEN_HEIGHT - 100) joueur2.y = SCREEN_HEIGHT - 100;
        
        // Draw split screen
        // Left viewport (Player 1)
        SDL_BlitSurface(scaled_background, &viewport1, ecran, &viewport1);
        update_animation_personnage(&joueur1);
        afficher_personnage(&joueur1);
        afficher_interface_jeu(joueur1.vie, joueur1.score);
        
        // Right viewport (Player 2)
        SDL_BlitSurface(scaled_background, &viewport2, ecran, &viewport2);
        update_animation_personnage(&joueur2);
        afficher_personnage(&joueur2);
        afficher_interface_jeu(joueur2.vie, joueur2.score);
        
        // Draw ground line for both viewports
        SDL_Rect ground1 = {0, GROUND_LEVEL, VIEWPORT_WIDTH, 2};
        SDL_Rect ground2 = {VIEWPORT_WIDTH, GROUND_LEVEL, VIEWPORT_WIDTH, 2};
        SDL_FillRect(ecran, &ground1, SDL_MapRGB(ecran->format, 100, 100, 100));
        SDL_FillRect(ecran, &ground2, SDL_MapRGB(ecran->format, 100, 100, 100));

        // Draw split line
        SDL_Rect split_line = {VIEWPORT_WIDTH - 2, 0, 4, SCREEN_HEIGHT};
        SDL_FillRect(ecran, &split_line, SDL_MapRGB(ecran->format, 255, 255, 255));

        // Update screen
        SDL_Flip(ecran);
        
        // Check player death conditions
        if (joueur1.vie <= 0 || joueur2.vie <= 0) {
            continuer = 0;
        }
    }
    
    // Cleanup
    SDL_FreeSurface(scaled_background);
    liberer_animations_personnage(&joueur1);
    liberer_animations_personnage(&joueur2);
    SDL_Quit();
    
    return EXIT_SUCCESS;
}
