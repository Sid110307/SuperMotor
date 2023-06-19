#include <iostream>
#include <vector>
#include <cmath>

#include "include/utils.h"

constexpr int SCREEN_WIDTH = 800, SCREEN_HEIGHT = 600;

constexpr double WHEEL_RADIUS = 100;
constexpr double MOTOR_RADIUS = 50;
constexpr double BELT_WIDTH = 20;
constexpr double MOTOR_SPEED = 1;
constexpr double CRITICAL_THRESHOLD = 10;
constexpr double MOTOR_SPACING = 75;

class Wheel
{
public:
    Wheel(double x, double y, double radius, SDL_Renderer *renderer) : x(x), y(y), radius(radius), renderer(renderer) {}

    void draw()
    {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        for (int angle = 0; angle < 360; ++angle)
            SDL_RenderDrawPoint(renderer, (int) (x + radius * cos(angle * M_PI / 180 + rotation)),
                                (int) (y + radius * sin(angle * M_PI / 180 + rotation)));

        for (int angle = 0; angle < 360; angle += 45)
            SDL_RenderDrawLine(renderer, (int) x, (int) y, (int) (x + radius * cos(angle * M_PI / 180 + rotation)),
                               (int) (y + radius * sin(angle * M_PI / 180 + rotation)));
    }

    void update(double speed)
    {
        rotation += speed * M_PI / 180;

        while (rotation >= 2 * M_PI) rotation -= 2 * M_PI;
        while (rotation < 0) rotation += 2 * M_PI;
    }

    double x, y, radius;
    double rotation = 0;

private:
    SDL_Renderer *renderer;
};

class Motor
{
public:
    Motor(double x, double y, double radius, double speed, SDL_Renderer *renderer, SDL_Texture *texture)
            : x(x), y(y), radius(radius), speed(speed), renderer(renderer), texture(texture) {}

    void draw()
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

        SDL_Rect rect = {(int) (x - radius), (int) (y - radius), (int) (2 * radius), (int) (2 * radius)};
        SDL_RenderCopyEx(renderer, texture, nullptr, &rect, rotation * 180 / M_PI, nullptr, SDL_FLIP_NONE);
    }

    void update()
    {
        rotation += speed * M_PI / 180;
    }

    double x, y, radius, speed, charge = 100, rotation = 0;

private:
    SDL_Renderer *renderer;
    SDL_Texture *texture;
};

class Belt
{
public:
    Belt(SDL_Renderer *renderer, Wheel wheel, Motor motor) : renderer(renderer), wheel(wheel), motor(motor) {}

    void draw()
    {
        double x1 = wheel.x + wheel.radius * cos(wheel.rotation);
        double y1 = wheel.y + wheel.radius * sin(wheel.rotation);
        double x2 = motor.x + motor.radius * cos(motor.rotation);
        double y2 = motor.y + motor.radius * sin(motor.rotation);

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

        SDL_RenderDrawLine(renderer, (int) x1, (int) (y1 + BELT_WIDTH), (int) x2, (int) (y2 + BELT_WIDTH));
        SDL_RenderDrawLine(renderer, (int) x1, (int) (y1 - BELT_WIDTH), (int) x2, (int) (y2 - BELT_WIDTH));
    }

private:
    SDL_Renderer *renderer;

    Wheel wheel;
    Motor motor;
};

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Window *window = SDL_CreateWindow("SuperMotor", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                          SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr)
    {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    if (TTF_Init() < 0)
    {
        std::cerr << "SDL TTF could not initialize! SDL TTF Error: " << TTF_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Texture *motorATexture = IMG_LoadTexture(renderer, "lib/motorA.png");
    SDL_Texture *motorBTexture = IMG_LoadTexture(renderer, "lib/motorB.png");

    Motor motorA(SCREEN_WIDTH / 4.0, SCREEN_HEIGHT / 2.0 - MOTOR_SPACING, MOTOR_RADIUS, MOTOR_SPEED, renderer,
                 motorATexture);
    Motor motorB(SCREEN_WIDTH / 4.0, SCREEN_HEIGHT / 2.0 + MOTOR_SPACING, MOTOR_RADIUS, 0, renderer,
                 motorBTexture);

    Wheel wheel(SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0, WHEEL_RADIUS, renderer);

    Belt beltA(renderer, wheel, motorA);
    Belt beltB(renderer, wheel, motorB);

    bool quit = false;
    SDL_Event e;

    double originalMotorASpeed = motorA.speed, originalMotorBSpeed = motorB.speed;

    while (!quit)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT) quit = true;
            if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                    case SDLK_UP:
                        motorA.speed += 0.1;
                        break;
                    case SDLK_DOWN:
                        motorA.speed -= 0.1;
                        break;
                    case SDLK_LEFT:
                        motorB.speed -= 0.1;
                        break;
                    case SDLK_RIGHT:
                        motorB.speed += 0.1;
                        break;
                    case SDLK_SPACE:
                        motorA.speed = MOTOR_SPEED;
                        motorB.speed = 0;

                        motorA.charge = 100;
                        motorB.charge = 100;

                        break;
                    case SDLK_HOME:
                        if (motorA.speed == 0 && motorB.speed == 0)
                        {
                            motorA.speed = originalMotorASpeed;
                            motorB.speed = originalMotorBSpeed;
                        } else
                        {
                            originalMotorASpeed = motorA.speed;
                            originalMotorBSpeed = motorB.speed;

                            motorA.speed = 0;
                            motorB.speed = 0;
                        }

                        break;
                    default:
                        break;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        beltA.draw();
        beltB.draw();
        wheel.draw();
        motorA.draw();
        motorB.draw();

        wheel.update((motorA.speed + motorB.speed) / 2);
        motorA.update();
        motorB.update();

        if (motorA.charge > 100) motorA.charge = 100;
        if (motorB.charge > 100) motorB.charge = 100;

        if (motorA.charge <= CRITICAL_THRESHOLD)
        {
            motorA.speed = 0;
            motorB.speed += 1;
        } else if (motorB.charge <= CRITICAL_THRESHOLD)
        {
            motorA.speed += 1;
            motorB.speed = 0;
        }

        if (motorA.speed > 0)
        {
            motorB.charge += 0.1;
            motorA.charge -= 0.1;
            wheel.rotation -= (double) motorA.speed * M_PI / 180;
        } else if (motorB.speed > 0)
        {
            motorA.charge += 0.1;
            motorB.charge -= 0.1;
            wheel.rotation -= (double) motorB.speed * M_PI / 180;
        }

        SDL_Color textColor = {0, 0, 0, 255};
        TTF_Font *font = TTF_OpenFont("lib/Hack.ttf", 11);
        if (font == nullptr)
        {
            std::cerr << "Failed to load font! SDL TTF Error: " << TTF_GetError() << std::endl;
            return EXIT_FAILURE;
        }

        drawText(renderer, font, "Motor A Charge: " + std::to_string(motorA.charge) + "%", textColor, 10, 10);
        drawText(renderer, font, "Motor B Charge: " + std::to_string(motorB.charge) + "%", textColor, 10, 25);
        drawText(renderer, font, "Motor A speed: " + std::to_string(motorA.speed), textColor, 10, 40);
        drawText(renderer, font, "Motor B speed: " + std::to_string(motorB.speed), textColor, 10, 55);
        drawText(renderer, font, "Controls:", textColor, 10, 85);
        drawText(renderer, font, "Up/Down arrows: change motor A speed", textColor, 10, 100);
        drawText(renderer, font, "Left/Right arrows: change motor B speed", textColor, 10, 115);
        drawText(renderer, font, "Home: stop/resume motors", textColor, 10, 130);
        drawText(renderer, font, "Space: reset", textColor, 10, 145);
        drawText(renderer, font, "Escape: quit", textColor, 10, 160);

        TTF_CloseFont(font);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(motorATexture);
    SDL_DestroyTexture(motorBTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return EXIT_SUCCESS;
}
