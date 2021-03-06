#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#define DEBUG
#define CGLM_DEFINE_PRINTS
#include <cglm/mat4.h>
#include <cglm/vec3.h>
#include <cglm/vec2.h>
#include <cglm/affine.h>
#include <cglm/version.h>
#include <cglm/quat.h>
#include <cglm/io.h>

#include "shaders.h"
#include "meshes.h"
#include "camera.h"
#include "defines.h"
#include "input.h"

#define MAX_X_CELLS 80
#define MAX_Y_CELLS 80
#define MAX_Z_CELLS 80

#define MIN_X_CELLS 10
#define MIN_Y_CELLS 10
#define MIN_Z_CELLS 10

int X_CELLS = 40;
int Y_CELLS = 40;
int Z_CELLS = 40;

enum STATE {
    EDITOR,
    SIMULATION
};

SDL_GLContext ctx = NULL;
SDL_Window* window = NULL;
SDL_bool running = SDL_TRUE;
enum STATE state = EDITOR;

Uint64 wireframe_cube_timer;

int main_grid[MAX_X_CELLS][MAX_Y_CELLS][MAX_Z_CELLS];
double grid_update_interval = 0.55f;

int update_counter = 1;

int num_color_schemes = 4;
enum COLOR_SCHEME {
    RGB_CUBE = 0,
    STATE_SHADING,
    GRAY_CUBES,
    RGB_STATE_CUBE
};

enum COLOR_SCHEME current_color_scheme = RGB_CUBE;

struct CA_rules {
    char* name;

    int num_survive_bounds;
    int* survive_bounds;

    int num_born_bounds;
    int* born_bounds;

    int num_states;
    char neighborhood;
    int edge_wrap;
};

#define NUM_RULES 5
int current_rule = 0;

struct CA_rules rules[NUM_RULES] = {
    {
        "Amoeba-1", 1, NULL, 3, NULL, 16, 'M', 1
    },
    {
        "test1", 2, NULL, 2, NULL, 4, 'M', 0
    },
    {
        "Triangle", 1, NULL, 1, NULL, 5, 'M', 0
    },
    {
        "Pyroclastic", 1, NULL, 1, NULL, 10, 'M', 0
    },
    {
        "Clouds 1", 1, NULL, 2, NULL, 2, 'M', 1
    }
};

int survive_bounds_counter = 0;
int survive_bounds[] = {
    9, 26,
    10, 11, 13, 26,
    2, 3,
    4, 7,
    13, 26
};

int born_bounds_counter = 0;
int born_bounds[] = {
    5, 7, 12, 13, 15, 15,
    14, 15, 17, 19,
    4, 10,
    6, 8,
    13, 14, 17, 19
};

int INIT();
void DRAW();
void FREE();

void RaycastAtMouse(vec3 out_ray);
SDL_bool RaycastHit(vec3 ray, vec3 ray_origin, vec3 object_center, float radius);

double GetDeltaTime(Uint64 start_time, Uint64 end_time);


SDL_bool CheckCell(int x, int y, int z);
void CheckCellStates();
int GetCell(int x, int y, int z);
void IncrementCell(int x, int y, int z);
void ClearGrid();
void FillGrid();
void RandomizeGrid();
int CountMooreNeighbors(int x, int y, int z);
int CountNeumannNeighbors(int x, int y, int z);
void UpdateCells();
void SwitchRule(int new_rule_index);


int main(int argc, char** argv){

    srand(5546);

    // Init SDL2, OpenGL, GLEW, and create shaders
    if(INIT() == -1){
        return -1;
    }

    InitInput();
    InitCubeVAO();
    Camera_Init();
    Camera_SetPosition((vec3){0.0f, 25.0f * 2.3f, 70.0f * 2.3f});

    for(int i = 0; i < NUM_RULES; i ++){
        rules[i].survive_bounds = malloc(sizeof(int) * rules[i].num_survive_bounds * 2);
        rules[i].born_bounds = malloc(sizeof(int) * rules[i].num_born_bounds * 2);
        for(int j = 0; j < rules[i].num_survive_bounds * 2; j += 2){
            rules[i].survive_bounds[j] = survive_bounds[survive_bounds_counter];
            survive_bounds_counter ++;
            rules[i].survive_bounds[j + 1] = survive_bounds[survive_bounds_counter];
            survive_bounds_counter ++;
        }
        for(int j = 0; j < rules[i].num_born_bounds * 2; j += 2){
            rules[i].born_bounds[j] = born_bounds[born_bounds_counter];
            born_bounds_counter ++;
            rules[i].born_bounds[j + 1] = born_bounds[born_bounds_counter];
            born_bounds_counter ++;
        }
    }

    printf("\nRULES\n");
    for(int i = 0; i < NUM_RULES; i ++){
        printf("%s: ", rules[i].name);
        for(int j = 0; j < rules[i].num_survive_bounds * 2; j += 2){
            if(rules[i].survive_bounds[j] == rules[i].survive_bounds[j + 1]){
                printf("%d", rules[i].survive_bounds[j]);
            } else {
                printf("%d-%d", rules[i].survive_bounds[j], rules[i].survive_bounds[j + 1]);
            }
            if(j != rules[i].num_survive_bounds * 2 - 2){
                printf(",");
            }
        }
        printf("/");
        for(int j = 0; j < rules[i].num_born_bounds * 2; j += 2){
            if(rules[i].born_bounds[j] == rules[i].born_bounds[j + 1]){
                printf("%d", rules[i].born_bounds[j]);
            } else {
                printf("%d-%d", rules[i].born_bounds[j], rules[i].born_bounds[j + 1]);
            }
            if(j != rules[i].num_born_bounds * 2 - 2){
                printf(",");
            }
        }
        printf("/%d/%c\n", rules[i].num_states, rules[i].neighborhood);
    }
    printf("\n");

    // glm_rotate

    FillGrid();
    SwitchRule(current_rule);
    wireframe_cube_timer = SDL_GetPerformanceCounter();

    Uint64 grid_last_update_time = SDL_GetPerformanceCounter();
    SDL_Event event;
    Uint64 curr_frame_time, prev_frame_time;
    prev_frame_time = SDL_GetPerformanceCounter();
    while(running){

        curr_frame_time = SDL_GetPerformanceCounter();
        double delta = GetDeltaTime(prev_frame_time, curr_frame_time);

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

        if(KeyClicked(SDLK_c)){
            ClearGrid(main_grid);
        }

        if(KeyClicked(SDLK_r)){
            RandomizeGrid();
        }

        if(KeyClicked(SDLK_f)){
            FillGrid();
        }

        if(KeyClicked(SDLK_s)){
            int next_rule = current_rule + 1;
            if(next_rule >= NUM_RULES){
                next_rule = 0;
            }
            SwitchRule(next_rule);
        }

        if(KeyClicked(SDLK_a)){
            int prev_rule = current_rule - 1;
            if(prev_rule < 0){
                prev_rule = NUM_RULES - 1;
            }
            SwitchRule(prev_rule);
        }

        if(KeyClicked(SDLK_x)){
            // Increment color scheme
            current_color_scheme = (current_color_scheme + 1) % num_color_schemes;
        }

        if(KeyClicked(SDLK_z)){
            // Decrement color scheme
            current_color_scheme = (current_color_scheme + num_color_schemes - 1) % num_color_schemes;
        }

        if(KeyClicked(SDLK_DOWN)){
            grid_update_interval += 0.1f;
            if(grid_update_interval > 0.55f){
                grid_update_interval = 0.55f;
            }
        }

        if(KeyClicked(SDLK_UP)){
            grid_update_interval -= 0.1f;
            if(grid_update_interval < 0.05f){
                grid_update_interval = 0.05f;
            }
        }

        if(KeyClicked(SDLK_1)){
            ClearGrid();
            main_grid[X_CELLS / 2][Y_CELLS / 2][Z_CELLS / 2] = 1;
        }

        if(KeyClicked(SDLK_2)){
            ClearGrid();
            for(int x = -2; x < 2; x ++){
                for(int y = -2; y < 2; y ++){
                    for(int z = -2; z < 2; z ++){
                        main_grid[X_CELLS / 2 + x][Y_CELLS / 2 + y][Z_CELLS / 2 + z] = 1;
                    }
                }
            }
        }

        if(KeyClicked(SDLK_3)){
            ClearGrid();
            for(int x = -4; x < 4; x ++){
                for(int y = -4; y < 4; y ++){
                    for(int z = -4; z < 4; z ++){
                        main_grid[X_CELLS / 2 + x][Y_CELLS / 2 + y][Z_CELLS / 2 + z] = 1;
                    }
                }
            }
        }

        if(KeyClicked(SDLK_4)){
            ClearGrid();
            for(int x = -4; x < 4; x ++){
                for(int y = -4; y < 4; y ++){
                    for(int z = -4; z < 4; z ++){
                        main_grid[X_CELLS / 2 + x][Y_CELLS / 2 + y][Z_CELLS / 2 + z] = rand() % 2;
                    }
                }
            }
        }


        Sint32 wheel_move;
        if(Mouse_WheelMoved(&wheel_move)){

            if(wheel_move < 0){
                X_CELLS -= 2;
                Y_CELLS -= 2;
                Z_CELLS -= 2;
            } else if(wheel_move > 0){
                X_CELLS += 2;
                Y_CELLS += 2;
                Z_CELLS += 2;
            }

            if(X_CELLS < MIN_X_CELLS) X_CELLS = MIN_X_CELLS;
            if(X_CELLS > MAX_X_CELLS) X_CELLS = MAX_X_CELLS;
            if(Y_CELLS < MIN_Y_CELLS) Y_CELLS = MIN_Y_CELLS;
            if(Y_CELLS > MAX_Y_CELLS) Y_CELLS = MAX_Y_CELLS;
            if(Z_CELLS < MIN_Z_CELLS) Z_CELLS = MIN_Z_CELLS;
            if(Z_CELLS > MAX_Z_CELLS) Z_CELLS = MAX_Z_CELLS;

            wireframe_cube_timer = SDL_GetPerformanceCounter();

            // Make sure cell states do not go past current rule
            CheckCellStates();
        }

        if(ButtonDown(SDL_BUTTON_MIDDLE)){

            vec2 curr_mouse_pos, prev_mouse_pos;
            Mouse_GetCurrPos(curr_mouse_pos);
            Mouse_GetPrevPos(prev_mouse_pos);

            float mouse_diff_x = curr_mouse_pos[0] - prev_mouse_pos[0];
            if(mouse_diff_x != 0){
                double rotation_step = mouse_diff_x / 3 * 0.01f;
                Camera_RotateAroundOrigin(rotation_step);
            }
        }

        if(KeyDown(SDLK_RIGHT)){
            double rotation_step = 0.8f * delta;
            Camera_RotateAroundOrigin(rotation_step);
        }

        if(KeyDown(SDLK_LEFT)){
            double rotation_step = -0.8f * delta;
            Camera_RotateAroundOrigin(rotation_step);
        }

        // Delete cubes
        if(ButtonDown(SDL_BUTTON_RIGHT)){

            // Get the ray from mouse
            vec3 ray, ray_origin;
            RaycastAtMouse(ray);
            Camera_GetPosition(ray_origin);

            float cube_delete_radius = 2.5f;

            for(int x = 0; x < X_CELLS; x ++){
                for(int y = 0; y < Y_CELLS; y ++){
                    for(int z = 0; z < Z_CELLS; z ++){

                        vec3 cube_center = {
                            (x - X_CELLS / 2) * 1.0f,
                            (y - Y_CELLS / 2) * 1.0f,
                            (z - Z_CELLS / 2) * 1.0f
                        };
                        
                        // If the ray intersects with any cube (determined with a bounding sphere
                        // around the cube), then delete the cube
                        if(RaycastHit(ray, ray_origin, cube_center, cube_delete_radius)){
                            main_grid[x][y][z] = 0;
                        }
                    }
                }
            }
        }

        // Add cells
        if(ButtonDown(SDL_BUTTON_LEFT)){

            // Same as deletion, raycast from the mouse and check for collisions

            vec3 ray, ray_origin;
            RaycastAtMouse(ray);
            Camera_GetPosition(ray_origin);

            float cube_creation_radius = 1.1f;

            for(int x = 0; x < X_CELLS; x ++){
                for(int y = 0; y < Y_CELLS; y ++){
                    for(int z = 0; z < Z_CELLS; z ++){

                        vec3 cube_center = {
                            (x - X_CELLS / 2) * 1.0f,
                            (y - Y_CELLS / 2) * 1.0f,
                            (z - Z_CELLS / 2) * 1.0f
                        };

                        if(RaycastHit(ray, ray_origin, cube_center, cube_creation_radius)){
                            main_grid[x][y][z] = 1;
                        }
                    }
                }
            }
        }

        // Update uniforms with new camera position
        Camera_UpdateUniform();
        vec3 cam_position;
        Camera_GetPosition(cam_position);

        vec3 light_direction;
        glm_vec3_sub((vec3){0.0f, 0.0f, 0.0f}, cam_position, light_direction);
        glm_vec3_normalize(light_direction);
        UpdateLightDirectionUniform(light_direction);

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
                if(grid_update_delta_time >= grid_update_interval){
                    grid_last_update_time = current_time;
                    UpdateCells();
                }
                break;
        }        

        // Draw updated objects
        DRAW();
        

        prev_frame_time = curr_frame_time;
    }

    FREE();    

    return 0;
}

SDL_bool RaycastHit(vec3 ray, vec3 ray_origin, vec3 object_center, float radius){

    // Determine if a ray collides with obects in a scene
    // https://stackoverflow.com/questions/34251763/ray-sphere-intersections-in-opengl
    // Using equation: length(cross(direction, center - origin)) / length(direction) < radius

    SDL_bool hit = SDL_FALSE;

    vec3 center_minus_origin;
    glm_vec3_sub(object_center, ray_origin, center_minus_origin);

    vec3 cross_dir_and_center_minus_origin;
    glm_vec3_cross(ray, center_minus_origin, cross_dir_and_center_minus_origin);

    float cross_dir_and_center_minus_origin_norm = glm_vec3_norm(
        cross_dir_and_center_minus_origin
    );

    float ray_norm = glm_vec3_norm(ray);
    if(cross_dir_and_center_minus_origin_norm / ray_norm < radius){
        hit = SDL_TRUE;
    }

    return hit;
}

void RaycastAtMouse(vec3 out_ray){

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

    vec4 ray_world;
    mat4 view_matrix, inv_view_matrix;
    Camera_GetViewMatrix(view_matrix);
    glm_mat4_inv(view_matrix, inv_view_matrix);
    glm_mat4_mulv(inv_view_matrix, ray_eye, ray_world);

    glm_vec3(ray_world, out_ray);
    glm_normalize(out_ray);
}

void DRAW(){

    // Clear window
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Draw elements
    for(int x = 0; x < X_CELLS; x ++){
        for(int y = 0; y < Y_CELLS; y ++){
            for(int z = 0; z < Z_CELLS; z ++){
                if(main_grid[x][y][z]){

                    switch(current_color_scheme){
                        case RGB_CUBE:{
                            // Assign colors from xyz coordinate (RGB cube)
                            float r = (x * 3.0f + (150 - X_CELLS)) / 255.0f;
                            float g = (y * 1.2f + (170 - Y_CELLS)) / 255.0f;
                            float b = (z * 3.5f + (150 - Z_CELLS)) / 255.0f;
                            UpdateColorUniform((vec4){r, g, b, 1.0f});
                            break;
                        }
                        case STATE_SHADING:{
                            if(main_grid[x][y][z] == 1){
                                UpdateColorUniform(ORANGE);
                            } else {
                                UpdateColorUniform(RED);
                            }
                            break;
                        }
                        case GRAY_CUBES:{
                            UpdateColorUniform(GRAY);
                            break;
                        }
                        case RGB_STATE_CUBE:{
                            if(main_grid[x][y][z] == 1){
                                // Assign colors from xyz coordinate (RGB cube)
                                float r = (x * 3.0f + (150 - X_CELLS)) / 255.0f;
                                float g = (y * 1.2f + (170 - Y_CELLS)) / 255.0f;
                                float b = (z * 3.5f + (150 - Z_CELLS)) / 255.0f;
                                UpdateColorUniform((vec4){r, g, b, 1.0f});
                            } else {
                                UpdateColorUniform(RED);
                            }
                            
                            break;
                        }

                    }
                    
                    vec3 dst_vec = {
                        ((float)x - X_CELLS / 2.0f) * 1.0f,
                        ((float)y - Y_CELLS / 2.0f) * 1.0f,
                        ((float)z - Z_CELLS / 2.0f) * 1.0f
                    };
                    DrawStaticCube(dst_vec, (vec3){0.5f, 0.5f, 0.5f});
                }
            }
        }
    }

    // Draw big cube
    if(GetDeltaTime(wireframe_cube_timer, SDL_GetPerformanceCounter()) < 1.0f){
        UpdateColorUniform((vec4){1.0f, 1.0f, 1.0f, 1.0f});
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        vec3 dst_vec = {
            -0.51f, -0.51f, -0.51f
        };
        vec3 scale_vec = {
            X_CELLS / 2.0f * 1.001f,
            Y_CELLS / 2.0f * 1.001f,
            Z_CELLS / 2.0f * 1.001f
        };
        DrawStaticCube(dst_vec, scale_vec);
    }



    // Swap buffers
    SDL_GL_SwapWindow(window);
}

double GetDeltaTime(Uint64 start_time, Uint64 end_time){
    return (double)(end_time - start_time) / SDL_GetPerformanceFrequency();
}

int CountMooreNeighbors(int x, int y, int z){
    int count = 0;
    for(int i = -1; i < 2; i ++){
        for(int j = -1; j < 2; j ++){
            for(int k = -1; k < 2; k ++){
                if(i == 0 && j == 0 && k == 0) continue;
                int new_x = x + i;
                int new_y = y + j;
                int new_z = z + k;
                if(rules[current_rule].edge_wrap){
                    // Clouds 1 looks better when edges wrap
                    if(new_x == -1) new_x = X_CELLS - 1;
                    if(new_x == X_CELLS) new_x = 1;
                    if(new_y == -1) new_y = Y_CELLS - 1;
                    if(new_y == Y_CELLS) new_y = 1;
                    if(new_z == -1) new_z = Z_CELLS - 1;
                    if(new_z == Z_CELLS) new_z = 1;
                    if(GetCell(new_x, new_y, new_z) == 1){
                        count ++;
                    }
                } else {
                    if(CheckCell(new_x, new_y, new_z)){
                       if(GetCell(new_x, new_y, new_z) == 1){
                            count ++;
                        }
                    }
                }
            }
        }
    }
    return count;
}

// int CountNeumannNeighbors(int x, int y, int z){
//     int count = 0;
//     for(int i = -1; i < 2; i ++){
//         for(int j = -1; j < 2; j ++){
//             for(int k = -1; k < 2; k ++){
//                 if(i == 0 && j == 0 && k == 0) continue;
//                 SDL_bool n = SDL_FALSE;
//                 if(i == 0 && j == 0) n = SDL_TRUE;
//                 if(i == 0 && k == 0) n = SDL_TRUE;
//                 if(j == 0 && k == 0) n = SDL_TRUE;
//                 if(n){
//                     int new_x = x + i;
//                     int new_y = y + j;
//                     int new_z = z + k;
//                     if(new_x == -1) new_x = X_CELLS - 1;
//                     if(new_x == X_CELLS) new_x = 1;
//                     if(new_y == -1) new_y = Y_CELLS - 1;
//                     if(new_y == Y_CELLS) new_y = 1;
//                     if(new_z == -1) new_z = Z_CELLS - 1;
//                     if(new_z == Z_CELLS) new_z = 1;
//                     if(GetCell(new_x, new_y, new_z) == 1){
//                         count ++;
//                     }
//                 }
//             }
//         }
//     }
//     return count;
// }

SDL_bool CheckCell(int x, int y, int z){
    if(x < 0 || x >= X_CELLS){
        return SDL_FALSE;
    }
    if(y < 0 || y >= Y_CELLS){
        return SDL_FALSE;
    }
    if(z < 0 || z >= Z_CELLS){
        return SDL_FALSE;
    }
    return main_grid[x][y][z] > 0;
}

// Check cell first to ensure in bounds
int GetCell(int x, int y, int z){
    return main_grid[x][y][z];
}

void IncrementCell(int x, int y, int z){
    main_grid[x][y][z] ++;
    if(main_grid[x][y][z] == rules[current_rule].num_states){
        main_grid[x][y][z] = 0;
    }
}

void ClearGrid(){
    for(int x = 0; x < X_CELLS; x ++){
        for(int y = 0; y < Y_CELLS; y ++){
            for(int z = 0; z < Z_CELLS; z ++){
                main_grid[x][y][z] = 0;
            }
        }
    }
}

void FillGrid(){
    for(int x = 0; x < X_CELLS; x ++){
        for(int y = 0; y < Y_CELLS; y ++){
            for(int z = 0; z < Z_CELLS; z ++){
                main_grid[x][y][z] = 1;
            }
        }
    }
}

void RandomizeGrid(){
    for(int x = 0; x < X_CELLS; x ++){
        for(int y = 0; y < Y_CELLS; y ++){
            for(int z = 0; z < Z_CELLS; z ++){
                main_grid[x][y][z] = rand() % 2;
            }
        }
    }
}

void UpdateCells(){

    int num_survive_bounds = rules[current_rule].num_survive_bounds;
    int num_born_bounds = rules[current_rule].num_born_bounds;
    char nbh = rules[current_rule].neighborhood;

    // Count neighbors.
    int num_neighbors[X_CELLS][Y_CELLS][Z_CELLS];
    for(int x = 0; x < X_CELLS; x ++){
        for(int y = 0; y < Y_CELLS; y ++){
            for(int z = 0; z < Z_CELLS; z ++){
                if(nbh == 'M'){
                    num_neighbors[x][y][z] = CountMooreNeighbors(x, y, z);
                }
                // } else if(nbh == 'N'){
                //     num_neighbors[x][y][z] = CountNeumannNeighbors(x, y, z);
                // }
            }
        }
    }

    // Apply CA rules to grid
    for(int x = 0; x < X_CELLS; x ++){
        for(int y = 0; y < Y_CELLS; y ++){
            for(int z = 0; z < Z_CELLS; z ++){

                int nn = num_neighbors[x][y][z];
                if(nn < 0 || nn > 26){
                    printf("PROBLEM\n");
                }
                int cell = GetCell(x, y, z);

                switch(cell){
                    case 0:{
                        for(int i = 0; i < num_born_bounds * 2; i += 2){
                            int lb = rules[current_rule].born_bounds[i];
                            int up = rules[current_rule].born_bounds[i + 1];
                            if(nn >= lb && nn <= up){
                                IncrementCell(x, y, z);
                                break;
                            }
                        }
                        break;
                    }
                    case 1:{
                        SDL_bool survives = SDL_FALSE;
                        for(int i = 0; i < num_survive_bounds * 2; i += 2){
                            int lb = rules[current_rule].survive_bounds[i];
                            int up = rules[current_rule].survive_bounds[i + 1];
                            if(nn >= lb && nn <= up){
                                survives = SDL_TRUE;
                                break;
                            }
                        }
                        if(!survives){
                            IncrementCell(x, y, z);
                        }
                        break;
                    }
                    default:
                        IncrementCell(x, y, z);
                        break;
                }
            }
        }
    }

    update_counter ++;
}

void SwitchRule(int next_rule_index){
    current_rule = next_rule_index;
    SDL_SetWindowTitle(window, rules[current_rule].name);
    CheckCellStates();
}

void CheckCellStates(){
    for(int x = 0; x < X_CELLS; x ++){
        for(int y = 0; y < Y_CELLS; y ++){
            for(int z = 0; z < Z_CELLS; z ++){
                if(main_grid[x][y][z] >= rules[current_rule].num_states){
                    main_grid[x][y][z] = rules[current_rule].num_states - 1;
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
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    
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
    glEnable(GL_MULTISAMPLE);
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

void FREE(){

    DeleteShaders();
    SDL_GL_DeleteContext(ctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
