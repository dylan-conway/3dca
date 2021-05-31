#include "camera.h"

static struct Camera _cam;

struct Camera Camera_Init(){
    struct Camera c;
    glm_vec3_copy((vec3){0.0f, 2.0f, 5.0f}, c.position);
    glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, c.up);
    glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, c.target);
    _cam = c;
    return c;
}

void Camera_SetPosition(vec3 new_position){
    glm_vec3_copy(new_position, _cam.position);
}

void Camera_MovePosition(vec3 v){
    vec3 old_pos;
    glm_vec3_copy(_cam.position, old_pos);
    glm_vec3_add(v, old_pos, _cam.position);
}

void Camera_GetPosition(vec3 out_pos){
    glm_vec3_copy(_cam.position, out_pos);
}

void Camera_RotateAroundOrigin(double step){

     // Rotate camera around origin.
    // This helped a ton:
    // https://gamedev.stackexchange.com/questions/31218/how-to-move-an-object-along-a-circumference-of-another-object
    
    vec3 new_cam_pos;
    Camera_GetPosition(new_cam_pos);

    double radius = sqrt(pow(new_cam_pos[0], 2) + pow(new_cam_pos[2], 2));

    double angle = atan2(new_cam_pos[2], new_cam_pos[0]);    
    angle += step;

    new_cam_pos[0] = cos(angle) * radius;
    new_cam_pos[2] = sin(angle) * radius;

    Camera_SetPosition(new_cam_pos);
}

void Camera_RotateAroundOriginY(double step){

    // vec3 new_cam_pos;
    // Camera_GetPosition(new_cam_pos);

    // double radius = sqrt(pow)
}

void Camera_GetViewMatrix(mat4 out_view_matrix){
    glm_lookat(
        _cam.position,
        _cam.target,
        _cam.up,
        out_view_matrix
    );
}

void Camera_GetProjectionMatrix(mat4 out_proj_matrix){
    glm_perspective(
        glm_rad(45),
        (float)WINDOW_W / (float)WINDOW_H,
        0.1f, 1000.0f,
        out_proj_matrix
    );
}

void Camera_GetMVP(mat4* model, mat4 out_mvp){
    mat4 projection_matrix = GLM_MAT4_ZERO_INIT;
    Camera_GetProjectionMatrix(projection_matrix);

    mat4 view_matrix = GLM_MAT4_ZERO_INIT;
    Camera_GetViewMatrix(view_matrix);

    glm_mat4_mulN(
        (mat4 *[]){&projection_matrix, &view_matrix, model},
        3, out_mvp
    );
}

void Camera_UpdateUniform(){
    UpdateCameraUniform(_cam.position);
}