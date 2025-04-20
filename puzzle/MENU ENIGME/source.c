#include "header.h"
#include "header1.h"
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Mix_Chunk* hoverSound = NULL;
Mix_Chunk* clickSound = NULL;
Mix_Music* bgMusic = NULL;
GameState currentState = MAIN_MENU;
int currentQuizIndex = 0;
int score = 0;

// Quiz game variables
Quiz quizzes[MAX_QUESTIONS];
int quizCount = 0;

// Puzzle game variables (from main1.c)
Mix_Chunk *successSound = NULL;
Mix_Chunk *failureSound = NULL;
Mix_Chunk *countdownSounds[11];
Puzzle puzzles[NUM_PUZZLES] = {
    {
        "assets/puzzle2_full.png",
        {
            "assets/puzzle2_piece0.png", "assets/puzzle2_piece1.png", "assets/puzzle2_piece2.png",
            "assets/puzzle2_piece3.png", "assets/puzzle2_piece4.png", "assets/puzzle2_piece5.png",
            "assets/puzzle2_piece6.png", "assets/puzzle2_piece7.png", "assets/puzzle2_piece8.png"
        }
    }
};

void load_quizzes(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        exit(1);
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        if (quizCount >= MAX_QUESTIONS) {
            fprintf(stderr, "Maximum number of questions exceeded.\n");
            break;
        }

        line[strcspn(line, "\n")] = 0;

        char* question = strtok(line, "|");
        char* answerA = strtok(NULL, "|");
        char* answerB = strtok(NULL, "|");
        char* answerC = strtok(NULL, "|");
        char* correctAnswerStr = strtok(NULL, "|");

        if (!question || !answerA || !answerB || !answerC || !correctAnswerStr) {
            fprintf(stderr, "Invalid line format: %s\n", line);
            continue;
        }

        int correctAnswer = atoi(correctAnswerStr);

        quizzes[quizCount].question = strdup(question);
        quizzes[quizCount].answerA = strdup(answerA);
        quizzes[quizCount].answerB = strdup(answerB);
        quizzes[quizCount].answerC = strdup(answerC);
        quizzes[quizCount].correctAnswer = correctAnswer;

        quizCount++;
    }

    fclose(file);
}

SDL_Surface* load_image(const char* path) {
    SDL_Surface* loadedImage = IMG_Load(path);
    if (!loadedImage) {
        fprintf(stderr, "IMG_Load Error: %s\n", IMG_GetError());
        exit(1);
    }
    return loadedImage;
}

void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* dest) {
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;
    SDL_BlitSurface(source, NULL, dest, &offset);
}

void init_SDL() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        exit(1);
    }
    if (TTF_Init() == -1) {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        exit(1);
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
        fprintf(stderr, "Mix_OpenAudio Error: %s\n", Mix_GetError());
        exit(1);
    }
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        fprintf(stderr, "IMG_Init Error: %s\n", IMG_GetError());
        exit(1);
    }
}

void load_assets(SDL_Surface** background, SDL_Surface** quizBtnImg, SDL_Surface** puzzleBtnImg, SDL_Surface** optionA, SDL_Surface** optionB, SDL_Surface** optionC, TTF_Font** font) {
    *background = load_image("assets/background.png");
    if (!*background) {
        fprintf(stderr, "Failed to load background image: %s\n", IMG_GetError());
        exit(1);
    }

    *quizBtnImg = load_image("assets/quiz.png");
    if (!*quizBtnImg) {
        fprintf(stderr, "Failed to load quiz button image: %s\n", IMG_GetError());
        exit(1);
    }

    *puzzleBtnImg = load_image("assets/puzzle.png");
    if (!*puzzleBtnImg) {
        fprintf(stderr, "Failed to load puzzle button image: %s\n", IMG_GetError());
        exit(1);
    }

    *optionA = load_image("assets/A.png");
    if (!*optionA) {
        fprintf(stderr, "Failed to load option A image: %s\n", IMG_GetError());
        exit(1);
    }

    *optionB = load_image("assets/B.png");
    if (!*optionB) {
        fprintf(stderr, "Failed to load option B image: %s\n", IMG_GetError());
        exit(1);
    }

    *optionC = load_image("assets/C.png");
    if (!*optionC) {
        fprintf(stderr, "Failed to load option C image: %s\n", IMG_GetError());
        exit(1);
    }

    *font = TTF_OpenFont("assets/font.ttf", 35);
    if (!*font) {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
        exit(1);
    }

    hoverSound = Mix_LoadWAV("assets/hover.wav");
    if (!hoverSound) {
        printf("Failed to load hover sound: %s\n", Mix_GetError());
    }

    clickSound = Mix_LoadWAV("assets/click.wav");
    if (!clickSound) {
        printf("Failed to load click sound: %s\n", Mix_GetError());
    }

    bgMusic = Mix_LoadMUS("assets/background.wav");
    if (!bgMusic) {
        printf("Failed to load background music: %s\n", Mix_GetError());
    }

    // Load puzzle-specific assets (from main1.c)
    successSound = Mix_LoadWAV("assets/explo.wav");
    if (!successSound) {
        fprintf(stderr, "Failed to load success sound: %s\n", Mix_GetError());
    }
    failureSound = Mix_LoadWAV("assets/fail.wav");
    if (!failureSound) {
        fprintf(stderr, "Failed to load failure sound: %s\n", Mix_GetError());
    }

    const char* countdownFiles[11] = {
        "assets/ten.wav", "assets/nine.wav", "assets/eight.wav", "assets/seven.wav",
        "assets/six.wav", "assets/five.wav", "assets/four.wav", "assets/three.wav",
        "assets/two.wav", "assets/one.wav", "assets/zero.wav"
    };
    for (int i = 0; i < 11; i++) {
        countdownSounds[i] = Mix_LoadWAV(countdownFiles[i]);
        if (!countdownSounds[i]) {
            fprintf(stderr, "Failed to load countdown sound %s: %s\n", countdownFiles[i], Mix_GetError());
        }
    }
}

void handle_events(int* running, Button buttons[], int buttonCount) {
    SDL_Event event;
    static int lastHover[5] = {0};

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
            *running = 0;
        }

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        for (int i = 0; i < buttonCount; i++) {
            int hover = SDL_PointInRect(&(SDL_Point){mouseX, mouseY}, &buttons[i].rect);
            if (hover && !lastHover[i]) {
                Mix_PlayChannel(-1, hoverSound, 0);
            }
            buttons[i].active = hover;
            lastHover[i] = hover;
        }

        if (currentState == MAIN_MENU) {
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (buttons[0].active) { // Quiz button
                    currentState = QUIZ;
                    Mix_PlayMusic(bgMusic, -1);
                }
                if (buttons[1].active) { // Puzzle button
                    currentState = PUZZLE;
                    Mix_PlayMusic(bgMusic, -1);
                }
            }
        } else if (currentState == QUIZ) {
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (buttons[2].active || buttons[3].active || buttons[4].active) {
                    Mix_PlayChannel(-1, clickSound, 0);
                }

                if (buttons[2].active && quizzes[currentQuizIndex].correctAnswer == 1) {
                    printf("Correct answer!\n");
                    score++;
                } else if (buttons[3].active && quizzes[currentQuizIndex].correctAnswer == 2) {
                    printf("Correct answer!\n");
                    score++;
                } else if (buttons[4].active && quizzes[currentQuizIndex].correctAnswer == 3) {
                    printf("Correct answer!\n");
                    score++;
                } else {
                    printf("Wrong answer!\n");
                }

                currentQuizIndex++;
                if (currentQuizIndex >= quizCount) {
                    printf("Quiz finished! Your score: %d\n", score);
                    show_score_window(score);
                    currentState = MAIN_MENU;
                    currentQuizIndex = 0;
                    score = 0;
                }
            }
        }
    }
}

void render(SDL_Surface* screen, SDL_Surface* background, Button buttons[], int buttonCount, TTF_Font* font) {
    SDL_FillRect(screen, NULL, 0);
    apply_surface(0, 0, background, screen);

    if (currentState == MAIN_MENU) {
        apply_surface(buttons[0].rect.x, buttons[0].rect.y, buttons[0].surface, screen);
        apply_surface(buttons[1].rect.x, buttons[1].rect.y, buttons[1].surface, screen);
    } else if (currentState == QUIZ) {
        SDL_Color textColor = {255, 255, 255};
        SDL_Surface* question = TTF_RenderText_Solid(font, quizzes[currentQuizIndex].question, textColor);
        apply_surface(100, 100, question, screen);

        int centerX = (SCREEN_W - buttons[2].surface->w) / 2;
        int centerY = (SCREEN_H - buttons[2].surface->w) / 2;

        SDL_Surface* answerA = TTF_RenderText_Solid(font, quizzes[currentQuizIndex].answerA, textColor);
        SDL_Surface* answerB = TTF_RenderText_Solid(font, quizzes[currentQuizIndex].answerB, textColor);
        SDL_Surface* answerC = TTF_RenderText_Solid(font, quizzes[currentQuizIndex].answerC, textColor);

        apply_surface(centerX - 300, centerY - 50, answerA, screen);
        apply_surface(centerX, centerY - 50, answerB, screen);
        apply_surface(centerX + 300, centerY - 50, answerC, screen);

        buttons[2].rect.x = centerX - 300;
        buttons[2].rect.y = centerY;
        buttons[3].rect.x = centerX;
        buttons[3].rect.y = centerY;
        buttons[4].rect.x = centerX + 300;
        buttons[4].rect.y = centerY;

        apply_surface(buttons[2].rect.x, buttons[2].rect.y, buttons[2].surface, screen);
        apply_surface(buttons[3].rect.x, buttons[3].rect.y, buttons[3].surface, screen);
        apply_surface(buttons[4].rect.x, buttons[4].rect.y, buttons[4].surface, screen);

        SDL_FreeSurface(question);
        SDL_FreeSurface(answerA);
        SDL_FreeSurface(answerB);
        SDL_FreeSurface(answerC);

        char scoreText[50];
        sprintf(scoreText, "Score: %d", score);
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText, textColor);
        apply_surface(10, 10, scoreSurface, screen);
        SDL_FreeSurface(scoreSurface);
    } else if (currentState == PUZZLE) {
        // Call the puzzle game logic from fonction1.c
        run_puzzle_game(screen, font, clickSound, successSound, failureSound, countdownSounds, bgMusic);
        currentState = MAIN_MENU; // Return to main menu after puzzle game ends
    }

    SDL_Flip(screen);
}

void cleanup(SDL_Surface* background, SDL_Surface* quizBtnImg, SDL_Surface* puzzleBtnImg, SDL_Surface* optionA, SDL_Surface* optionB, SDL_Surface* optionC, TTF_Font* font) {
    Mix_FreeChunk(hoverSound);
    Mix_FreeChunk(clickSound);
    Mix_FreeMusic(bgMusic);
    Mix_FreeChunk(successSound);
    Mix_FreeChunk(failureSound);
    for (int i = 0; i < 11; i++) {
        if (countdownSounds[i]) Mix_FreeChunk(countdownSounds[i]);
    }
    TTF_CloseFont(font);
    SDL_FreeSurface(background);
    SDL_FreeSurface(quizBtnImg);
    SDL_FreeSurface(puzzleBtnImg);
    SDL_FreeSurface(optionA);
    SDL_FreeSurface(optionB);
    SDL_FreeSurface(optionC);
    TTF_Quit();
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();
}

void show_score_window(int score) {
    SDL_Surface* screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32, SDL_SWSURFACE);
    if (!screen) {
        fprintf(stderr, "SDL_SetVideoMode Error: %s\n", SDL_GetError());
        return;
    }
    SDL_WM_SetCaption("Quiz Finished", NULL);

    SDL_Color textColor = {255, 255, 255};
    TTF_Font* font = TTF_OpenFont("assets/font.ttf", 48);
    if (!font) {
        fprintf(stderr, "TTF_OpenFont Error: %s\n", TTF_GetError());
        return;
    }

    char message[100];
    sprintf(message, "Quiz finished! Your score: %d", score);

    SDL_Surface* messageSurface = TTF_RenderText_Solid(font, message, textColor);
    if (!messageSurface) {
        fprintf(stderr, "TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        TTF_CloseFont(font);
        return;
    }

    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    apply_surface((SCREEN_W - messageSurface->w) / 2, (SCREEN_H - messageSurface->h) / 2, messageSurface, screen);
    SDL_Flip(screen);

    SDL_FreeSurface(messageSurface);
    TTF_CloseFont(font);

    SDL_Event event;
    int waiting = 1;
    while (waiting) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN)) {
                waiting = 0;
            }
        }
    }
}
