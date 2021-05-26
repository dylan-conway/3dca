#include <stdio.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <cglm/cglm.h>

#include "shaders.h"
#include "meshes.h"
#include "camera.h"
#include "defines.h"

SDL_GLContext ctx = NULL;
SDL_Window* window = NULL;
struct Camera cam;

SDL_bool wireframe = SDL_FALSE;

int INIT();

int main(int argc, char** argv){

    // Init SDL2, OpenGL, GLEW, and create shaders
    if(INIT() == -1){
        return -1;
    }

    cam = InitCamera();

    struct Cube test_cube = InitCube();

    SDL_bool running = SDL_TRUE;
    SDL_Event event;

    SDL_bool right_key = SDL_FALSE;
    SDL_bool left_key = SDL_FALSE;
    SDL_bool up_key = SDL_FALSE;
    SDL_bool down_key = SDL_FALSE;

    Uint64 curr_frame_time, prev_frame_time;
    prev_frame_time = SDL_GetPerformanceCounter();
    while(running){
        curr_frame_time = SDL_GetPerformanceCounter();
        double delta = (double)(curr_frame_time - prev_frame_time) / (double)SDL_GetPerformanceFrequency();

        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    running = SDL_FALSE;
                    break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym){
                        case SDLK_ESCAPE:
                            running = SDL_FALSE;
                            break;
                        case SDLK_RIGHT:
                            right_key = SDL_TRUE;
                            break;
                        case SDLK_LEFT:
                            left_key = SDL_TRUE;
                            break;
                        case SDLK_UP:
                            up_key = SDL_TRUE;
                            break;
                        case SDLK_DOWN:
                            down_key = SDL_TRUE;
                            break;
                        case SDLK_SPACE:
                            if(wireframe){
                                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                                wireframe = SDL_FALSE;
                            } else {
                                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                                wireframe = SDL_TRUE;
                            }
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    switch(event.key.keysym.sym){
                        case SDLK_RIGHT:
                            right_key = SDL_FALSE;
                            break;
                        case SDLK_LEFT:
                            left_key = SDL_FALSE;
                            break;
                        case SDLK_UP:
                            up_key = SDL_FALSE;
                            break;
                        case SDLK_DOWN:
                            down_key = SDL_FALSE;
                            break;
                    }
                    break;
            }
        }

        if(right_key){
            
            cam.position[0] += 10.0f * delta;
        }

        if(left_key){
            cam.position[0] += -10.0f * delta;
        }

        if(up_key){
            cam.position[1] += 10.0f * delta;
        }

        if(down_key){
            cam.position[1] += -10.0f * delta;
        }
        
        mat4 mvp;
        mat4 model = GLM_MAT4_IDENTITY_INIT;
        GetMVP(&cam, &model, mvp);

        // Update shader uniforms
        UpdateMVPUniform(mvp);
        UpdateCameraUniform(cam.position);

        vec3 light_direction;
        glm_vec3_sub((vec3){0.0f, 0.0f, 0.0f}, cam.position, light_direction);
        glm_vec3_normalize(light_direction);
        UpdateLightDirectionUniform(light_direction);

        // Clear window
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



        // Draw elements
        DrawCube(&test_cube);

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

int INIT(){

    // Initialize sdl2 and print version
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
        printf("SDL INIT FAILED\n");
        return -1;
    }

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
