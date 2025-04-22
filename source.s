#include "header.h"
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h> // Include SDL_image
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> // Include time.h for timer functionality

#define MAX_QUESTIONS 100
#define MAX_LINE_LENGTH 512

Mix_Chunk* hoverSound = NULL;
Mix_Chunk* clickSound = NULL; // Add click sound
Mix_Music* bgMusic = NULL;
GameState currentState = MAIN_MENU;
int currentQuizIndex = 0;
int score = 0; // Add a score counter

// Global variable to track the start time of the quiz
time_t quiz_start_time;

Quiz quizzes[MAX_QUESTIONS];
int quizCount = 0;

// Function to start the timer when the quiz button is clicked
void start_quiz_timer() {
    quiz_start_time = time(NULL);
}

// Function to calculate and display the elapsed time when the quiz ends
void end_quiz_timer() {
    time_t quiz_end_time = time(NULL);
    double elapsed_time = difftime(quiz_end_time, quiz_start_time);
    printf("Quiz completed in %.0f seconds.\n", elapsed_time);
}

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

        // Remove newline character
        line[strcspn(line, "\n")] = 0;

        // Split the line into tokens
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

        // Populate the quiz structure
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

    clickSound = Mix_LoadWAV("assets/click.wav"); // Load click sound
    if (!clickSound) {
        printf("Failed to load click sound: %s\n", Mix_GetError());
    }

    bgMusic = Mix_LoadMUS("assets/song.mp3");
    if (!bgMusic) {
        printf("Failed to load background music: %s\n", Mix_GetError());
    }
}

void handle_events(int* running, Button buttons[], int buttonCount) {
    SDL_Event event;
    static int lastHover[5] = {0}; // Array to track last hover state for each button

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
                    start_quiz_timer(); // Start the quiz timer
                }
            }
        } else if (currentState == QUIZ) {
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (buttons[2].active || buttons[3].active || buttons[4].active) {
                    Mix_PlayChannel(-1, clickSound, 0); // Play click sound
                }

                SDL_Color feedbackColor;
                const char* feedbackMessage;
                SDL_Surface* feedbackBackground;

                if (buttons[2].active && quizzes[currentQuizIndex].correctAnswer == 1) {
                    feedbackColor = (SDL_Color){0, 255, 0}; // Green for correct
                    feedbackMessage = "Correct answer!";
                    feedbackBackground = load_image("assets/correct_background.png");
                    score++;
                } else if (buttons[3].active && quizzes[currentQuizIndex].correctAnswer == 2) {
                    feedbackColor = (SDL_Color){0, 255, 0}; // Green for correct
                    feedbackMessage = "Correct answer!";
                    feedbackBackground = load_image("assets/correct_background.png");
                    score++;
                } else if (buttons[4].active && quizzes[currentQuizIndex].correctAnswer == 3) {
                    feedbackColor = (SDL_Color){0, 255, 0}; // Green for correct
                    feedbackMessage = "Correct answer!";
                    feedbackBackground = load_image("assets/correct_background.png");
                    score++;
                } else {
                    feedbackColor = (SDL_Color){255, 0, 0}; // Red for incorrect
                    feedbackMessage = "Wrong answer!";
                    feedbackBackground = load_image("assets/wrong_background.png");
                }

                // Center the feedback background and scale it to be larger
                int scaledWidth = feedbackBackground->w * 5.5; // Scale width by 1.5x
                int scaledHeight = feedbackBackground->h * 5.5; // Scale height by 1.5x

                SDL_Surface* scaledFeedbackBackground = SDL_CreateRGBSurface(
                    feedbackBackground->flags, scaledWidth, scaledHeight,
                    feedbackBackground->format->BitsPerPixel,
                    feedbackBackground->format->Rmask, feedbackBackground->format->Gmask,
                    feedbackBackground->format->Bmask, feedbackBackground->format->Amask
                );

                SDL_Rect srcRect = {0, 0, feedbackBackground->w, feedbackBackground->h};
                SDL_Rect dstRect = {0, 0, scaledWidth, scaledHeight};

                // Use SDL_SoftStretch for scaling
                if (SDL_SoftStretch(feedbackBackground, &srcRect, scaledFeedbackBackground, &dstRect) != 0) {
                    fprintf(stderr, "SDL_SoftStretch Error: %s\n", SDL_GetError());
                    SDL_FreeSurface(scaledFeedbackBackground);
                    return;
                }

                int centerX = (SCREEN_W - scaledWidth) /2;
                int centerY = (SCREEN_H - scaledHeight) ;
                apply_surface(centerX, centerY, scaledFeedbackBackground, SDL_GetVideoSurface());

                SDL_FreeSurface(scaledFeedbackBackground);

                // Show feedback message
                show_feedback(SDL_GetVideoSurface(), TTF_OpenFont("assets/font.ttf", 35), feedbackMessage, feedbackColor);
                SDL_FreeSurface(feedbackBackground);

                // Move to the next quiz question
                currentQuizIndex++;
                if (currentQuizIndex >= quizCount) {
                    printf("Quiz finished! Your score: %d\n", score);
                    end_quiz_timer(); // End the quiz timer and display elapsed time
                    double elapsed_time = difftime(time(NULL), quiz_start_time);
                    show_score_window(score, elapsed_time); // Show score window with elapsed time
                    currentState = MAIN_MENU; // Return to the main menu when the quiz is finished
                    currentQuizIndex = 0; // Reset quiz index for next playthrough
                    score = 0; // Reset score for next playthrough
                }
            }
        }
    }
}

void buttonhighlight(SDL_Surface* screen, Button* button) {
    if (button->active) {
        SDL_SetAlpha(button->surface, SDL_SRCALPHA, 128);
    } else {
        SDL_SetAlpha(button->surface, 0, 0);
    }
}

void render(SDL_Surface* screen, SDL_Surface* background, Button buttons[], int buttonCount, TTF_Font* font) {
    // Clear the screen
    SDL_FillRect(screen, NULL, 0);

    // Draw the background
    apply_surface(0, 0, background, screen);

    if (currentState == MAIN_MENU) {
        // Draw the buttons in the main menu
        apply_surface(buttons[0].rect.x, buttons[0].rect.y, buttons[0].surface, screen);
        apply_surface(buttons[1].rect.x, buttons[1].rect.y, buttons[1].surface, screen);
    } else if (currentState == QUIZ) {
        // Draw the quiz elements
        SDL_Color textColor = {255, 255, 255}; // White color for text
        SDL_Color outlineColor = {0, 0, 0};   // Black color for outline

        // Render the question with an outline
        SDL_Surface* questionOutline = TTF_RenderText_Solid(font, quizzes[currentQuizIndex].question, outlineColor);
        SDL_Surface* question = TTF_RenderText_Solid(font, quizzes[currentQuizIndex].question, textColor);

        if (questionOutline && question) {
            int x = 100;
            int y = 100;

            // Render the outline by offsetting the text
            apply_surface(x - 1, y, questionOutline, screen); // Left
            apply_surface(x + 1, y, questionOutline, screen); // Right
            apply_surface(x, y - 1, questionOutline, screen); // Up
            apply_surface(x, y + 1, questionOutline, screen); // Down

            // Render the main text
            apply_surface(x, y, question, screen);

            SDL_FreeSurface(questionOutline);
            SDL_FreeSurface(question);
        }

        // Calculate the positions to center the buttons
        int centerX = (SCREEN_W - buttons[2].surface->w) / 2;
        int centerY = (SCREEN_H - buttons[2].surface->h) / 2;

        // Draw the answer options text above the buttons
        SDL_Surface* answerA = TTF_RenderText_Solid(font, quizzes[currentQuizIndex].answerA, textColor);
        SDL_Surface* answerB = TTF_RenderText_Solid(font, quizzes[currentQuizIndex].answerB, textColor);
        SDL_Surface* answerC = TTF_RenderText_Solid(font, quizzes[currentQuizIndex].answerC, textColor);

        apply_surface(centerX - 300, centerY - 50, answerA, screen); // Option A text
        apply_surface(centerX, centerY - 50, answerB, screen);       // Option B text
        apply_surface(centerX + 300, centerY - 50, answerC, screen); // Option C text

        // Draw the buttons A, B, and C centered
        buttons[2].rect.x = centerX - 300;
        buttons[2].rect.y = centerY;
        buttons[3].rect.x = centerX;
        buttons[3].rect.y = centerY;
        buttons[4].rect.x = centerX + 300;
        buttons[4].rect.y = centerY;

        apply_surface(buttons[2].rect.x, buttons[2].rect.y, buttons[2].surface, screen); // Option A
        apply_surface(buttons[3].rect.x, buttons[3].rect.y, buttons[3].surface, screen); // Option B
        apply_surface(buttons[4].rect.x, buttons[4].rect.y, buttons[4].surface, screen); // Option C

        SDL_FreeSurface(answerA);
        SDL_FreeSurface(answerB);
        SDL_FreeSurface(answerC);

        // Render the score
        char scoreText[50];
        sprintf(scoreText, "Score: %d", score);
        SDL_Surface* scoreSurface = TTF_RenderText_Solid(font, scoreText, textColor);
        apply_surface(10, 10, scoreSurface, screen);
        SDL_FreeSurface(scoreSurface);

        // Render the timer
        render_timer(screen, font);
    }

    // Update the screen
    SDL_Flip(screen);
}

// Function to render the timer on the screen
void render_timer(SDL_Surface* screen, TTF_Font* font) {
    time_t current_time = time(NULL);
    double elapsed_time = difftime(current_time, quiz_start_time);

    char timer_text[50];
    sprintf(timer_text, "Time: %.0f seconds", elapsed_time);

    SDL_Color textColor = {255, 255, 255}; // White color for the timer text
    SDL_Surface* timer_surface = TTF_RenderText_Solid(font, timer_text, textColor);

    if (timer_surface) {
        apply_surface(10, 50, timer_surface, screen); // Render the timer at the top-left corner
        SDL_FreeSurface(timer_surface);
    }
}

void cleanup(SDL_Surface* background, SDL_Surface* quizBtnImg, SDL_Surface* puzzleBtnImg, SDL_Surface* optionA, SDL_Surface* optionB, SDL_Surface* optionC, TTF_Font* font) {
    Mix_FreeChunk(hoverSound);
    Mix_FreeChunk(clickSound); // Free click sound
    Mix_FreeMusic(bgMusic);
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

void show_score_window(int score, double elapsed_time) {
    SDL_Surface* screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32, SDL_SWSURFACE);
    if (!screen) {
        fprintf(stderr, "SDL_SetVideoMode Error: %s\n", SDL_GetError());
        return;
    }
    SDL_WM_SetCaption("Quiz Finished", NULL);

    SDL_Color textColor = {255, 255, 255};
    TTF_Font* font = TTF_OpenFont("assets/font.ttf", 48); // Larger font size for better visibility
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

    char timerMessage[100];
    sprintf(timerMessage, "Time taken: %.0f seconds", elapsed_time);

    SDL_Surface* timerSurface = TTF_RenderText_Solid(font, timerMessage, textColor);
    if (!timerSurface) {
        fprintf(stderr, "TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        SDL_FreeSurface(messageSurface);
        TTF_CloseFont(font);
        return;
    }

    // Fill the screen with a background color (black)
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

    // Center the score message on the screen
    apply_surface((SCREEN_W - messageSurface->w) / 2, (SCREEN_H - messageSurface->h) / 2 - 50, messageSurface, screen);

    // Center the timer message below the score message
    apply_surface((SCREEN_W - timerSurface->w) / 2, (SCREEN_H - timerSurface->h) / 2 + 50, timerSurface, screen);

    SDL_Flip(screen);

    SDL_FreeSurface(messageSurface);
    SDL_FreeSurface(timerSurface);
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

void show_feedback(SDL_Surface* screen, TTF_Font* font, const char* message, SDL_Color color) {
    SDL_Surface* messageSurface = TTF_RenderText_Solid(font, message, color);
    if (!messageSurface) {
        fprintf(stderr, "TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }

    // Centrer le message sur l'écran
    int x = (SCREEN_W - messageSurface->w) / 2;
    int y = (SCREEN_H - messageSurface->h) / 2;

    // Appliquer le message sur l'écran
    apply_surface(x, y, messageSurface, screen);
    SDL_Flip(screen);

    // Attendre un court instant pour que l'utilisateur voie le message
    SDL_Delay(2000);

    SDL_FreeSurface(messageSurface);
}
