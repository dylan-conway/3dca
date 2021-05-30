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

enum STATE {
    EDITOR,
    SIMULATION
};

SDL_GLContext ctx = NULL;
SDL_Window* window = NULL;
SDL_bool running = SDL_TRUE;
enum STATE state = EDITOR;

SDL_bool wireframe = SDL_FALSE;

int*** main_grid = NULL;
int*** update_grid = NULL;

int INIT();
void UPDATE();
void DRAW();
void FREE();

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
    Camera_Init();
    Camera_SetPosition((vec3){0.0f, 25.0f, 70.0f});

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

    Uint64 grid_last_update_time = SDL_GetPerformanceCounter();
    SDL_Event event;
    Uint64 curr_frame_time, prev_frame_time;
    prev_frame_time = SDL_GetPerformanceCounter();
    while(running){

        curr_frame_time = SDL_GetPerformanceCounter();
        double delta = GetDeltaTime(prev_frame_time, curr_frame_time);
        delta ++;

        // Process user input
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


        // Update program state from user input

        if(KeyClicked(SDLK_ESCAPE)){
            running = SDL_FALSE;
            continue;
        }

        if(KeyClicked(SDLK_w)){
            if(wireframe){
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                wireframe = SDL_FALSE;
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                wireframe = SDL_TRUE;
            }
        }



        Sint32 wheel_move;
        if(Mouse_WheelMoved(&wheel_move)){

            // TODO: ZOOM

            // Sint32 wheel_move = Mouse_GetWheelMovement();
            // double step = 10.0f;
            // // camera_radius_from_origin += wheel_move * step * delta;
            // Camera_MovePosition((vec3){0.0f, wheel_move, wheel_move});
        }

        if(ButtonDown(SDL_BUTTON_MIDDLE)){

            vec3 cam_position;
            Camera_GetPosition(cam_position);

            vec2 curr_mouse_pos, prev_mouse_pos;
            Mouse_GetCurrPos(curr_mouse_pos);
            Mouse_GetPrevPos(prev_mouse_pos);

            float mouse_diff_x = curr_mouse_pos[0] - prev_mouse_pos[0];
            if(mouse_diff_x != 0){
                double rotation_step = mouse_diff_x  * 0.004f;
                Camera_RotateAroundOrigin(rotation_step);
            }
        }

        // Update uniforms with new camera position
        vec3 cam_position;
        Camera_GetPosition(cam_position);
        UpdateCameraUniform(cam_position);
        vec3 light_direction;
        glm_vec3_sub((vec3){0.0f, 0.0f, 0.0f}, cam_position, light_direction);
        glm_vec3_normalize(light_direction);
        UpdateLightDirectionUniform(light_direction);

        if(ButtonDown(SDL_BUTTON_RIGHT)){
            
            // Make a ray from mouse coordinates. Origin of ray is camera position
            // https://antongerdelan.net/opengl/raycasting.html

            vec2 mouse_coords;
            Mouse_GetCurrPos(mouse_coords);

            float x = (2.0f * mouse_coords[0]) / WINDOW_W - 1.0f;
            float y = 1.0f - (2.0f * mouse_coords[1]) / WINDOW_H;
            float z = 1.0f;

            vec3 ray_nds = {x, y, z};

            vec4 ray_clip = {
                ray_nds[0],
                ray_nds[1],
                -1.0f, 1.0f
            };

            mat4 proj_matrix, inv_proj_matrix;
            Camera_GetProjectionMatrix(proj_matrix);
            glm_mat4_inv(proj_matrix, inv_proj_matrix);

            vec4 ray_eye;
            glm_mat4_mulv(inv_proj_matrix, ray_clip, ray_eye);
            ray_eye[2] = -1.0f;
            ray_eye[3] = 0.0f;

            vec4 ray_world_temp;
            mat4 view_matrix, inv_view_matrix;
            Camera_GetViewMatrix(view_matrix);
            glm_mat4_inv(view_matrix, inv_view_matrix);
            glm_mat4_mulv(inv_view_matrix, ray_eye, ray_world_temp);

            vec3 ray_world, ray_origin;
            glm_vec3(ray_world_temp, ray_world);
            glm_vec3_normalize(ray_world);
            float ray_world_length = glm_vec3_norm(ray_world);
            Camera_GetPosition(ray_origin);

            float cube_delete_radius = 2.0f;

            for(int x = 0; x < X_CELLS; x ++){
                for(int y = 0; y < Y_CELLS; y ++){
                    for(int z = 0; z < Z_CELLS; z ++){

                        // Determine if a ray collides with obects in a scene
                        // https://stackoverflow.com/questions/34251763/ray-sphere-intersections-in-opengl
                        
                        vec3 cube_center = {
                            (x - X_CELLS / 2) * 1.0f,
                            (y - Y_CELLS / 2) * 1.0f,
                            (z - Z_CELLS / 2) * 1.0f
                        };

                        vec3 center_minus_origin;
                        glm_vec3_sub(cube_center, ray_origin, center_minus_origin);

                        vec3 cross_dir_and_center_minus_origin;
                        glm_vec3_cross(ray_world, center_minus_origin, cross_dir_and_center_minus_origin);

                        float cross_dir_and_center_minus_origin_norm = glm_vec3_norm(
                            cross_dir_and_center_minus_origin
                        );

                        if(cross_dir_and_center_minus_origin_norm / ray_world_length < cube_delete_radius){
                            main_grid[x][y][z] = 0;
                        }

                        
                    }
                }
            }
            
        }

        switch(state){

            case EDITOR:
                if(KeyClicked(SDLK_SPACE)){
                    state = SIMULATION;
                }
                break;

            case SIMULATION:
                if(KeyClicked(SDLK_SPACE)){
                    state = EDITOR;
                }
                // Update the cells if enough time has passed.
                Uint64 current_time = SDL_GetPerformanceCounter();
                double grid_update_delta_time = GetDeltaTime(grid_last_update_time, current_time);
                if(grid_update_delta_time >= GRID_UPDATE_INTERVAL){
                    grid_last_update_time = current_time;
                    UpdateCells();
                }
                break;
        }        

        // Draw updated objects
        DRAW();
        

        prev_frame_time = curr_frame_time;
    }

    DeleteShaders();
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();    

    return 0;
}

void DRAW(){

    // Clear window
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // Draw elements
    for(int x = 0; x < X_CELLS; x ++){
        for(int y = 0; y < Y_CELLS; y ++){
            for(int z = 0; z < Z_CELLS; z ++){
                if(main_grid[x][y][z]){

                    // Assign colors from xyz coordinate
                    float r = (x * 3 + 130) / 255.0f;
                    float g = (y * 1.2f + 100) / 255.0f;
                    float b = (z * 3.5 + 100) / 255.0f;
                    UpdateColorUniform((vec4){r, g, b, 1.0f});
                    vec3 dst_vec = {
                        (x - X_CELLS / 2) * 1.0f,
                        (y - Y_CELLS / 2) * 1.0f,
                        (z - Z_CELLS / 2) * 1.0f
                    };
                    DrawStaticCube(dst_vec);
                }
            }
        }
    }

    // Swap buffers
    SDL_GL_SwapWindow(window);
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
