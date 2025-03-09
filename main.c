#include "header.h"

int main(int argc __attribute__((unused)), char* argv[] __attribute__((unused))) {
    // Initialiser SDL
    if (!initialiserSDL()) {
        fprintf(stderr, "Erreur lors de l'initialisation de SDL\n");
        return 1;
    }
    
    // Créer et initialiser le jeu
    Jeu jeu;
    if (!initialiserJeu(&jeu)) {
        fprintf(stderr, "Erreur lors de l'initialisation du jeu\n");
        return 1;
    }
    
    // Afficher le menu principal
    afficherMenuPrincipal(&jeu);
    
    // Gérer les événements
    gererEvenements(&jeu);
    
    // Libérer les ressources
    libererJeu(&jeu);
    
    return 0;
}
