#include "camera.h"

struct Camera InitCamera(){
    struct Camera c;
    c.position[0] = 0.0f;
    c.position[1] = 2.0f;
    c.position[2] = 5.0f;
    c.up[0] = 0.0f;
    c.up[1] = 1.0f;
    c.up[2] = 0.0f;
    c.target[0] = 0.0f;
    c.target[1] = 0.0f;
    c.target[2] = 0.0f;
    return c;
}

void GetMVP(struct Camera* cam, mat4* model, mat4 out_mvp){
    mat4 projection_matrix;
    glm_perspective(
        glm_rad(45),
        (float)WINDOW_W / (float)WINDOW_H,
        0.1f,
        1000.0f,
        projection_matrix
    );

    mat4 view_matrix;
    glm_lookat(
        cam->position,
        cam->target,
        cam->up,
        view_matrix
    );

    glm_mat4_mulN(
        (mat4 *[]){&projection_matrix, &view_matrix, model},
        3, out_mvp
    );
}