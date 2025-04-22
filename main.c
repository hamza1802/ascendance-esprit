#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_ENNEMIS 5  // Changed from 10 to 3

int main(int argc, char *argv[]) {
    // Initialisation de la SDL et ses extensions
    initialiser_SDL();
    
    // Création de la fenêtre et configuration de l'écran
    SDL_Surface *ecran = SDL_SetVideoMode(1280, 720, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!ecran) {
        printf("Erreur lors de la création de la fenêtre: %s\n", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    SDL_WM_SetCaption("Jeu avec Ennemis", NULL);
    
    // Load background
    SDL_Surface *background = IMG_Load("resources/background.png");
    if (!background) {
        printf("Erreur de chargement du background: %s\n", IMG_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    // Scale background to screen size if needed
    SDL_Surface *scaled_background = SDL_CreateRGBSurface(SDL_HWSURFACE, 
        1280, 720, 32, 0, 0, 0, 0);
    SDL_SoftStretch(background, NULL, scaled_background, NULL);
    SDL_FreeSurface(background);
    
    // Initialisation du personnage
    Personnage joueur;
    initialiser_personnage(&joueur, "resources/player/player.png");
    
    // Initialisation des ennemis
    Enemy ennemis[MAX_ENNEMIS];
    int nb_ennemis_niveau1 = 2;  // Reduced from 3 to 2
    int nb_ennemis_niveau2 = 3;  // Reduced from 2 to 1
    
    // Créer des ennemis de niveau 1
    initialiser_ennemis(ennemis, nb_ennemis_niveau1, 1);
    
    // Créer des ennemis de niveau 2
    initialiser_ennemis(&ennemis[nb_ennemis_niveau1], nb_ennemis_niveau2, 2);
    
    int total_ennemis = nb_ennemis_niveau1 + nb_ennemis_niveau2;
    
    // After enemy initialization
    Boss boss;
    initialiser_boss(&boss);
    
    // Variables pour la gestion des événements et la boucle de jeu
    SDL_Event evenement;
    int continuer = 1;
    int dernierTemps = 0, tempsActuel = 0;
    
    // Boucle principale du jeu
    while (continuer) {
        tempsActuel = SDL_GetTicks();
        
        // Limiter à 60 FPS (environ 16.7ms par frame)
        if (tempsActuel - dernierTemps < 16)
            continue;
            
        dernierTemps = tempsActuel;

        // Update hitboxes
        joueur.hitbox.x = joueur.x + 30;
        joueur.hitbox.y = joueur.y + 10;

        // Update invulnerability timer
        if (joueur.invulnerability_timer > 0) {
            joueur.invulnerability_timer--;
        }
        
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
                        case SDLK_k:  // Attack key
                            if (!joueur.is_attacking && joueur.anim.state != PLAYER_ATTACK) {
                                joueur.anim.state = PLAYER_ATTACK;
                                joueur.anim.current_frame = 0;
                                joueur.frame_timer = 0;
                                joueur.is_attacking = 1;
                            }
                            break;
                        case SDLK_p:  // Debug key to damage boss
                            if (boss.is_active && boss.health > 0) {
                                boss.health -= boss.max_health / 3;
                                if (boss.health < 0) boss.health = 0;
                                printf("Debug: Reduced boss health to %d\n", boss.health);
                            }
                            break;
                    }
                    break;
            }
        }

        // Get keyboard state for continuous movement
        Uint8 *keystate = SDL_GetKeyState(NULL);
        
        // Apply gravity and update player position
        joueur.velocity_y += GRAVITY;
        joueur.y += joueur.velocity_y;

        // Ground collision
        if (joueur.y > GROUND_LEVEL - 100) {  // Changed from 64 to 100
            joueur.y = GROUND_LEVEL - 100;
            joueur.velocity_y = 0;
            joueur.is_jumping = 0;
        }

        // Update player movement flags - only if not attacking
        if (!joueur.is_attacking) {
            int new_x = joueur.x;
            if (keystate[SDLK_LEFT]) {
                new_x = joueur.x - joueur.vitesse;
                strcpy(joueur.direction, "left");
                joueur.moving_left = 1;
                joueur.moving_right = 0;
            } else if (keystate[SDLK_RIGHT]) {
                new_x = joueur.x + joueur.vitesse;
                strcpy(joueur.direction, "right");
                joueur.moving_right = 1;
                joueur.moving_left = 0;
            } else {
                joueur.moving_left = 0;
                joueur.moving_right = 0;
            }

            // Check collision with enemies before applying movement
            int can_move = 1;
            for (int i = 0; i < total_ennemis; i++) {
                if (ennemis[i].state != ENEMY_NEUTRALIZED && 
                    detecter_collision_position(&joueur, new_x, joueur.y, &ennemis[i])) {
                    can_move = 0;
                    break;
                }
            }

            if (can_move) {
                joueur.x = new_x;
            }
        }
        
        // Handle jumping with spacebar (moved from UP to SPACE)
        if (keystate[SDLK_SPACE] && !joueur.is_jumping) {
            joueur.velocity_y = -12;  // Jump velocity
            joueur.is_jumping = 1;
        }

        // Empêcher le joueur de sortir de l'écran
        if (joueur.x < 0) joueur.x = 0;
        if (joueur.x > 1280 - 145) joueur.x = 1280 - 145;  // Changed from 64 to 145
        if (joueur.y < 0) joueur.y = 0;
        if (joueur.y > 720 - 100) joueur.y = 720 - 100;    // Changed from 64 to 100
        
        // Mise à jour des ennemis
        for (int i = 0; i < total_ennemis; i++) {
            // Update enemy hitbox
            ennemis[i].hitbox.x = ennemis[i].x + 15;
            ennemis[i].hitbox.y = ennemis[i].y + 5;
            
            // Check for player's attack hitting enemies
            if (detecter_collision_joueur_ennemi(&joueur, &ennemis[i])) {
                ennemis[i].health -= 20;
                
                // Apply enemy knockback
                float knockback_force = 8.0f;
                ennemis[i].knockback_velocity_x = strcmp(joueur.direction, "right") == 0 ? 
                    knockback_force : -knockback_force;
                
                printf("Enemy hit! Health: %d\n", ennemis[i].health);
            }
            
            // Enemy knockback movement
            if (ennemis[i].knockback_velocity_x != 0) {
                int new_x = ennemis[i].x + (int)ennemis[i].knockback_velocity_x;
                // Check if new position is valid
                int can_move = 1;
                for (int j = 0; j < total_ennemis; j++) {
                    if (i != j && ennemis[j].state != ENEMY_NEUTRALIZED) {
                        Enemy temp = ennemis[i];
                        temp.x = new_x;
                        if (detecter_collision_entre_ennemis(&temp, &ennemis[j])) {
                            can_move = 0;
                            break;
                        }
                    }
                }
                if (can_move) {
                    ennemis[i].x = new_x;
                }
                // Gradually reduce knockback
                ennemis[i].knockback_velocity_x *= 0.8f;
                if (fabs(ennemis[i].knockback_velocity_x) < 0.1f) {
                    ennemis[i].knockback_velocity_x = 0;
                }
            }
            
            // Détection de collision entre le joueur et les attaques des ennemis
            if (joueur.invulnerability_timer == 0 && 
                detecter_collision_attaque_ennemi(&joueur, &ennemis[i])) {
                joueur.vie -= 5;
                joueur.invulnerability_timer = 60; // 1 second invulnerability
                joueur.anim.state = PLAYER_HURT;
                joueur.frame_timer = 0;
                joueur.is_attacking = 0; // Cancel attack when hurt
                printf("Le joueur a été touché! Vie restante: %d\n", joueur.vie);
            }
            
            // IA pour le mouvement des ennemis
            ia_mouvement_ennemi(&ennemis[i], &joueur);
        }
        
        // Check if all enemies are defeated
        int all_enemies_defeated = 1;
        for (int i = 0; i < total_ennemis; i++) {
            if (ennemis[i].state != ENEMY_NEUTRALIZED) {
                all_enemies_defeated = 0;
                break;
            }
        }
        
        if (all_enemies_defeated && !boss.is_active) {
            boss.is_active = 1;
        }
        
        // Animation des ennemis
        animer_ennemis(ennemis, total_ennemis);
        
        // Gestion de la santé des ennemis
        gerer_sante_ennemis(ennemis, total_ennemis);
        
        // Effacer l'écran et dessiner le background
        SDL_BlitSurface(scaled_background, NULL, ecran, NULL);
        
        // Afficher le joueur
        afficher_personnage(&joueur);
        
        // Update player animation
        update_animation_personnage(&joueur);
        
        // Afficher les ennemis
        afficher_ennemis(ennemis, total_ennemis);
        
        // Afficher le boss
        if (boss.is_active) {
            update_boss(&boss, &joueur);
            
            // Check for player attack hitting boss
            if (joueur.is_attacking && detecter_collision_joueur_boss(&joueur, &boss)) {
                boss.health -= 5;  // Boss takes less damage
                printf("Boss hit! Health: %d\n", boss.health);
            }
            
            // Check for player collision with boss body
            if (detecter_collision_joueur_boss_body(&joueur, &boss)) {
                joueur.vie -= 10;  // Boss does more damage than regular enemies
                joueur.invulnerability_timer = 90; // 1.5 seconds invulnerability
                joueur.anim.state = PLAYER_HURT;
                joueur.frame_timer = 0;
                joueur.is_attacking = 0; // Cancel attack when hurt
                
                // Strong knockback when hit by boss
                float knockback_force = 7.0f;  // Reduced from 15.0f to 7.0f
                if (joueur.x < boss.x) {
                    joueur.knockback_velocity_x = -knockback_force;
                } else {
                    joueur.knockback_velocity_x = knockback_force;
                }
                joueur.knockback_timer = 20;
                
                printf("Le joueur a été touché par le boss! Vie restante: %d\n", joueur.vie);
            }
            
            afficher_boss(&boss);
        }
        
        // Afficher l'interface du jeu (vie, score)
        afficher_interface_jeu(joueur.vie, joueur.score);
        
        // Draw ground line
        SDL_Rect ground = {0, GROUND_LEVEL, 1280, 2};
        SDL_FillRect(ecran, &ground, SDL_MapRGB(ecran->format, 100, 100, 100));

        // Mise à jour de l'écran
        SDL_Flip(ecran);
        
        // Vérifier si le joueur est mort
        if (joueur.vie <= 0) {
            // Trigger death animation
            joueur.anim.state = PLAYER_DEATH;
            joueur.frame_timer = 0;
            joueur.anim.current_frame = 0;
            
            // Create fade surface
            SDL_Surface *fade = SDL_CreateRGBSurface(SDL_HWSURFACE, 1280, 720, 32, 0, 0, 0, 0);
            SDL_FillRect(fade, NULL, SDL_MapRGB(fade->format, 0, 0, 0));
            
            // Load font
            TTF_Font *font = TTF_OpenFont("resources/font.ttf", 70);
            if (!font) {
                printf("Font loading error: %s\n", TTF_GetError());
            }
            
            SDL_Color red = {255, 0, 0};
            SDL_Surface *text = TTF_RenderText_Blended(font, "DEAD", red);
            
            // Play death animation and fade
            int alpha = 0;
            while (alpha < 255) {
                SDL_BlitSurface(scaled_background, NULL, ecran, NULL);
                afficher_personnage(&joueur);
                if (joueur.anim.current_frame < joueur.anim.frame_counts[PLAYER_DEATH]) {
                    update_animation_personnage(&joueur);
                }
                
                // Apply fade with increasing opacity
                SDL_SetAlpha(fade, SDL_SRCALPHA, alpha);
                SDL_BlitSurface(fade, NULL, ecran, NULL);
                
                // Draw "DEAD" text centered
                if (alpha > 128) {  // Show text when screen is darker
                    SDL_Rect textPos;
                    textPos.x = (1280 - text->w) / 2;
                    textPos.y = (720 - text->h) / 2;
                    SDL_BlitSurface(text, NULL, ecran, &textPos);
                }
                
                SDL_Flip(ecran);
                SDL_Delay(50);  // Control fade speed
                alpha += 5;
            }
            
            SDL_Delay(2000);  // Keep final state for 2 seconds
            SDL_FreeSurface(fade);
            SDL_FreeSurface(text);
            TTF_CloseFont(font);
            continuer = 0;
        }
    }
    
    // Libération des ressources
    for (int i = 0; i < total_ennemis; i++) {
        for (int j = 0; j < 4; j++) {
            if (ennemis[i].images[j])
                SDL_FreeSurface(ennemis[i].images[j]);
        }
    }
    
    SDL_FreeSurface(scaled_background);
    // Remove this line since we don't use joueur.image anymore:
    // SDL_FreeSurface(joueur.image);
    liberer_animations_personnage(&joueur);
    SDL_FreeSurface(boss.sprite_L);
    SDL_FreeSurface(boss.sprite_R);
    SDL_Quit();
    
    return EXIT_SUCCESS;
}
