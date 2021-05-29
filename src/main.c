#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <cglm/mat4.h>
#include <cglm/vec3.h>
#include <cglm/vec2.h>
#include <cglm/affine.h>
#include <cglm/version.h>

#include "shaders.h"
#include "meshes.h"
#include "camera.h"
#include "defines.h"
#include "input.h"

SDL_GLContext ctx = NULL;
SDL_Window* window = NULL;

SDL_bool wireframe = SDL_FALSE;

int*** main_grid = NULL;
int*** update_grid = NULL;


int INIT();

double GetDeltaTime(Uint64 start_time, Uint64 end_time);

SDL_bool CheckCell(int*** grid, int x, int y, int z);
int CountNeighbors(int x, int y, int z);
void UpdateCells();


int main(int argc, char** argv){

    srand(5546);

    // Init SDL2, OpenGL, GLEW, and create shaders
    if(INIT() == -1){
        return -1;
    }

    InitInput();
    InitCubeVAO();

    main_grid = malloc(sizeof(int**) * X_CELLS);
    for(int i = 0; i < X_CELLS; i ++){
        main_grid[i] = malloc(sizeof(int*) * Y_CELLS);
        for(int j = 0; j < Y_CELLS; j ++){
            main_grid[i][j] = malloc(sizeof(int) * Z_CELLS);
        }
    }

    for(int x = 0; x < X_CELLS; x ++){
        for(int y = 0; y < Y_CELLS; y ++){
            for(int z = 0; z < Z_CELLS; z ++){
                main_grid[x][y][z] = rand() % 2;
            }
        }
    }

    update_grid = malloc(sizeof(int**) * X_CELLS);
    for(int i = 0; i < X_CELLS; i ++){
        update_grid[i] = malloc(sizeof(int*) * Y_CELLS);
        for(int j = 0; j < Y_CELLS; j ++){
            update_grid[i][j] = malloc(sizeof(int) * Z_CELLS);
        }
    }

    for(int x = 0; x < X_CELLS; x ++){
        for(int y = 0; y < Y_CELLS; y ++){
            for(int z = 0; z < Z_CELLS; z ++){
                update_grid[x][y][z] = 0;
            }
        }
    }

    Camera_Init();
    Camera_SetPosition((vec3){0.0f, 15.0f, 70.0f});

    Uint64 grid_last_update_time = SDL_GetPerformanceCounter();

    SDL_bool running = SDL_TRUE;
    SDL_Event event;

    int counter = 0;
    Uint64 curr_frame_time, prev_frame_time;
    prev_frame_time = SDL_GetPerformanceCounter();
    while(running){
        curr_frame_time = SDL_GetPerformanceCounter();
        double delta = GetDeltaTime(prev_frame_time, curr_frame_time);

        counter ++;

        UpdateInput();

        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    running = SDL_FALSE;
                    continue;
                    break;
                case SDL_KEYDOWN:
                    KeyboardHandleKeyDown(event.key.keysym.sym);
                    break;
                case SDL_KEYUP:
                    KeyboardHandleKeyUp(event.key.keysym.sym);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    MouseHandleButtonDown(event.button.button);
                    break;
                case SDL_MOUSEBUTTONUP:
                    MouseHandleButtonUp(event.button.button);
                    break;
                case SDL_MOUSEWHEEL:{
                    MouseHandleWheel(event.wheel);
                    break;
                }
            }
        }

        if(KeyClicked(SDLK_ESCAPE)){
            running = SDL_FALSE;
            continue;
        }

        if(KeyClicked(SDLK_SPACE)){
            if(wireframe){
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                wireframe = SDL_FALSE;
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                wireframe = SDL_TRUE;
            }
        }

        if(ButtonDown(SDL_BUTTON_LEFT)){
            vec3 cam_position;
            Camera_GetPosition(cam_position);

            vec2 curr_mouse_pos, prev_mouse_pos;
            Mouse_GetCurrPos(curr_mouse_pos);
            Mouse_GetPrevPos(prev_mouse_pos);

            // float mouse_diff_x = curr_mouse_pos[0] - prev_mouse_pos[0];

            float cam_x = cam_position[0];
            float cam_y = cam_position[1];
            float cam_z = cam_position[2];
            
            Camera_SetPosition((vec3){cam_x, cam_y, cam_z});
        }

        if(KeyDown(SDLK_RIGHT)){
            Camera_MovePosition((vec3){50.0f * delta, 0.0f, 0.0f});
        }

        if(KeyDown(SDLK_LEFT)){
            Camera_MovePosition((vec3){-50.0f * delta, 0.0f, 0.0f});
        }

        Uint64 current_time = SDL_GetPerformanceCounter();
        double grid_update_delta_time = GetDeltaTime(grid_last_update_time, current_time);
        if(grid_update_delta_time >= GRID_UPDATE_INTERVAL){
            grid_last_update_time = current_time;
            UpdateCells();
        }

        vec3 cam_position;
        Camera_GetPosition(cam_position);
        
        UpdateCameraUniform(cam_position);

        vec3 light_direction;
        glm_vec3_sub((vec3){0.0f, 0.0f, 0.0f}, cam_position, light_direction);
        glm_vec3_normalize(light_direction);
        UpdateLightDirectionUniform(light_direction);

        // Clear window
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // Draw elements
        for(int x = 0; x < X_CELLS; x ++){
            for(int y = 0; y < Y_CELLS; y ++){
                for(int z = 0; z < Z_CELLS; z ++){
                    if(main_grid[x][y][z]){
                        DrawStaticCube((vec3){x - X_CELLS / 2, y - Y_CELLS / 2, z - Z_CELLS / 2});
                    }
                }
            }
        }

        // Swap buffers
        SDL_GL_SwapWindow(window);

        prev_frame_time = curr_frame_time;
    }

    DeleteShaders();
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();    

    return 0;
}

double GetDeltaTime(Uint64 start_time, Uint64 end_time){
    return (double)(end_time - start_time) / SDL_GetPerformanceFrequency();
}


int CountNeighbors(int x, int y, int z){
    int count = 0;
    for(int i = -1; i < 2; i ++){
        for(int j = -1; j < 2; j ++){
            for(int k = -1; k < 2; k ++){
                if(i == 0 && j == 0 && z == 0) continue;
                if(CheckCell(main_grid, x + i, y + j, z + k)){
                    count ++;
                }
            }
        }
    }
    return count;
}

SDL_bool CheckCell(int*** grid, int x, int y, int z){
    if(x < 0 || x >= X_CELLS){
        return SDL_FALSE;
    }
    if(y < 0 || y >= Y_CELLS){
        return SDL_FALSE;
    }
    if(z < 0 || z >= Z_CELLS){
        return SDL_FALSE;
    }
    return (SDL_bool)grid[x][y][z];
}

void UpdateCells(){

    for(int x = 0; x < X_CELLS; x ++){
        for(int y = 0; y < Y_CELLS; y ++){
            for(int z = 0; z < Z_CELLS; z ++){
                update_grid[x][y][z] = 0;
            }
        }
    }

    /**
     * Interesting rules:
     * - B 8-11 S 11-16
     */
    for(int x = 0; x < X_CELLS; x ++){
        for(int y = 0; y < Y_CELLS; y ++){
            for(int z = 0; z < Z_CELLS; z ++){
                int num_neighbors = CountNeighbors(x, y, z);
                if(CheckCell(main_grid, x, y, z)){
                    // S
                    if(num_neighbors < 11 || num_neighbors > 16){
                        update_grid[x][y][z] = 1;
                    }
                } else {
                    // B
                    if(num_neighbors >= 8 && num_neighbors <= 11){
                        update_grid[x][y][z] = 1;
                    }
                }
            }
        }
    }

    for(int x = 0; x < X_CELLS; x ++){
        for(int y = 0; y < Y_CELLS; y ++){
            for(int z = 0; z < Z_CELLS; z ++){
                if(update_grid[x][y][z]){
                    if(main_grid[x][y][z]){
                        main_grid[x][y][z] = 0;
                    } else {
                        main_grid[x][y][z] = 1;
                    }
                    update_grid[x][y][z] = 0;
                }
            }
        }
    }
}

int INIT(){

    // Initialize sdl2 and print version
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
        printf("SDL INIT FAILED\n");
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    // Make an sdl2 window for opengl
    Uint32 flags;
    flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    window = SDL_CreateWindow(
        "441 Final",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_W, WINDOW_H, flags
    );
    if(window == NULL){
        printf("WINDOW CREATION FAILED\n");
        return -1;
    }

    // Create the opengl context
    ctx = SDL_GL_CreateContext(window);
    if(ctx == NULL){
        printf("SDL GL CREATE CONTEXT FAILED\n");
        return -1;
    }
    SDL_GL_SetSwapInterval(0);
    glViewport(0, 0, WINDOW_W, WINDOW_H);

    // Initialize glew
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK){
        printf("GLEW INIT FAILED\n");
        return -1;
    }

    // Print versions
    SDL_version version;
    SDL_VERSION(&version);
    printf("SDL %d.%d.%d\n", version.major, version.minor, version.patch);
    printf("CGLM %d.%d.%d\n", CGLM_VERSION_MAJOR, CGLM_VERSION_MINOR, CGLM_VERSION_PATCH);
    printf("GLEW %s\n", glewGetString(GLEW_VERSION));
    printf("RENDERER: %s\n", glGetString(GL_RENDERER));
    printf("OPENGL %s\n", glGetString(GL_VERSION));

    // GL will only draw pixels closer to view (smaller value is closer)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Create a shader program for opengl and compile, attach, link shaders
    if(CreateShaders() == -1){
        printf("ERROR: CreateShaders failed\n");
        return -1;
    }
    
    return 0;
}
