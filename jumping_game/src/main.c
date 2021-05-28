#include "raylib.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

static bool basket_flip = false;
static Rectangle apple_dim = { 0, 0, 36, 44 };
static Rectangle basket_dim = { 0, 0, 94, 54 };
static Rectangle tamagotchi_dim = { 0, 0, 69, 80 };

static Texture2D apple_tex;
static Texture2D basket_tex;
static Texture2D tamagotchi_tex;

static float tamagotchi_x = 0.0f;

static int score = 0;
static double time_in_game = 0.0;
static bool in_game = false;
static bool lost_game = false;

typedef struct {
    Vector2 pos;
    bool visible, valid;
} Apple;

static Apple* latest_apple = NULL;

static Apple apples[10];

float randf(float a, float b) {
    return a + ((float)rand() / (float)RAND_MAX) * (b - a);
}

int randi(int a, int b) {
    return (int)randf(a, b);
}

void draw_sprite(Texture2D tex, Rectangle region, Vector2 dst, int alpha) {
    DrawTexturePro(tex, region,
        (Rectangle) {
        dst.x, dst.y, region.width, region.height
    },
        (Vector2) {0,0},
        0.0f,
        (Color) {255,255,255, alpha
        });
}

int main(void) {
    srand(time(NULL));

    const int font_size = 20;
    const int screenWidth = 500;
    const int screenHeight = 500;

    InitAudioDevice();
    SetMasterVolume(1.0f);
    InitWindow(screenWidth, screenHeight, "Tamagotchi");

    SetTargetFPS(60);

    apple_tex = LoadTexture("rsc/sprites/apple.png");
    basket_tex = LoadTexture("rsc/sprites/basket.png");
    tamagotchi_tex = LoadTexture("rsc/sprites/tamagotchi.png");

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(WHITE);

        static float lost_time = 0.0f;

        if (in_game) {
            /* timing */
            double time_since_start = GetTime() - time_in_game;

            if (lost_game) {
                time_since_start = lost_time;
            }

            
            /* run game code */
            if (!lost_game) {
                /* time to create a new apple! */
                if (!latest_apple) {
                    apples[0] = (Apple){ {randi(0, screenWidth - apple_dim.width), 0}, true, true };
                    latest_apple = &apples[0];
                }

                if (latest_apple->pos.y > screenHeight * 0.4f) {
                    for (int i = sizeof(apples) / sizeof(*apples) - 1; i >= 0; i--) {
                        if (!apples[i].valid) {
                            apples[i] = (Apple){ {randi(0, screenWidth - apple_dim.width), 0}, true, true };
                            latest_apple = &apples[i];
                            break;
                        }
                    }
                }

                Rectangle basket_rect = (Rectangle){ roundf(tamagotchi_x + (basket_flip ? -basket_dim.width : tamagotchi_dim.width)), screenHeight - basket_dim.height, basket_dim.width, basket_dim.height };

                /* move all apples down */
                for (int i = sizeof(apples) / sizeof(*apples) - 1; i >= 0; i--) {
                    apples[i].pos.y += GetFrameTime() * (time_since_start * 5.0f + 100.0f);

                    if (apples[i].valid && apples[i].visible) {
                        if (apples[i].pos.y >= screenHeight) {
                            apples[i].valid = apples[i].visible = false;
                            lost_game = true;
                        }
                        else if (CheckCollisionRecs(
                            (Rectangle) {
                            apples[i].pos.x, apples[i].pos.y, apple_dim.width, apple_dim.height
                        },
                            basket_rect)) {
                            apples[i].valid = apples[i].visible = false;
                            score++;
                        }
                    }
                }
                
                if (IsKeyDown(KEY_RIGHT)) {
                    tamagotchi_x += GetFrameTime() * 300.0f;
                    basket_flip = false;
                }
                else if (IsKeyDown(KEY_LEFT)) {
                    tamagotchi_x -= GetFrameTime() * 300.0f;
                    basket_flip = true;
                }

                if (tamagotchi_x < 0.0f)
                    tamagotchi_x = 0.0f;
                else if (tamagotchi_x > screenWidth - tamagotchi_dim.width)
                    tamagotchi_x = screenWidth - tamagotchi_dim.width;
            }

            /* draw apples */
            for (int i = sizeof(apples) / sizeof(*apples) - 1; i >= 0; i--) {
                if (apples[i].visible && apples[i].valid)
                    draw_sprite(apple_tex, apple_dim, apples[i].pos, 255);
            }

            if (IsKeyPressed(KEY_SPACE) && lost_game) {
                in_game = false;
                lost_game = false;
            }
        }
        else {
            tamagotchi_x = screenWidth / 2 - tamagotchi_dim.width / 2;

            /* title screen */
            time_in_game = GetTime();

            const char* prompt = "PRESS SPACE TO START";
            const int text_w = MeasureText(prompt, font_size);
            DrawText(prompt, screenWidth / 2 - text_w / 2, screenHeight / 2 - font_size / 2, font_size, (Color) {0,0,0, (sinf(GetTime() * 2.0f * PI) * 0.5f + 0.5f) * 255.0f });

            if (IsKeyPressed(KEY_SPACE)) {
                score = 0;
                in_game = true;
                basket_flip = false;
                latest_apple = NULL;
                lost_game = false;
                lost_time = 0.0f;
                memset(apples, 0, sizeof(apples));
            }
        }

        draw_sprite(basket_tex, basket_dim, (Vector2) { roundf(tamagotchi_x + (basket_flip ? -basket_dim.width : tamagotchi_dim.width)), screenHeight - basket_dim.height }, 255);
        draw_sprite(tamagotchi_tex, tamagotchi_dim, (Vector2) { roundf(tamagotchi_x), screenHeight - tamagotchi_dim.height }, 255);

        /* score */
        int alpha = (lost_game && in_game) ? (fmodf(GetTime(), 0.75f) > 0.375f ? 0 : 255) : 255;

        char score_fmt[] = "00000";
        char score_str[32] = { 0 };
        sprintf(score_str, "%d", min(in_game ? score : 0, 99999));
        int score_fmt_len = strlen(score_fmt);
        int score_str_len = strlen(score_str);
        for (int i = 0; i < score_str_len; i++)
            score_fmt[score_fmt_len - score_str_len + i] = score_str[i];

        const int text_w = MeasureText(score_fmt, font_size);
        DrawText(score_fmt, screenWidth - text_w - 10, 10, font_size, (lost_game&& in_game) ? (Color) {255,0,0,alpha} : GRAY);

        if (lost_game && in_game) {
            const char* prompt = "GAME OVER";
            const int text_w = MeasureText(prompt, font_size);
            DrawText(prompt, screenWidth / 2 - text_w / 2, screenHeight / 2 - font_size / 2, font_size, (Color) { 0, 0 ,0, alpha});
        }

        EndDrawing();
    }

    CloseWindow();
    
    return 0;
}