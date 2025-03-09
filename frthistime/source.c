#include "header.h"

bool initialiserSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Erreur lors de l'initialisation de SDL: %s\n", SDL_GetError());
        return false;
    }
    
    SDL_EnableUNICODE(1);
    
    if (TTF_Init() < 0) {
        fprintf(stderr, "Erreur lors de l'initialisation de SDL_ttf: %s\n", TTF_GetError());
        return false;
    }
    
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "Erreur lors de l'initialisation de SDL_mixer: %s\n", Mix_GetError());
        return false;
    }
    
    return true;
}

bool initialiserJeu(Jeu* jeu) {
    // Initialisation des valeurs par défaut
    jeu->ecran = NULL;
    jeu->arrierePlan1 = NULL;
    jeu->arrierePlan2 = NULL;
    jeu->arrierePlan3 = NULL;
    jeu->arrierePlan4 = NULL;
    jeu->musiqueMenu = NULL;
    jeu->musiqueOptions = NULL;
    jeu->musiqueVictoire = NULL;
    jeu->musiqueSuspense = NULL;
    jeu->sonBouton = NULL;
    jeu->sonClick = NULL;
    jeu->police = NULL;
    jeu->boutons = NULL;
    jeu->nbBoutons = 0;
    jeu->volumeActuel = MIX_MAX_VOLUME;
    jeu->pleinEcran = false;
    jeu->etatActuel = MENU_PRINCIPAL;
    jeu->menuPrecedent = MENU_PRINCIPAL;
    
    // Création de la fenêtre
    jeu->ecran = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!jeu->ecran) {
        fprintf(stderr, "Erreur lors de la création de la fenêtre: %s\n", SDL_GetError());
        return false;
    }
    
    SDL_WM_SetCaption("Mon Jeu", NULL);
    
    // Chargement des arrière-plans
    jeu->arrierePlan1 = IMG_Load("assets/background.png");
    jeu->arrierePlan2 = IMG_Load("assets/background2.png");
    jeu->arrierePlan3 = IMG_Load("assets/background3.png");
    jeu->arrierePlan4 = IMG_Load("assets/background4.png");
    
    if (!jeu->arrierePlan1 || !jeu->arrierePlan2 || !jeu->arrierePlan3 || !jeu->arrierePlan4) {
        fprintf(stderr, "Erreur lors du chargement des arrière-plans: %s\n", IMG_GetError());
        return false;
    }
    
    // Chargement du logo
    jeu->logo = IMG_Load("assets/logo.png");
    if (!jeu->logo) {
        fprintf(stderr, "Erreur lors du chargement du logo: %s\n", IMG_GetError());
        // Ne pas retourner false ici, ce n'est pas critique
    }
    
    // Chargement des musiques
    jeu->musiqueMenu = Mix_LoadMUS("assets/backgroundsong1.mp3");
    jeu->musiqueOptions = Mix_LoadMUS("assets/backgroundsong1.mp3"); // À remplacer par la musique appropriée
    jeu->musiqueVictoire = Mix_LoadMUS("assets/backgroundsong1.mp3"); // À remplacer par la musique appropriée
    jeu->musiqueSuspense = Mix_LoadMUS("assets/backgroundsong1.mp3"); // À remplacer par la musique appropriée
    
    if (!jeu->musiqueMenu) {
        fprintf(stderr, "Erreur lors du chargement des musiques: %s\n", Mix_GetError());
        return false;
    }
    
    // Chargement des sons
    jeu->sonBouton = Mix_LoadWAV("assets/hover.wav");
    jeu->sonClick = Mix_LoadWAV("assets/click.wav");
    
    if (!jeu->sonBouton || !jeu->sonClick) {
        fprintf(stderr, "Erreur lors du chargement des sons: %s\n", Mix_GetError());
        return false;
    }
    
    // Chargement de la police
    jeu->police = TTF_OpenFont("assets/font.ttf", 24);
    if (!jeu->police) {
        fprintf(stderr, "Erreur lors du chargement de la police: %s\n", TTF_GetError());
        return false;
    }
    
    // Initialisation du volume
    jeu->volumeActuel = MIX_MAX_VOLUME;
    Mix_VolumeMusic(jeu->volumeActuel);
    Mix_Volume(-1, jeu->volumeActuel);
    
    // Chargement des meilleurs scores
    chargerMeilleursScores(jeu);

    jeu->inputText[0] = '\0';
    jeu->isInputActive = false;
    jeu->textInputBox = NULL;
    
    return true;
}

void libererJeu(Jeu* jeu) {
    // Libération des surfaces
    if (jeu->arrierePlan1) SDL_FreeSurface(jeu->arrierePlan1);
    if (jeu->arrierePlan2) SDL_FreeSurface(jeu->arrierePlan2);
    if (jeu->arrierePlan3) SDL_FreeSurface(jeu->arrierePlan3);
    if (jeu->arrierePlan4) SDL_FreeSurface(jeu->arrierePlan4);
    if (jeu->logo) SDL_FreeSurface(jeu->logo);
    
    // Libération des musiques
    if (jeu->musiqueMenu) Mix_FreeMusic(jeu->musiqueMenu);
    if (jeu->musiqueOptions) Mix_FreeMusic(jeu->musiqueOptions);
    if (jeu->musiqueVictoire) Mix_FreeMusic(jeu->musiqueVictoire);
    if (jeu->musiqueSuspense) Mix_FreeMusic(jeu->musiqueSuspense);
    
    // Libération des sons
    if (jeu->sonBouton) Mix_FreeChunk(jeu->sonBouton);
    if (jeu->sonClick) Mix_FreeChunk(jeu->sonClick);
    
    // Libération de la police
    if (jeu->police) TTF_CloseFont(jeu->police);
    
    // Libération des boutons
    for (int i = 0; i < jeu->nbBoutons; i++) {
        libererBouton(&jeu->boutons[i]);
    }
    free(jeu->boutons);
    
    // Fermeture des bibliothèques
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
}

Bouton creerBouton(int x, int y, const char* cheminImage, const char* cheminImageHover, const char* nom) {
    Bouton bouton;
    bouton.rect.x = x;
    bouton.rect.y = y;
    bouton.surface = IMG_Load(cheminImage);
    bouton.surfaceHover = IMG_Load(cheminImageHover);
    bouton.hover = false;
    bouton.nom = strdup(nom);
    
    if (bouton.surface) {
        bouton.rect.w = bouton.surface->w;
        bouton.rect.h = bouton.surface->h;
    }
    
    return bouton;
}

void dessinerBouton(Bouton bouton, SDL_Surface* ecran) {
    SDL_Surface* surfaceADessiner = bouton.hover ? bouton.surfaceHover : bouton.surface;
    if (surfaceADessiner) {
        SDL_BlitSurface(surfaceADessiner, NULL, ecran, &bouton.rect);
    }
}

bool estSurBouton(Bouton bouton, int x, int y) {
    return (x >= bouton.rect.x && x < bouton.rect.x + bouton.rect.w &&
            y >= bouton.rect.y && y < bouton.rect.y + bouton.rect.h);
}

void libererBouton(Bouton* bouton) {
    if (bouton->surface) SDL_FreeSurface(bouton->surface);
    if (bouton->surfaceHover) SDL_FreeSurface(bouton->surfaceHover);
    free(bouton->nom);
}

void effacerEcran(SDL_Surface* ecran) {
    SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
}

void afficherMenuPrincipal(Jeu* jeu) {
    // Clear screen first
    effacerEcran(jeu->ecran);
    
    // Arrêter toute musique en cours
    Mix_HaltMusic();
    
    // Jouer la musique du menu principal
    Mix_PlayMusic(jeu->musiqueMenu, -1);
    
    // Afficher l'arrière-plan
    SDL_BlitSurface(jeu->arrierePlan1, NULL, jeu->ecran, NULL);
    
    // Afficher le logo
    if (jeu->logo) {
        SDL_Rect positionLogo = {50, 100, 0, 0}; // Position à gauche
        SDL_BlitSurface(jeu->logo, NULL, jeu->ecran, &positionLogo);
    }
    
    // Libérer les boutons précédents
    for (int i = 0; i < jeu->nbBoutons; i++) {
        libererBouton(&jeu->boutons[i]);
    }
    free(jeu->boutons);
    
    // Créer les boutons du menu principal
    jeu->nbBoutons = 5;
    jeu->boutons = malloc(jeu->nbBoutons * sizeof(Bouton));
    
    // Placer les boutons à droite avec un espacement vertical de 100 pixels
    int yOffset = 150;
    int xOffset = 500;
    
    // Bouton Play (Normal - 180x80)
    jeu->boutons[0] = creerBouton(xOffset, yOffset, "assets/play.png", "assets/highlighted/play.png", "jouer");
    
    // Bouton Settings (Normal - 180x80)
    jeu->boutons[1] = creerBouton(xOffset, yOffset + 100, "assets/settings.png", "assets/highlighted/settings.png", "options");
    
    // Bouton Story (Normal - 180x80)
    jeu->boutons[2] = creerBouton(xOffset, yOffset + 200, "assets/story.png", "assets/highlighted/story.png", "histoire");
    
    // Bouton Best Score (Wide - 300x80)
    jeu->boutons[3] = creerBouton(xOffset - 60, yOffset + 300, "assets/best_score.png", "assets/highlighted/best_score.png", "meilleurs_scores");
    
    // Bouton Quit (Normal - 180x80)
    jeu->boutons[4] = creerBouton(xOffset, yOffset + 400, "assets/quit.png", "assets/highlighted/quit.png", "quitter");
    
    // Dessiner les boutons
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
    
    // Mettre à jour l'écran
    SDL_Flip(jeu->ecran);
}

void afficherMenuOptions(Jeu* jeu) {
    // Clear screen first
    effacerEcran(jeu->ecran);
    
    // Sauvegarder l'état actuel de la musique
    int musiqueEnCours = Mix_PlayingMusic();
    
    // Si aucune musique n'est en cours, démarrer la musique du menu options
    if (!musiqueEnCours) {
        Mix_HaltMusic();
        Mix_PlayMusic(jeu->musiqueOptions, -1);
    }
    
    // Afficher l'arrière-plan du menu options
    SDL_BlitSurface(jeu->arrierePlan2, NULL, jeu->ecran, NULL);
    
    // Libérer les boutons précédents
    for (int i = 0; i < jeu->nbBoutons; i++) {
        libererBouton(&jeu->boutons[i]);
    }
    free(jeu->boutons);
    
    // Créer les boutons du menu options
    jeu->nbBoutons = 4;
    jeu->boutons = malloc(jeu->nbBoutons * sizeof(Bouton));
    
    // Afficher les étiquettes comme des images statiques
    SDL_Surface* volumeLabel = IMG_Load("assets/volume.png");
    if (volumeLabel) {
        SDL_Rect positionVolume = {90, 200, 0, 0};
        SDL_BlitSurface(volumeLabel, NULL, jeu->ecran, &positionVolume);
        SDL_FreeSurface(volumeLabel);
    }
    
    SDL_Surface* displayModeLabel = IMG_Load("assets/display mode.png");
    if (displayModeLabel) {
        SDL_Rect positionDisplayMode = {90, 400, 0, 0};
        SDL_BlitSurface(displayModeLabel, NULL, jeu->ecran, &positionDisplayMode);
        SDL_FreeSurface(displayModeLabel);
    }
    
    // Afficher la barre de volume
    mettreAJourBarreVolume(jeu);
    
    // Boutons de contrôle du volume (Small - 160x80)
    jeu->boutons[0] = creerBouton(1000, 200, "assets/right.png", "assets/highlighted/right.png", "augmenter");
    jeu->boutons[1] = creerBouton(540, 200, "assets/left.png", "assets/highlighted/left.png", "diminuer");
    
    // Boutons de mode d'affichage
    if (jeu->pleinEcran) {
        // Bouton Normal (Normal - 180x80)
        jeu->boutons[2] = creerBouton(980, 400, "assets/normal.png", "assets/highlighted/normal.png", "normal");
    } else {
        // Bouton Fullscreen (Wide - 300x80)
        jeu->boutons[2] = creerBouton(540, 400, "assets/fullscreen.png", "assets/highlighted/fullscreen.png", "pleinecran");
    }
    
    // Bouton retour (Normal - 180x80)
    jeu->boutons[3] = creerBouton(950, 600, "assets/return.png", "assets/highlighted/return.png", "retour");
    
    // Dessiner les boutons
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
    
    // Mettre à jour l'écran
    SDL_Flip(jeu->ecran);
}

void afficherMenuSauvegarde(Jeu* jeu) {
    // Clear screen first
    effacerEcran(jeu->ecran);
    
    // Arrêter toute musique en cours
    Mix_HaltMusic();
    
    // Jouer la musique du menu sauvegarde
    Mix_PlayMusic(jeu->musiqueOptions, -1);
    
    // Afficher l'arrière-plan
    SDL_BlitSurface(jeu->arrierePlan2, NULL, jeu->ecran, NULL);
    
    // Libérer les boutons précédents
    for (int i = 0; i < jeu->nbBoutons; i++) {
        libererBouton(&jeu->boutons[i]);
    }
    free(jeu->boutons);
    
    // Créer les boutons du menu sauvegarde
    jeu->nbBoutons = 3;
    jeu->boutons = malloc(jeu->nbBoutons * sizeof(Bouton));
    
    // Afficher le message "Voulez-vous sauvegarder votre jeu"
    SDL_Color couleurTexte = {255, 255, 255, 0};
    SDL_Surface* texte = TTF_RenderText_Blended(jeu->police, "Voulez-vous sauvegarder votre jeu ?", couleurTexte);
    SDL_Rect positionTexte = {200, 150, 0, 0};
    SDL_BlitSurface(texte, NULL, jeu->ecran, &positionTexte);
    SDL_FreeSurface(texte);
    
    // Centrer les boutons horizontalement
    int xOffset = (WINDOW_WIDTH - 180) / 2; // Pour les boutons normaux (180x80)
    
    // Boutons Yes et No (Normal - 180x80)
    jeu->boutons[0] = creerBouton(xOffset - 100, 250, "assets/yes.png", "assets/highlighted/yes.png", "oui");
    jeu->boutons[1] = creerBouton(xOffset + 100, 250, "assets/no.png", "assets/highlighted/no.png", "non");
    
    // Bouton retour (Normal - 180x80)
    jeu->boutons[2] = creerBouton(xOffset, 400, "assets/return.png", "assets/highlighted/return.png", "retour");
    
    // Dessiner les boutons
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
    
    // Mettre à jour l'écran
    SDL_Flip(jeu->ecran);
}

void afficherMenuJoueur(Jeu* jeu) {
    // Clear screen first
    effacerEcran(jeu->ecran);
    
    // Arrêter toute musique en cours
    Mix_HaltMusic();
    
    // Jouer la musique du menu joueur
    Mix_PlayMusic(jeu->musiqueOptions, -1);
    
    // Afficher l'arrière-plan
    SDL_BlitSurface(jeu->arrierePlan2, NULL, jeu->ecran, NULL);
    
    // Libérer les boutons précédents
    for (int i = 0; i < jeu->nbBoutons; i++) {
        libererBouton(&jeu->boutons[i]);
    }
    free(jeu->boutons);
    
    // Créer les boutons du menu joueur
    jeu->nbBoutons = 3;
    jeu->boutons = malloc(jeu->nbBoutons * sizeof(Bouton));
    
    // Centrer les boutons horizontalement
    int xOffset = (WINDOW_WIDTH - 300) / 2; // Pour les boutons larges (300x80)
    
    // Boutons Singleplayer et Multiplayer (Wide - 300x80)
    jeu->boutons[0] = creerBouton(xOffset, 200, "assets/singleplayer.png", "assets/highlighted/singleplayer.png", "monojoueur");
    jeu->boutons[1] = creerBouton(xOffset, 300, "assets/multiplayer.png", "assets/highlighted/multiplayer.png", "multijoueurs");
    
    // Bouton retour (Normal - 180x80)
    jeu->boutons[2] = creerBouton((WINDOW_WIDTH - 180) / 2, 400, "assets/return.png", "assets/highlighted/return.png", "retour");
    
    // Dessiner les boutons
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
    
    // Mettre à jour l'écran
    SDL_Flip(jeu->ecran);
}

void afficherMenuMeilleursScores(Jeu* jeu) {
    effacerEcran(jeu->ecran);
    Mix_HaltMusic();
    SDL_BlitSurface(jeu->arrierePlan3, NULL, jeu->ecran, NULL);
    
    // Create larger white rectangle for text input
    if (!jeu->textInputBox) {
        jeu->textInputBox = SDL_CreateRGBSurface(SDL_HWSURFACE, 600, 80, 32, 0, 0, 0, 0);
    }
    SDL_FillRect(jeu->textInputBox, NULL, SDL_MapRGB(jeu->ecran->format, 255, 255, 255));
    
    // Center the text input box
    SDL_Rect inputRect = {(WINDOW_WIDTH - 600) / 2, 200, 600, 80};
    SDL_BlitSurface(jeu->textInputBox, NULL, jeu->ecran, &inputRect);
    
    // Draw prompt text or input text with larger font
    SDL_Color textColor = {0, 0, 0, 255}; // Black text
    SDL_Surface* textSurface;
    
    if (!jeu->isInputActive || strlen(jeu->inputText) == 0) {
        textSurface = TTF_RenderText_Blended(jeu->police, "Click here to type your name", textColor);
    } else {
        textSurface = TTF_RenderText_Blended(jeu->police, jeu->inputText, textColor);
    }
    
    if (textSurface) {
        SDL_Rect textPos = {inputRect.x + 20, inputRect.y + 20, 0, 0};
        SDL_BlitSurface(textSurface, NULL, jeu->ecran, &textPos);
        SDL_FreeSurface(textSurface);
    }
    
    // Create buttons with more space below input box
    jeu->nbBoutons = 2;
    jeu->boutons = malloc(jeu->nbBoutons * sizeof(Bouton));
    
    jeu->boutons[0] = creerBouton((WINDOW_WIDTH - 400) / 2, 350, "assets/confirm.png", "assets/highlighted/confirm.png", "confirmer");
    jeu->boutons[1] = creerBouton((WINDOW_WIDTH + 20) / 2, 350, "assets/return.png", "assets/highlighted/return.png", "retour");
    
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
    
    SDL_Flip(jeu->ecran);
}

void afficherScores(Jeu* jeu) {
    effacerEcran(jeu->ecran);
    
    // Jouer la musique de victoire
    Mix_PlayMusic(jeu->musiqueVictoire, -1);
    
    // Afficher l'arrière-plan
    SDL_BlitSurface(jeu->arrierePlan3, NULL, jeu->ecran, NULL);
    
    // Afficher les scores
    SDL_Color couleurTexte = {255, 255, 255, 0};
    char texteScore[100];
    
    // Titre
    SDL_Surface* titre = TTF_RenderText_Blended(jeu->police, "Meilleurs Scores", couleurTexte);
    SDL_Rect positionTitre = {300, 100, 0, 0};
    SDL_BlitSurface(titre, NULL, jeu->ecran, &positionTitre);
    SDL_FreeSurface(titre);
    
    // Liste des scores
    for (int i = 0; i < 3; i++) {
        if (jeu->meilleursScores[i].score > 0) {
            sprintf(texteScore, "%d. %s: %d", i+1, jeu->meilleursScores[i].nom, jeu->meilleursScores[i].score);
            SDL_Surface* texte = TTF_RenderText_Blended(jeu->police, texteScore, couleurTexte);
            SDL_Rect positionTexte = {300, 200 + i*40, 0, 0};
            SDL_BlitSurface(texte, NULL, jeu->ecran, &positionTexte);
            SDL_FreeSurface(texte);
        }
    }
    
    // Bouton retour uniquement
    jeu->nbBoutons = 1;
    jeu->boutons = malloc(sizeof(Bouton));
    jeu->boutons[0] = creerBouton(300, 400, "assets/return.png", "assets/highlighted/return.png", "retour");
    dessinerBouton(jeu->boutons[0], jeu->ecran);
    
    SDL_Flip(jeu->ecran);
}

void afficherMenuEnigme(Jeu* jeu) {
    // Clear screen first
    effacerEcran(jeu->ecran);
    
    // Arrêter toute musique en cours
    Mix_HaltMusic();
    
    // Afficher l'arrière-plan
    SDL_BlitSurface(jeu->arrierePlan4, NULL, jeu->ecran, NULL);
    
    // Libérer les boutons précédents
    for (int i = 0; i < jeu->nbBoutons; i++) {
        libererBouton(&jeu->boutons[i]);
    }
    free(jeu->boutons);
    
    // Créer les boutons du menu énigme
    jeu->nbBoutons = 3;
    jeu->boutons = malloc(jeu->nbBoutons * sizeof(Bouton));
    
    jeu->boutons[0] = creerBouton(200, 200, "assets/quiz.png", "assets/highlighted/quiz.png", "quiz");
    jeu->boutons[1] = creerBouton(400, 200, "assets/puzzle.png", "assets/highlighted/puzzle.png", "puzzle");
    jeu->boutons[2] = creerBouton(300, 400, "assets/return.png", "assets/highlighted/return.png", "retour");
    
    // Dessiner les boutons
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
    
    // Mettre à jour l'écran
    SDL_Flip(jeu->ecran);
}

void afficherMenuQuiz(Jeu* jeu) {
    // Clear screen first
    effacerEcran(jeu->ecran);
    
    // Arrêter toute musique en cours
    Mix_HaltMusic();
    
    // Jouer la musique de suspense
    Mix_PlayMusic(jeu->musiqueSuspense, -1);
    
    // Afficher l'arrière-plan
    SDL_BlitSurface(jeu->arrierePlan4, NULL, jeu->ecran, NULL);
    
    // Libérer les boutons précédents
    for (int i = 0; i < jeu->nbBoutons; i++) {
        libererBouton(&jeu->boutons[i]);
    }
    free(jeu->boutons);
    
    // Créer les boutons du menu quiz
    jeu->nbBoutons = 4;
    jeu->boutons = malloc(jeu->nbBoutons * sizeof(Bouton));
    
    // Afficher le message "Quiz"
    SDL_Color couleurTexte = {255, 255, 255, 0};
    SDL_Surface* texte = TTF_RenderText_Blended(jeu->police, "Quiz", couleurTexte);
    SDL_Rect positionTexte = {350, 100, 0, 0};
    SDL_BlitSurface(texte, NULL, jeu->ecran, &positionTexte);
    SDL_FreeSurface(texte);
    
    // Afficher la question
    texte = TTF_RenderText_Blended(jeu->police, "Quelle est la capitale de la France ?", couleurTexte);
    positionTexte.y = 150;
    positionTexte.x = 200;
    SDL_BlitSurface(texte, NULL, jeu->ecran, &positionTexte);
    SDL_FreeSurface(texte);
    
    // Centrer les boutons de réponse horizontalement
    int xOffset = (WINDOW_WIDTH - 160) / 2; // Pour les boutons petits (160x80)
    
    // Boutons de réponse (Small - 160x80)
    jeu->boutons[0] = creerBouton(xOffset, 250, "assets/A.png", "assets/highlighted/A.png", "reponseA");
    jeu->boutons[1] = creerBouton(xOffset, 350, "assets/B.png", "assets/highlighted/B.png", "reponseB");
    jeu->boutons[2] = creerBouton(xOffset, 450, "assets/C.png", "assets/highlighted/C.png", "reponseC");
    
    // Bouton retour (Normal - 180x80)
    jeu->boutons[3] = creerBouton((WINDOW_WIDTH - 180) / 2, 550, "assets/return.png", "assets/highlighted/return.png", "retour");
    
    // Dessiner les boutons
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
    
    // Mettre à jour l'écran
    SDL_Flip(jeu->ecran);
}

void afficherMenuPuzzle(Jeu* jeu) {
    // Clear screen first
    effacerEcran(jeu->ecran);
    
    // Arrêter toute musique en cours
    Mix_HaltMusic();
    
    // Afficher l'arrière-plan
    SDL_BlitSurface(jeu->arrierePlan4, NULL, jeu->ecran, NULL);
    
    // Libérer les boutons précédents
    for (int i = 0; i < jeu->nbBoutons; i++) {
        libererBouton(&jeu->boutons[i]);
    }
    free(jeu->boutons);
    
    // Créer les boutons du menu puzzle
    jeu->nbBoutons = 1;
    jeu->boutons = malloc(jeu->nbBoutons * sizeof(Bouton));
    
    jeu->boutons[0] = creerBouton(300, 450, "assets/return.png", "assets/highlighted/return.png", "retour");
    
    // Dessiner les boutons
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
    
    // Mettre à jour l'écran
    SDL_Flip(jeu->ecran);
}

void gererEvenements(Jeu* jeu) {
    SDL_Event evenement;
    bool quitter = false;
    Uint32 lastRedraw = SDL_GetTicks();
    const Uint32 REDRAW_INTERVAL = 16; // ~60 FPS
    bool needsRedraw = true;
    
    while (!quitter) {
        Uint32 currentTime = SDL_GetTicks();
        
        while (SDL_PollEvent(&evenement)) {
            switch (evenement.type) {
                case SDL_QUIT:
                    quitter = true;
                    jeu->etatActuel = QUITTER;
                    break;
                    
                case SDL_KEYDOWN:
                    if (jeu->etatActuel == MENU_MEILLEURS_SCORES && jeu->isInputActive) {
                        if (evenement.key.keysym.sym == SDLK_RETURN) {
                            // Keep the current text when pressing enter
                            jeu->isInputActive = false;
                            if (strlen(jeu->inputText) > 0) {
                                sauvegarderScore(jeu, jeu->inputText, 1000);
                                afficherScoresFinaux(jeu);
                            }
                            needsRedraw = true;
                        }
                        else if (evenement.key.keysym.sym == SDLK_BACKSPACE) {
                            int len = strlen(jeu->inputText);
                            if (len > 0) {
                                jeu->inputText[len-1] = '\0';
                                needsRedraw = true;
                            }
                        }
                        else if (evenement.key.keysym.unicode >= 32 && evenement.key.keysym.unicode <= 126) {
                            // Accept printable ASCII characters
                            int len = strlen(jeu->inputText);
                            if (len < 48) {
                                jeu->inputText[len] = (char)evenement.key.keysym.unicode;
                                jeu->inputText[len + 1] = '\0';
                                needsRedraw = true;
                            }
                        }
                    } else {
                        switch (evenement.key.keysym.sym) {
                            case SDLK_j:  // 'j' key for play
                                Mix_PlayChannel(-1, jeu->sonClick, 0);
                                jeu->menuPrecedent = jeu->etatActuel;
                                jeu->etatActuel = MENU_SAUVEGARDE;
                                afficherMenuSauvegarde(jeu);
                                needsRedraw = true;
                                break;
                                
                            case SDLK_o:  // 'o' key for options
                                Mix_PlayChannel(-1, jeu->sonClick, 0);
                                jeu->menuPrecedent = jeu->etatActuel;
                                jeu->etatActuel = MENU_OPTIONS;
                                afficherMenuOptions(jeu);
                                needsRedraw = true;
                                break;
                                
                            case SDLK_m:  // 'm' key for best scores
                                Mix_PlayChannel(-1, jeu->sonClick, 0);
                                jeu->menuPrecedent = jeu->etatActuel;
                                jeu->etatActuel = MENU_MEILLEURS_SCORES;
                                afficherMenuMeilleursScores(jeu);
                                needsRedraw = true;
                                break;
                                
                            case SDLK_f:  // 'f' key for fullscreen
                                if (jeu->etatActuel == MENU_OPTIONS && !jeu->pleinEcran) {
                                    Mix_PlayChannel(-1, jeu->sonClick, 0);
                                    basculerPleinEcran(jeu);
                                    needsRedraw = true;
                                }
                                break;
                                
                            case SDLK_n:  // 'n' key for normal screen
                                if (jeu->etatActuel == MENU_OPTIONS && jeu->pleinEcran) {
                                    Mix_PlayChannel(-1, jeu->sonClick, 0);
                                    basculerPleinEcran(jeu);
                                    needsRedraw = true;
                                }
                                break;
                                
                            case SDLK_ESCAPE:  // ESC key to quit or return
                                if (jeu->etatActuel != MENU_PRINCIPAL) {
                                    Mix_PlayChannel(-1, jeu->sonClick, 0);
                                    EtatMenu ancienEtat = jeu->etatActuel;
                                    jeu->etatActuel = jeu->menuPrecedent;
                                    jeu->menuPrecedent = ancienEtat;
                                    
                                    switch (jeu->etatActuel) {
                                        case MENU_PRINCIPAL:
                                            afficherMenuPrincipal(jeu);
                                            break;
                                        case MENU_OPTIONS:
                                            afficherMenuOptions(jeu);
                                            break;
                                        case MENU_SAUVEGARDE:
                                            afficherMenuSauvegarde(jeu);
                                            break;
                                        case MENU_JOUEUR:
                                            afficherMenuJoueur(jeu);
                                            break;
                                        case MENU_MEILLEURS_SCORES:
                                            afficherMenuMeilleursScores(jeu);
                                            break;
                                        case MENU_ENIGME:
                                            afficherMenuEnigme(jeu);
                                            break;
                                        default:
                                            break;
                                    }
                                    needsRedraw = true;
                                } else {
                                    quitter = true;
                                    jeu->etatActuel = QUITTER;
                                }
                                break;
                                
                            case SDLK_RETURN:
                                if (jeu->etatActuel == MENU_MEILLEURS_SCORES) {
                                    // Save score and show high scores
                                    sauvegarderScore(jeu, "PlayerName", 1000); // Replace with actual score
                                    afficherScores(jeu);
                                    needsRedraw = true;
                                }
                                break;
                                
                            case SDLK_e:
                                if (jeu->etatActuel == MENU_MEILLEURS_SCORES) {
                                    jeu->menuPrecedent = jeu->etatActuel;
                                    jeu->etatActuel = MENU_ENIGME;
                                    afficherMenuEnigme(jeu);
                                    needsRedraw = true;
                                }
                                break;
                                
                            default:
                                break;
                        }
                    }
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    if (evenement.button.button == SDL_BUTTON_LEFT) {
                        // Update click detection area to match the new input box size
                        SDL_Rect inputRect = {(WINDOW_WIDTH - 600) / 2, 200, 600, 80};
                        if (jeu->etatActuel == MENU_MEILLEURS_SCORES &&
                            evenement.button.x >= inputRect.x && 
                            evenement.button.x < inputRect.x + inputRect.w &&
                            evenement.button.y >= inputRect.y && 
                            evenement.button.y < inputRect.y + inputRect.h) {
                            jeu->isInputActive = true;
                            needsRedraw = true;
                        }
                        
                        for (int i = 0; i < jeu->nbBoutons; i++) {
                            if (estSurBouton(jeu->boutons[i], evenement.button.x, evenement.button.y)) {
                                Mix_PlayChannel(-1, jeu->sonClick, 0);
                                
                                // Handle all button clicks
                                if (strcmp(jeu->boutons[i].nom, "jouer") == 0) {
                                    jeu->menuPrecedent = jeu->etatActuel;
                                    jeu->etatActuel = MENU_SAUVEGARDE;
                                    afficherMenuSauvegarde(jeu);
                                }
                                else if (strcmp(jeu->boutons[i].nom, "options") == 0) {
                                    jeu->menuPrecedent = jeu->etatActuel;
                                    jeu->etatActuel = MENU_OPTIONS;
                                    afficherMenuOptions(jeu);
                                }
                                else if (strcmp(jeu->boutons[i].nom, "histoire") == 0) {
                                    jeu->menuPrecedent = jeu->etatActuel;
                                    jeu->etatActuel = MENU_ENIGME;
                                    afficherMenuEnigme(jeu);
                                }
                                else if (strcmp(jeu->boutons[i].nom, "meilleurs_scores") == 0) {
                                    jeu->menuPrecedent = jeu->etatActuel;
                                    jeu->etatActuel = MENU_MEILLEURS_SCORES;
                                    afficherMenuMeilleursScores(jeu);
                                }
                                else if (strcmp(jeu->boutons[i].nom, "quiz") == 0) {
                                    jeu->menuPrecedent = jeu->etatActuel;
                                    jeu->etatActuel = MENU_QUIZ;
                                    afficherMenuQuiz(jeu);
                                }
                                else if (strcmp(jeu->boutons[i].nom, "puzzle") == 0) {
                                    jeu->menuPrecedent = jeu->etatActuel;
                                    jeu->etatActuel = MENU_PUZZLE;
                                    afficherMenuPuzzle(jeu);
                                }
                                else if (strcmp(jeu->boutons[i].nom, "pleinecran") == 0) {
                                    basculerPleinEcran(jeu);
                                }
                                else if (strcmp(jeu->boutons[i].nom, "normal") == 0) {
                                    basculerPleinEcran(jeu);
                                }
                                else if (strcmp(jeu->boutons[i].nom, "augmenter") == 0) {
                                    jeu->volumeActuel = SDL_min(jeu->volumeActuel + 16, MIX_MAX_VOLUME);
                                    Mix_VolumeMusic(jeu->volumeActuel);
                                    Mix_Volume(-1, jeu->volumeActuel);
                                    mettreAJourBarreVolume(jeu);
                                }
                                else if (strcmp(jeu->boutons[i].nom, "diminuer") == 0) {
                                    jeu->volumeActuel = SDL_max(jeu->volumeActuel - 16, 0);
                                    Mix_VolumeMusic(jeu->volumeActuel);
                                    Mix_Volume(-1, jeu->volumeActuel);
                                    mettreAJourBarreVolume(jeu);
                                }
                                else if (strcmp(jeu->boutons[i].nom, "retour") == 0) {
                                    // Always go back to main menu on return
                                    jeu->menuPrecedent = jeu->etatActuel;
                                    jeu->etatActuel = MENU_PRINCIPAL;
                                    afficherMenuPrincipal(jeu);
                                    needsRedraw = true;
                                }
                                else if (strcmp(jeu->boutons[i].nom, "quitter") == 0) {
                                    quitter = true;
                                    jeu->etatActuel = QUITTER;
                                }
                                else if (strcmp(jeu->boutons[i].nom, "oui") == 0) {
                                    jeu->menuPrecedent = jeu->etatActuel;
                                    jeu->etatActuel = MENU_SELECTION_JEU;
                                    afficherMenuSelectionJeu(jeu);
                                }
                                else if (strcmp(jeu->boutons[i].nom, "nouveau_jeu") == 0) {
                                    jeu->menuPrecedent = jeu->etatActuel;
                                    jeu->etatActuel = MENU_JOUEUR;
                                    afficherMenuJoueur(jeu);
                                }
                                else if (strcmp(jeu->boutons[i].nom, "monojoueur") == 0) {
                                    jeu->menuPrecedent = jeu->etatActuel;
                                    jeu->etatActuel = MENU_SELECTION_PERSONNAGE;
                                    afficherMenuSelectionPersonnage(jeu);
                                }
                                else if (strcmp(jeu->boutons[i].nom, "multijoueurs") == 0) {
                                    // Just go back to main menu for now since multiplayer isn't implemented
                                    jeu->menuPrecedent = jeu->etatActuel;
                                    jeu->etatActuel = MENU_SELECTION_PERSONNAGE;
                                    afficherMenuSelectionPersonnage(jeu);
                                }
                                else if (strcmp(jeu->boutons[i].nom, "apparence1") == 0) {
                                    printf("Appearance 1 clicked\n");
                                    jeu->selectionPerso.apparence1Selected = !jeu->selectionPerso.apparence1Selected;
                                    jeu->selectionPerso.apparence2Selected = false;
                                    redessinerMenuSelectionPersonnage(jeu);
                                    needsRedraw = true;
                                }
                                else if (strcmp(jeu->boutons[i].nom, "apparence2") == 0) {
                                    printf("Appearance 2 clicked\n");
                                    jeu->selectionPerso.apparence2Selected = !jeu->selectionPerso.apparence2Selected;
                                    jeu->selectionPerso.apparence1Selected = false;
                                    redessinerMenuSelectionPersonnage(jeu);
                                    needsRedraw = true;
                                }
                                else if (strcmp(jeu->boutons[i].nom, "input1") == 0) {
                                    printf("Input 1 clicked\n");
                                    jeu->selectionPerso.input1Selected = !jeu->selectionPerso.input1Selected;
                                    jeu->selectionPerso.input2Selected = false;
                                    redessinerMenuSelectionPersonnage(jeu);
                                    needsRedraw = true;
                                }
                                else if (strcmp(jeu->boutons[i].nom, "input2") == 0) {
                                    printf("Input 2 clicked\n");
                                    jeu->selectionPerso.input2Selected = !jeu->selectionPerso.input2Selected;
                                    jeu->selectionPerso.input1Selected = false;
                                    redessinerMenuSelectionPersonnage(jeu);
                                    needsRedraw = true;
                                }
                                else if (strcmp(jeu->boutons[i].nom, "confirmer") == 0) {
                                    // Check if we're in character selection menu and confirm was clicked
                                    if (jeu->etatActuel == MENU_SELECTION_PERSONNAGE) {
                                        printf("Confirm clicked in character selection. Selections: app1=%d, app2=%d, in1=%d, in2=%d\n",
                                               jeu->selectionPerso.apparence1Selected,
                                               jeu->selectionPerso.apparence2Selected,
                                               jeu->selectionPerso.input1Selected,
                                               jeu->selectionPerso.input2Selected);
                                               
                                        if ((jeu->selectionPerso.apparence1Selected || jeu->selectionPerso.apparence2Selected) &&
                                            (jeu->selectionPerso.input1Selected || jeu->selectionPerso.input2Selected)) {
                                            jeu->menuPrecedent = jeu->etatActuel;
                                            jeu->etatActuel = MENU_MEILLEURS_SCORES;
                                            afficherMenuMeilleursScores(jeu);
                                            needsRedraw = true;
                                        }
                                    }
                                    // Handle confirm button in other menus
                                    else if (jeu->etatActuel == MENU_MEILLEURS_SCORES) {
                                        if (strlen(jeu->inputText) > 0) {
                                            sauvegarderScore(jeu, jeu->inputText, 1000);
                                            afficherScoresFinaux(jeu);
                                            needsRedraw = true;
                                        }
                                    }
                                }
                                else if (strcmp(jeu->boutons[i].nom, "confirmer") == 0 && 
                                         jeu->etatActuel == MENU_MEILLEURS_SCORES) {
                                    if (strlen(jeu->inputText) > 0) {
                                        sauvegarderScore(jeu, jeu->inputText, 1000); // Use actual score
                                        afficherScoresFinaux(jeu);
                                        needsRedraw = true;
                                    }
                                }
                                else if (strcmp(jeu->boutons[i].nom, "retour") == 0 && 
                                         jeu->etatActuel == MENU_MEILLEURS_SCORES) {
                                    jeu->etatActuel = MENU_PRINCIPAL;
                                    afficherMenuPrincipal(jeu);
                                    needsRedraw = true;
                                }
                                
                                needsRedraw = true;
                                break;
                            }
                        }
                    }
                    break;

                case SDL_KEYUP:
                    if (jeu->etatActuel == MENU_MEILLEURS_SCORES && jeu->isInputActive) {
                        if (evenement.key.keysym.sym != SDLK_RETURN && 
                            evenement.key.keysym.sym != SDLK_BACKSPACE) {
                            char newChar = evenement.key.keysym.unicode;
                            if (isalnum(newChar) && strlen(jeu->inputText) < 48) {
                                int len = strlen(jeu->inputText);
                                jeu->inputText[len] = newChar;
                                jeu->inputText[len+1] = '\0';
                                needsRedraw = true;
                            }
                        }
                    }
                    break;

                // ...rest of event handling...
            }
        }
        
        if (needsRedraw && (currentTime - lastRedraw >= REDRAW_INTERVAL)) {
            switch (jeu->etatActuel) {
                case MENU_PRINCIPAL:
                    redessinerMenuPrincipal(jeu);
                    break;
                case MENU_OPTIONS:
                    redessinerMenuOptions(jeu);
                    break;
                case MENU_SAUVEGARDE:
                    redessinerMenuSauvegarde(jeu);
                    break;
                case MENU_JOUEUR:
                    redessinerMenuJoueur(jeu);
                    break;
                case MENU_MEILLEURS_SCORES:
                    redessinerMenuMeilleursScores(jeu);
                    break;
                case MENU_ENIGME:
                    redessinerMenuEnigme(jeu);
                    break;
                case MENU_QUIZ:
                    redessinerMenuQuiz(jeu);
                    break;
                case MENU_PUZZLE:
                    redessinerMenuPuzzle(jeu);
                    break;
                case MENU_SELECTION_JEU:
                    redessinerMenuSelectionJeu(jeu);
                    break;
                case MENU_SELECTION_PERSONNAGE:
                    redessinerMenuSelectionPersonnage(jeu);
                    break;
                case JEU_EN_COURS:
                    // Will be implemented later
                    break;
                case QUITTER:
                    // Nothing to redraw
                    break;
            }
            
            SDL_Flip(jeu->ecran);
            lastRedraw = currentTime;
            needsRedraw = false;
        }
        
        SDL_Delay(1);
    }
}

// Add new helper function for menu redrawing
void redessinerMenuPrincipal(Jeu* jeu) {
    effacerEcran(jeu->ecran);
    SDL_BlitSurface(jeu->arrierePlan1, NULL, jeu->ecran, NULL);
    
    // Draw logo
    if (jeu->logo) {
        SDL_Rect positionLogo = {50, 100, 0, 0};
        SDL_BlitSurface(jeu->logo, NULL, jeu->ecran, &positionLogo);
    }
    
    // Draw all buttons
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
}

void redessinerMenuOptions(Jeu* jeu) {
    effacerEcran(jeu->ecran);
    SDL_BlitSurface(jeu->arrierePlan2, NULL, jeu->ecran, NULL);
    
    // Redraw volume label
    SDL_Surface* volumeLabel = IMG_Load("assets/volume.png");
    if (volumeLabel) {
        SDL_Rect positionVolume = {90, 200, 0, 0};
        SDL_BlitSurface(volumeLabel, NULL, jeu->ecran, &positionVolume);
        SDL_FreeSurface(volumeLabel);
    }
    
    // Redraw display mode label
    SDL_Surface* displayModeLabel = IMG_Load("assets/display mode.png");
    if (displayModeLabel) {
        SDL_Rect positionDisplayMode = {90, 400, 0, 0};
        SDL_BlitSurface(displayModeLabel, NULL, jeu->ecran, &positionDisplayMode);
        SDL_FreeSurface(displayModeLabel);
    }
    
    // Draw volume bar
    mettreAJourBarreVolume(jeu);
    
    // Draw all buttons
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
}

// Update the volume bar function
void mettreAJourBarreVolume(Jeu* jeu) {
    // Déterminer quelle image de barre de volume utiliser
    char* imageVolume = "assets/volumeslider0.png";
    if (jeu->volumeActuel > 80) {
        imageVolume = "assets/volumeslider100.png";
    } else if (jeu->volumeActuel > 60) {
        imageVolume = "assets/volumeslider80.png";
    } else if (jeu->volumeActuel > 40) {
        imageVolume = "assets/volumeslider60.png";
    } else if (jeu->volumeActuel > 20) {
        imageVolume = "assets/volumeslider40.png";
    } else if (jeu->volumeActuel > 0) {
        imageVolume = "assets/volumeslider20.png";
    }
    
    // Charger l'image de la barre de volume
    SDL_Surface* sliderVolume = IMG_Load(imageVolume);
    if (sliderVolume) {
        // Position et dimensions de la barre de volume
        SDL_Rect positionSlider = {700, 200, 300, 80};
        
        // Effacer uniquement la zone de la barre de volume
        SDL_Rect zoneAEffacer = {700, 200, 300, 80};
        SDL_FillRect(jeu->ecran, &zoneAEffacer, SDL_MapRGB(jeu->ecran->format, 0, 0, 0));
        
        // Récupérer la partie de l'arrière-plan correspondant à la zone de la barre de volume
        SDL_Rect sourceRect = {700, 200, 300, 80};
        SDL_BlitSurface(jeu->arrierePlan2, &sourceRect, jeu->ecran, &zoneAEffacer);
        
        // Afficher la barre de volume
        SDL_BlitSurface(sliderVolume, NULL, jeu->ecran, &positionSlider);
        SDL_FreeSurface(sliderVolume);
    }
}

void redessinerMenuSauvegarde(Jeu* jeu) {
    effacerEcran(jeu->ecran);
    SDL_BlitSurface(jeu->arrierePlan2, NULL, jeu->ecran, NULL);
    
    SDL_Color couleurTexte = {255, 255, 255, 0};
    SDL_Surface* texte = TTF_RenderText_Blended(jeu->police, "Voulez-vous sauvegarder votre jeu ?", couleurTexte);
    SDL_Rect positionTexte = {200, 150, 0, 0};
    SDL_BlitSurface(texte, NULL, jeu->ecran, &positionTexte);
    SDL_FreeSurface(texte);
    
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
}

void redessinerMenuJoueur(Jeu* jeu) {
    effacerEcran(jeu->ecran);
    SDL_BlitSurface(jeu->arrierePlan2, NULL, jeu->ecran, NULL);
    
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
}

void redessinerMenuMeilleursScores(Jeu* jeu) {
    effacerEcran(jeu->ecran);
    SDL_BlitSurface(jeu->arrierePlan3, NULL, jeu->ecran, NULL);
    
    // Always redraw the input box
    if (jeu->textInputBox) {
        SDL_Rect inputRect = {(WINDOW_WIDTH - 600) / 2, 200, 600, 80};
        SDL_FillRect(jeu->textInputBox, NULL, SDL_MapRGB(jeu->ecran->format, 255, 255, 255));
        SDL_BlitSurface(jeu->textInputBox, NULL, jeu->ecran, &inputRect);
        
        // Draw current input text
        SDL_Color textColor = {0, 0, 0, 255};
        SDL_Surface* textSurface;
        
        if (!jeu->isInputActive || strlen(jeu->inputText) == 0) {
            textSurface = TTF_RenderText_Blended(jeu->police, "Click here to type your name", textColor);
        } else {
            textSurface = TTF_RenderText_Blended(jeu->police, jeu->inputText, textColor);
        }
        
        if (textSurface) {
            SDL_Rect textPos = {inputRect.x + 20, inputRect.y + 20, 0, 0};
            SDL_BlitSurface(textSurface, NULL, jeu->ecran, &textPos);
            SDL_FreeSurface(textSurface);
        }
    }
    
    // Draw buttons
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
    
    SDL_Flip(jeu->ecran);
}

void redessinerMenuEnigme(Jeu* jeu) {
    effacerEcran(jeu->ecran);
    SDL_BlitSurface(jeu->arrierePlan4, NULL, jeu->ecran, NULL);
    
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
}

void redessinerMenuQuiz(Jeu* jeu) {
    effacerEcran(jeu->ecran);
    SDL_BlitSurface(jeu->arrierePlan4, NULL, jeu->ecran, NULL);
    
    SDL_Color couleurTexte = {255, 255, 255, 0};
    SDL_Surface* texte = TTF_RenderText_Blended(jeu->police, "Quiz", couleurTexte);
    SDL_Rect positionTexte = {350, 100, 0, 0};
    SDL_BlitSurface(texte, NULL, jeu->ecran, &positionTexte);
    SDL_FreeSurface(texte);
    
    texte = TTF_RenderText_Blended(jeu->police, "Quelle est la capitale de la France ?", couleurTexte);
    positionTexte.y = 150;
    positionTexte.x = 200;
    SDL_BlitSurface(texte, NULL, jeu->ecran, &positionTexte);
    SDL_FreeSurface(texte);
    
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
}

void redessinerMenuPuzzle(Jeu* jeu) {
    effacerEcran(jeu->ecran);
    SDL_BlitSurface(jeu->arrierePlan4, NULL, jeu->ecran, NULL);
    
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
}

void basculerPleinEcran(Jeu* jeu) {
    jeu->pleinEcran = !jeu->pleinEcran;
    
    // Recréer la fenêtre avec le nouveau mode
    Uint32 flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
    if (jeu->pleinEcran) {
        flags |= SDL_FULLSCREEN;
    }
    
    jeu->ecran = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, flags);
    if (!jeu->ecran) {
        fprintf(stderr, "Erreur lors du changement de mode d'affichage: %s\n", SDL_GetError());
    }
    
    // Redessiner le menu options avec le nouveau mode
    afficherMenuOptions(jeu);
}

void sauvegarderScore(Jeu* jeu, const char* nom, int score) {
    // Vérifier si le score est meilleur que les scores existants
    int position = -1;
    for (int i = 0; i < 3; i++) {
        if (score > jeu->meilleursScores[i].score) {
            position = i;
            break;
        }
    }
    
    // Si le score est meilleur, l'insérer et décaler les autres
    if (position != -1) {
        for (int i = 2; i > position; i++) {
            jeu->meilleursScores[i] = jeu->meilleursScores[i-1];
        }
        
        strncpy(jeu->meilleursScores[position].nom, nom, 49);
        jeu->meilleursScores[position].nom[49] = '\0'; // Assurer la terminaison
        jeu->meilleursScores[position].score = score;
        
        // Sauvegarder les scores dans un fichier
        FILE* fichier = fopen("scores.dat", "wb");
        if (fichier) {
            fwrite(jeu->meilleursScores, sizeof(Score), 3, fichier);
            fclose(fichier);
        }
    }
}

void chargerMeilleursScores(Jeu* jeu) {
    // Initialiser les scores à zéro
    for (int i = 0; i < 3; i++) {
        strcpy(jeu->meilleursScores[i].nom, "");
        jeu->meilleursScores[i].score = 0;
    }
    
    // Charger les scores depuis un fichier
    FILE* fichier = fopen("scores.dat", "rb");
    if (fichier) {
        fread(jeu->meilleursScores, sizeof(Score), 3, fichier);
        fclose(fichier);
    }
}

void afficherMenuSelectionJeu(Jeu* jeu) {
    effacerEcran(jeu->ecran);
    Mix_HaltMusic();
    SDL_BlitSurface(jeu->arrierePlan2, NULL, jeu->ecran, NULL);
    
    // Free previous buttons
    for (int i = 0; i < jeu->nbBoutons; i++) {
        libererBouton(&jeu->boutons[i]);
    }
    free(jeu->boutons);
    
    jeu->nbBoutons = 3;
    jeu->boutons = malloc(jeu->nbBoutons * sizeof(Bouton));
    
    int yStart = 200;
    int xCenter = (WINDOW_WIDTH - 300) / 2;
    
    // Fix button filenames - remove spaces and ensure correct path
    jeu->boutons[0] = creerBouton(xCenter, yStart, 
                                 "assets/new_game.png", 
                                 "assets/highlighted/new_game.png", 
                                 "nouveau_jeu");
    
    if (!jeu->boutons[0].surface) {
        fprintf(stderr, "Failed to load new_game.png: %s\n", IMG_GetError());
    }
    
    jeu->boutons[1] = creerBouton(xCenter, yStart + 120,
                                 "assets/load_game.png",
                                 "assets/highlighted/load_game.png",
                                 "charger_jeu");
    
    if (!jeu->boutons[1].surface) {
        fprintf(stderr, "Failed to load load_game.png: %s\n", IMG_GetError());
    }
    
    jeu->boutons[2] = creerBouton((WINDOW_WIDTH - 180) / 2, yStart + 240,
                                 "assets/return.png", 
                                 "assets/highlighted/return.png", 
                                 "retour");
    
    if (!jeu->boutons[2].surface) {
        fprintf(stderr, "Failed to load return.png: %s\n", IMG_GetError());
    }
    
    // Debug prints for button loading
    for (int i = 0; i < jeu->nbBoutons; i++) {
        if (jeu->boutons[i].surface) {
            printf("Button %d: %s loaded successfully. Size: %dx%d\n",
                   i, jeu->boutons[i].nom,
                   jeu->boutons[i].surface->w,
                   jeu->boutons[i].surface->h);
        } else {
            printf("Button %d: %s failed to load\n", i, jeu->boutons[i].nom);
        }
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
    
    SDL_Flip(jeu->ecran);
}

void redessinerMenuSelectionJeu(Jeu* jeu) {
    effacerEcran(jeu->ecran);
    SDL_BlitSurface(jeu->arrierePlan2, NULL, jeu->ecran, NULL);
    
    for (int i = 0; i < jeu->nbBoutons; i++) {
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
}

void afficherMenuSelectionPersonnage(Jeu* jeu) {
    effacerEcran(jeu->ecran);
    SDL_BlitSurface(jeu->arrierePlan2, NULL, jeu->ecran, NULL);
    
    // Reset selections
    jeu->selectionPerso.apparence1Selected = false;
    jeu->selectionPerso.apparence2Selected = false;
    jeu->selectionPerso.input1Selected = false;
    jeu->selectionPerso.input2Selected = false;
    
    // Create buttons
    jeu->nbBoutons = 5;
    jeu->boutons = malloc(jeu->nbBoutons * sizeof(Bouton));
    
    int xCenter = (WINDOW_WIDTH - 300) / 2;
    int yStart = 150;
    
    // Appearance buttons with different images for normal and selected states
    jeu->boutons[0] = creerBouton(xCenter, yStart, 
                                 "assets/appearance1.png",
                                 "assets/highlighted/appearance1.png", 
                                 "apparence1");
    
    if (!jeu->boutons[0].surface) {
        fprintf(stderr, "Failed to load appearance1.png: %s\n", IMG_GetError());
    }
                                 
    jeu->boutons[1] = creerBouton(xCenter + 350, yStart,
                                 "assets/appearance2.png",
                                 "assets/highlighted/appearance2.png",
                                 "apparence2");
    
    if (!jeu->boutons[1].surface) {
        fprintf(stderr, "Failed to load appearance2.png: %s\n", IMG_GetError());
    }
                                 
    // Input buttons
    jeu->boutons[2] = creerBouton(xCenter, yStart + 200,
                                 "assets/input1.png",
                                 "assets/highlighted/input1.png",
                                 "input1");
    
    if (!jeu->boutons[2].surface) {
        fprintf(stderr, "Failed to load input1.png: %s\n", IMG_GetError());
    }
                                 
    jeu->boutons[3] = creerBouton(xCenter + 350, yStart + 200,
                                 "assets/input2.png",
                                 "assets/highlighted/input2.png",
                                 "input2");
    
    if (!jeu->boutons[3].surface) {
        fprintf(stderr, "Failed to load input2.png: %s\n", IMG_GetError());
    }
    
    // Confirm button
    jeu->boutons[4] = creerBouton((WINDOW_WIDTH - 180) / 2, yStart + 400,
                                 "assets/confirm.png",
                                 "assets/highlighted/confirm.png",
                                 "confirmer");
    
    // Debug prints for button loading status
    for (int i = 0; i < jeu->nbBoutons; i++) {
        if (jeu->boutons[i].surface) {
            printf("Button %d: %s loaded successfully. Size: %dx%d\n",
                   i, jeu->boutons[i].nom,
                   jeu->boutons[i].surface->w,
                   jeu->boutons[i].surface->h);
        } else {
            printf("Button %d: %s failed to load\n", i, jeu->boutons[i].nom);
        }
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
    
    SDL_Flip(jeu->ecran);
}

void redessinerMenuSelectionPersonnage(Jeu* jeu) {
    effacerEcran(jeu->ecran);
    SDL_BlitSurface(jeu->arrierePlan2, NULL, jeu->ecran, NULL);
    
    // Debug print for button states
    printf("Redrawing character selection menu\n");
    printf("States: app1=%d, app2=%d, in1=%d, in2=%d\n",
           jeu->selectionPerso.apparence1Selected,
           jeu->selectionPerso.apparence2Selected,
           jeu->selectionPerso.input1Selected,
           jeu->selectionPerso.input2Selected);
    
    for (int i = 0; i < jeu->nbBoutons; i++) {
        // Update hover state based on selections
        switch(i) {
            case 0:
                jeu->boutons[i].hover = jeu->selectionPerso.apparence1Selected;
                break;
            case 1:
                jeu->boutons[i].hover = jeu->selectionPerso.apparence2Selected;
                break;
            case 2:
                jeu->boutons[i].hover = jeu->selectionPerso.input1Selected;
                break;
            case 3:
                jeu->boutons[i].hover = jeu->selectionPerso.input2Selected;
                break;
        }
        dessinerBouton(jeu->boutons[i], jeu->ecran);
    }
    
    SDL_Flip(jeu->ecran);
}

void afficherScoresFinaux(Jeu* jeu) {
    effacerEcran(jeu->ecran);
    
    // Play victory music
    Mix_PlayMusic(jeu->musiqueVictoire, -1);
    
    // Show background4
    SDL_BlitSurface(jeu->arrierePlan4, NULL, jeu->ecran, NULL);
    
    // Display scores
    SDL_Color textColor = {255, 255, 255, 255};
    char scoreText[100];
    
    SDL_Surface* titleText = TTF_RenderText_Blended(jeu->police, "High Scores", textColor);
    SDL_Rect titlePos = {(WINDOW_WIDTH - titleText->w) / 2, 100, 0, 0};
    SDL_BlitSurface(titleText, NULL, jeu->ecran, &titlePos);
    SDL_FreeSurface(titleText);
    
    for (int i = 0; i < 3; i++) {
        if (jeu->meilleursScores[i].score > 0) {
            sprintf(scoreText, "%d. %s: %d", i+1, jeu->meilleursScores[i].nom, jeu->meilleursScores[i].score);
            SDL_Surface* scoreSurface = TTF_RenderText_Blended(jeu->police, scoreText, textColor);
            SDL_Rect scorePos = {300, 200 + i*50, 0, 0};
            SDL_BlitSurface(scoreSurface, NULL, jeu->ecran, &scorePos);
            SDL_FreeSurface(scoreSurface);
        }
    }
    
    // Single return button
    jeu->nbBoutons = 1;
    jeu->boutons = malloc(sizeof(Bouton));
    jeu->boutons[0] = creerBouton(300, 400, "assets/return.png", "assets/highlighted/return.png", "retour");
    dessinerBouton(jeu->boutons[0], jeu->ecran);
    
    SDL_Flip(jeu->ecran);
}


