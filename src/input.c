#include "input.h"

static struct Mouse _mouse;
static struct Keyboard _keyboard;

void _InitMouse();
void _UpdateMouse();

void _InitKeyboard();
void _UpdateKeyboard();

void InitInput(){
    _InitMouse();
    _InitKeyboard();
}

void UpdateInput(){
    _UpdateMouse();
    _UpdateKeyboard();
}

void MouseHandleButtonDown(Uint8 button){
    if(button < NUM_MOUSE_BUTTONS){
        if(!_mouse.down[button]) _mouse.clicked[button] = SDL_TRUE;
        _mouse.down[button] = SDL_TRUE;
    }
}



void MouseHandleButtonUp(Uint8 button){
    if(button < NUM_MOUSE_BUTTONS){
        _mouse.down[button] = SDL_FALSE;
        _mouse.released[button] = SDL_TRUE;
    }
}

void MouseHandleWheel(SDL_MouseWheelEvent wheel_event){
    _mouse.wheel_move = wheel_event.y;
    _mouse.wheel_moved = SDL_TRUE;
}

void KeyboardHandleKeyDown(SDL_Keycode key){
    if(key < NUM_KEYBOARD_KEYS){
        if(!_keyboard.down[key]) _keyboard.clicked[key] = SDL_TRUE;
        _keyboard.down[key] = SDL_TRUE;
    }
}

void KeyboardHandleKeyUp(SDL_Keycode key){
    if(key < NUM_KEYBOARD_KEYS){
        _keyboard.down[key] = SDL_FALSE;
        _keyboard.released[key] = SDL_TRUE;
    }
}


void _InitMouse(){
    int x, y;
    SDL_GetMouseState(&x, &y);
    _mouse.curr_pos[0] = x;
    _mouse.curr_pos[1] = y;
    glm_vec2_copy(_mouse.curr_pos, _mouse.prev_pos);
    _mouse.wheel_move = 0;
    _mouse.wheel_moved = SDL_FALSE;

    SDL_memset(_mouse.down, SDL_FALSE, sizeof(SDL_bool) * NUM_MOUSE_BUTTONS);
    SDL_memset(_mouse.released, SDL_FALSE, sizeof(SDL_bool) * NUM_MOUSE_BUTTONS);
    SDL_memset(_mouse.clicked, SDL_FALSE, sizeof(SDL_bool) * NUM_MOUSE_BUTTONS);
}

void _UpdateMouse(){
    glm_vec2_copy(_mouse.curr_pos, _mouse.prev_pos);
    int x, y;
    SDL_GetMouseState(&x, &y);
    _mouse.curr_pos[0] = x;
    _mouse.curr_pos[1] = y;
    _mouse.wheel_move = 0;
    _mouse.wheel_moved = SDL_FALSE;

    SDL_memset(_mouse.released, SDL_FALSE, sizeof(SDL_bool) * NUM_MOUSE_BUTTONS);
    SDL_memset(_mouse.clicked, SDL_FALSE, sizeof(SDL_bool) * NUM_MOUSE_BUTTONS);
}

SDL_bool ButtonClicked(Uint8 button){
    SDL_bool clicked = _mouse.clicked[button];
    _mouse.clicked[button] = SDL_FALSE;
    return clicked;
}

SDL_bool ButtonReleased(Uint8 button){
    SDL_bool released = _mouse.released[button];
    _mouse.released[button] = SDL_FALSE;
    return released;
}

SDL_bool ButtonDown(Uint8 button){
    return _mouse.down[button];
}



void _InitKeyboard(){
    SDL_memset(_keyboard.down, SDL_FALSE, sizeof(SDL_bool) * NUM_KEYBOARD_KEYS);
    SDL_memset(_keyboard.released, SDL_FALSE, sizeof(SDL_bool) * NUM_KEYBOARD_KEYS);
    SDL_memset(_keyboard.clicked, SDL_FALSE, sizeof(SDL_bool) * NUM_KEYBOARD_KEYS);
}

void _UpdateKeyboard(){
    SDL_memset(_keyboard.released, SDL_FALSE, sizeof(SDL_bool) * NUM_KEYBOARD_KEYS);
    SDL_memset(_keyboard.clicked, SDL_FALSE, sizeof(SDL_bool) * NUM_KEYBOARD_KEYS);
}


SDL_bool KeyClicked(SDL_Keycode key){
    SDL_bool clicked = _keyboard.clicked[key];
    _keyboard.clicked[key] = SDL_FALSE;
    return clicked;
}

SDL_bool KeyReleased(SDL_Keycode key){
    SDL_bool released = _keyboard.released[key];
    _keyboard.released[key] = SDL_FALSE;
    return released;
}

SDL_bool KeyDown(SDL_Keycode key){
    return _keyboard.down[key];
}
