#ifndef HEADER_H
#define HEADER_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

// Définition des états du menu
typedef enum {
    MENU_PRINCIPAL,
    MENU_OPTIONS,
    MENU_SAUVEGARDE,
    MENU_JOUEUR,
    MENU_MEILLEURS_SCORES,
    MENU_ENIGME,
    MENU_QUIZ,
    MENU_PUZZLE,
    MENU_SELECTION_JEU,  // Add this new state
    MENU_SELECTION_PERSONNAGE,  // Add this new state
    JEU_EN_COURS,
    QUITTER
} EtatMenu;

// Structure pour les boutons
typedef struct {
    SDL_Rect rect;
    SDL_Surface* surface;
    SDL_Surface* surfaceHover;
    bool hover;
    char* nom;
} Bouton;

// Structure pour les scores
typedef struct {
    char nom[50];
    int score;
} Score;

// Add new struct for character selection
typedef struct {
    bool apparence1Selected;
    bool apparence2Selected;
    bool input1Selected;
    bool input2Selected;
} SelectionPersonnage;

// Structure principale du jeu
typedef struct {
    SDL_Surface* ecran;
    SDL_Surface* arrierePlan1;
    SDL_Surface* arrierePlan2;
    SDL_Surface* arrierePlan3;
    SDL_Surface* arrierePlan4;
    SDL_Surface* logo;
    
    Mix_Music* musiqueMenu;
    Mix_Music* musiqueOptions;
    Mix_Music* musiqueVictoire;
    Mix_Music* musiqueSuspense;
    
    Mix_Chunk* sonBouton;
    Mix_Chunk* sonClick;
    
    TTF_Font* police;
    
    Bouton* boutons;
    int nbBoutons;
    
    int volumeActuel;
    bool pleinEcran;
    
    EtatMenu menuPrecedent; // Add this new member
    EtatMenu etatActuel;
    
    Score meilleursScores[3];
    SelectionPersonnage selectionPerso; // Add this new member
    char inputText[50]; // Add this new member
    bool isInputActive; // Add this new member
    SDL_Surface* textInputBox; // Add this new member
} Jeu;

// Fonctions d'initialisation
bool initialiserSDL();
bool initialiserJeu(Jeu* jeu);
void libererJeu(Jeu* jeu);

// Fonctions de gestion des boutons
Bouton creerBouton(int x, int y, const char* cheminImage, const char* cheminImageHover, const char* nom);
void dessinerBouton(Bouton bouton, SDL_Surface* ecran);
bool estSurBouton(Bouton bouton, int x, int y);
void libererBouton(Bouton* bouton);

// Fonctions de gestion des menus
void afficherMenuPrincipal(Jeu* jeu);
void afficherMenuOptions(Jeu* jeu);
void afficherMenuSauvegarde(Jeu* jeu);
void afficherMenuJoueur(Jeu* jeu);
void afficherMenuMeilleursScores(Jeu* jeu);
void afficherMenuEnigme(Jeu* jeu);
void afficherMenuQuiz(Jeu* jeu);
void afficherMenuPuzzle(Jeu* jeu);
void afficherMenuSelectionJeu(Jeu* jeu); // Add new function prototype
void afficherMenuSelectionPersonnage(Jeu* jeu); // Add new function prototype
void afficherScoresFinaux(Jeu* jeu); // Add new function prototype

// Fonctions de gestion des événements
void gererEvenements(Jeu* jeu);

// Fonctions utilitaires
void changerVolume(Jeu* jeu, int delta);
void basculerPleinEcran(Jeu* jeu);
void sauvegarderScore(Jeu* jeu, const char* nom, int score);
void chargerMeilleursScores(Jeu* jeu);
void mettreAJourBarreVolume(Jeu* jeu);

void redessinerMenuPrincipal(Jeu* jeu);
void redessinerMenuOptions(Jeu* jeu);
void redessinerMenuSauvegarde(Jeu* jeu);
void redessinerMenuJoueur(Jeu* jeu);
void redessinerMenuMeilleursScores(Jeu* jeu);
void redessinerMenuEnigme(Jeu* jeu);
void redessinerMenuQuiz(Jeu* jeu);
void redessinerMenuPuzzle(Jeu* jeu);
void redessinerMenuSelectionJeu(Jeu* jeu); // Add new function prototype
void redessinerMenuSelectionPersonnage(Jeu* jeu); // Add new function prototype

#endif // HEADER_H
