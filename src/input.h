#ifndef _INPUT_H_
#define _INPUT_H_

#include <stdio.h>
#include <SDL2/SDL.h>
#include <cglm/vec2.h>

#define NUM_KEYBOARD_KEYS 128
#define NUM_MOUSE_BUTTONS 32

struct Mouse {
    vec2 curr_pos, prev_pos;
    Sint32 wheel_move;
    SDL_bool wheel_moved;

    SDL_bool down[NUM_MOUSE_BUTTONS];
    SDL_bool released[NUM_MOUSE_BUTTONS];
    SDL_bool clicked[NUM_MOUSE_BUTTONS];
};

struct Keyboard {
    SDL_bool down[NUM_KEYBOARD_KEYS];
    SDL_bool released[NUM_KEYBOARD_KEYS];
    SDL_bool clicked[NUM_KEYBOARD_KEYS];
};

void InitInput();
void UpdateInput();


SDL_bool ButtonClicked(Uint8 button);
SDL_bool ButtonReleased(Uint8 button);
SDL_bool ButtonDown(Uint8 button);

void MouseHandleButtonDown(Uint8 button);
void MouseHandleButtonUp(Uint8 button);
void MouseHandleWheel(SDL_MouseWheelEvent wheel_event);


/**
 * Check if a key is clicked. Will only happen once per button press.
 * Sets clicked to false before returning.
 */
SDL_bool KeyClicked(SDL_Keycode key);

/**
 * Check for a key release. Happens when user takes finger off key.
 * Set released to false before returning.
 */
SDL_bool KeyReleased(SDL_Keycode key);

/**
 * Check for a key that is down. Different from clicked, this fires the entire
 * time a key is down.
 */
SDL_bool KeyDown(SDL_Keycode key);

void KeyboardHandleKeyDown(SDL_Keycode key);
void KeyboardHandleKeyUp(SDL_Keycode key);

#endif