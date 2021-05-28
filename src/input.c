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

    // Arrow keys do not fall into normal ascii values, need special case for
    // them. Store in first four slots of keyboard state arrays.
    switch(key){
        case SDLK_RIGHT:
            if(!_keyboard.down[RIGHT_ARROW_KEY]) _keyboard.clicked[RIGHT_ARROW_KEY] = SDL_TRUE;
            _keyboard.down[RIGHT_ARROW_KEY] = SDL_TRUE;
            break;
        case SDLK_LEFT:
            if(!_keyboard.down[LEFT_ARROW_KEY]) _keyboard.clicked[LEFT_ARROW_KEY] = SDL_TRUE;
            _keyboard.down[LEFT_ARROW_KEY] = SDL_TRUE;
            break;
        case SDLK_UP:
            if(!_keyboard.down[UP_ARROW_KEY]) _keyboard.clicked[UP_ARROW_KEY] = SDL_TRUE;
            _keyboard.down[UP_ARROW_KEY] = SDL_TRUE;
            break;
        case SDLK_DOWN:
            if(!_keyboard.down[DOWN_ARROW_KEY]) _keyboard.clicked[DOWN_ARROW_KEY] = SDL_TRUE;
            _keyboard.down[DOWN_ARROW_KEY] = SDL_TRUE;
            break;
        default:{
            SDL_Keycode adjusted_key = key + 4;
            if(adjusted_key < NUM_KEYBOARD_KEYS){
                if(!_keyboard.down[adjusted_key]) _keyboard.clicked[adjusted_key] = SDL_TRUE;
                _keyboard.down[adjusted_key] = SDL_TRUE;
            }
            break;
        }
    }
}

void KeyboardHandleKeyUp(SDL_Keycode key){

    // Same things as handle keydown, arrow keys will fall outside of keyboard state
    // arrays so make special case and reserve first four slots for them.
    switch(key){
        case SDLK_RIGHT:
            _keyboard.down[RIGHT_ARROW_KEY] = SDL_FALSE;
            _keyboard.released[RIGHT_ARROW_KEY] = SDL_TRUE;
            break;
        case SDLK_LEFT:
            _keyboard.down[LEFT_ARROW_KEY] = SDL_FALSE;
            _keyboard.released[LEFT_ARROW_KEY] = SDL_TRUE;
            break;
        case SDLK_UP:
            _keyboard.down[UP_ARROW_KEY] = SDL_FALSE;
            _keyboard.released[UP_ARROW_KEY] = SDL_TRUE;
            break;
        case SDLK_DOWN:
            _keyboard.down[DOWN_ARROW_KEY] = SDL_FALSE;
            _keyboard.released[DOWN_ARROW_KEY] = SDL_TRUE;
            break;
        default:{
            SDL_Keycode adjusted_key = key + 4;
            if(adjusted_key < NUM_KEYBOARD_KEYS){
                _keyboard.down[adjusted_key] = SDL_FALSE;
                _keyboard.released[adjusted_key] = SDL_TRUE;
            }
            break;
        }
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
    SDL_bool clicked;
    switch(key){
        case SDLK_RIGHT:
            clicked = _keyboard.clicked[RIGHT_ARROW_KEY];
            _keyboard.clicked[RIGHT_ARROW_KEY] = SDL_FALSE;
            break;
        case SDLK_LEFT:
            clicked = _keyboard.clicked[LEFT_ARROW_KEY];
            _keyboard.clicked[LEFT_ARROW_KEY] = SDL_FALSE;
            break;
        case SDLK_UP:
            clicked = _keyboard.clicked[UP_ARROW_KEY];
            _keyboard.clicked[UP_ARROW_KEY] = SDL_FALSE;
            break;
        case SDLK_DOWN:
            clicked = _keyboard.clicked[DOWN_ARROW_KEY];
            _keyboard.clicked[DOWN_ARROW_KEY] = SDL_FALSE;
            break;
        default:{
            SDL_Keycode adjusted_key = key + 4;
            if(adjusted_key < NUM_KEYBOARD_KEYS){
                clicked = _keyboard.clicked[adjusted_key];
                _keyboard.clicked[adjusted_key] = SDL_FALSE;
            }
            break;
        }
    }
    return clicked;
}

SDL_bool KeyReleased(SDL_Keycode key){
    SDL_bool released;
    switch(key){
        case SDLK_RIGHT:
            released = _keyboard.released[RIGHT_ARROW_KEY];
            _keyboard.released[RIGHT_ARROW_KEY] = SDL_FALSE;
            break;
        case SDLK_LEFT:
            released = _keyboard.released[LEFT_ARROW_KEY];
            _keyboard.released[LEFT_ARROW_KEY] = SDL_FALSE;
            break;
        case SDLK_UP:
            released = _keyboard.released[UP_ARROW_KEY];
            _keyboard.released[UP_ARROW_KEY] = SDL_FALSE;
            break;
        case SDLK_DOWN:
            released = _keyboard.released[DOWN_ARROW_KEY];
            _keyboard.released[DOWN_ARROW_KEY] = SDL_FALSE;
            break;
        default:{
            SDL_Keycode adjusted_key = key + 4;
            if(adjusted_key < NUM_KEYBOARD_KEYS){
                released = _keyboard.released[adjusted_key];
                _keyboard.released[adjusted_key] = SDL_FALSE;
            }
            break;
        }
    }
    return released;
}

SDL_bool KeyDown(SDL_Keycode key){
    switch(key){
        case SDLK_RIGHT: return _keyboard.down[RIGHT_ARROW_KEY];
        case SDLK_LEFT: return _keyboard.down[LEFT_ARROW_KEY];
        case SDLK_UP: return _keyboard.down[UP_ARROW_KEY];
        case SDLK_DOWN: return _keyboard.down[DOWN_ARROW_KEY];
        default:{
            SDL_Keycode adjusted_key = key + 4;
            if(adjusted_key < NUM_KEYBOARD_KEYS){
                return _keyboard.down[adjusted_key];
            }
        }
    }
    return SDL_FALSE;
}
