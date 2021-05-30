#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cglm/vec3.h>
#include <cglm/mat4.h>
#include <cglm/cam.h>

#include "defines.h"
#include "meshes.h"

struct Camera {
    vec3 position;
    vec3 up;
    vec3 target;
};

struct Camera Camera_Init();
void Camera_SetPosition(vec3 new_position);
void Camera_MovePosition(vec3 v);
void Camera_GetPosition(vec3 out_pos);
void Camera_RotateAroundOrigin(double step);
void Camera_GetViewMatrix(mat4 out_view_matrix);
void Camera_GetProjectionMatrix(mat4 out_proj_matrix);
void Camera_GetMVP(mat4* model, mat4 out_mvp);

#endif