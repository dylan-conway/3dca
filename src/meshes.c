#include "meshes.h"

static GLuint _cube_vao;

void InitCubeVAO(){

    GLfloat vertices[] = {

        -1.0f,  -1.0f,  -1.0f,  // left
        -1.0f,  -1.0f,   1.0f,
        -1.0f,   1.0f,   1.0f,

         1.0f,   1.0f,  -1.0f,  // back
        -1.0f,  -1.0f,  -1.0f,
        -1.0f,   1.0f,  -1.0f,

         1.0f,  -1.0f,   1.0f,  // botton
        -1.0f,  -1.0f,  -1.0f,
         1.0f,  -1.0f,  -1.0f,

         1.0f,   1.0f,  -1.0f,  // back
         1.0f,  -1.0f,  -1.0f,
        -1.0f,  -1.0f,  -1.0f,

        -1.0f,  -1.0f,  -1.0f,  // left
        -1.0f,   1.0f,   1.0f,
        -1.0f,   1.0f,  -1.0f,

         1.0f,  -1.0f,   1.0f,  // bottom
        -1.0f,  -1.0f,   1.0f,
        -1.0f,  -1.0f,  -1.0f,

        -1.0f,   1.0f,   1.0f,  // front
        -1.0f,  -1.0f,   1.0f,
         1.0f,  -1.0f,   1.0f,

         1.0f,   1.0f,   1.0f,  // right
         1.0f,  -1.0f,  -1.0f,
         1.0f,   1.0f,  -1.0f,

         1.0f,  -1.0f,  -1.0f,  // right
         1.0f,   1.0f,   1.0f,
         1.0f,  -1.0f,   1.0f,

         1.0f,   1.0f,   1.0f,  // top
         1.0f,   1.0f,  -1.0f,
        -1.0f,   1.0f,  -1.0f,

         1.0f,   1.0f,   1.0f,  // top
        -1.0f,   1.0f,  -1.0f,
        -1.0f,   1.0f,   1.0f,

         1.0f,   1.0f,   1.0f,  // front
        -1.0f,   1.0f,   1.0f,
         1.0f,  -1.0f,   1.0f
    };

    GLfloat normals[] = {

         1.0f,   0.0f,   0.0f,  // left
         1.0f,   0.0f,   0.0f,
         1.0f,   0.0f,   0.0f,

         0.0f,   0.0f,   1.0f,  // back
         0.0f,   0.0f,   1.0f,
         0.0f,   0.0f,   1.0f,

         0.0f,   1.0f,   0.0f,  // bottom
         0.0f,   1.0f,   0.0f,
         0.0f,   1.0f,   0.0f,

         0.0f,   0.0f,   1.0f,  // back
         0.0f,   0.0f,   1.0f,
         0.0f,   0.0f,   1.0f,

         1.0f,   0.0f,   0.0f,  // left
         1.0f,   0.0f,   0.0f,
         1.0f,   0.0f,   0.0f,

         0.0f,   1.0f,   0.0f,  // bottom
         0.0f,   1.0f,   0.0f,
         0.0f,   1.0f,   0.0f,

         0.0f,   0.0f,  -1.0f,  // front
         0.0f,   0.0f,  -1.0f,
         0.0f,   0.0f,  -1.0f,

        -1.0f,   0.0f,   0.0f,  // right
        -1.0f,   0.0f,   0.0f,
        -1.0f,   0.0f,   0.0f,

        -1.0f,   0.0f,   0.0f,  // right
        -1.0f,   0.0f,   0.0f,
        -1.0f,   0.0f,   0.0f,

         0.0f,  -1.0f,   0.0f,  // top
         0.0f,  -1.0f,   0.0f,
         0.0f,  -1.0f,   0.0f,

         0.0f,  -1.0f,   0.0f,  // top
         0.0f,  -1.0f,   0.0f,
         0.0f,  -1.0f,   0.0f,

         0.0f,   0.0f,  -1.0f,  // front
         0.0f,   0.0f,  -1.0f,
         0.0f,   0.0f,  -1.0f,
    };

    glGenVertexArrays(1, &_cube_vao);
    glBindVertexArray(_cube_vao);

    GLuint vertices_vbo;
    glGenBuffers(1, &vertices_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertices_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL);

    GLuint normals_vbo;
    glGenBuffers(1, &normals_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
}

struct Cube InitCube(vec3 position){
    struct Cube c = {
        .position = GLM_VEC3_ZERO_INIT,
        .scale = GLM_VEC3_ONE_INIT
    };
    glm_vec3_copy(position, c.position);
    glm_vec3_copy((vec3){0.5f, 0.5f, 0.5f}, c.scale);
    return c;
}

void SetPositionCube(struct Cube* cube, vec3 new_position){
    glm_vec3_copy(new_position, cube->position);
}

void SetScaleCube(struct Cube* cube, vec3 new_scale){
    glm_vec3_copy(new_scale, cube->scale);
}

void DrawCube(struct Cube* cube){

    mat4 mvp = GLM_MAT4_IDENTITY_INIT;
    mat4 mt = GLM_MAT4_IDENTITY_INIT;
    mat4 ms = GLM_MAT4_IDENTITY_INIT;
    mat4 model = GLM_MAT4_IDENTITY_INIT;

    glm_translate(mt, cube->position);
    glm_scale(ms, cube->scale);

    glm_mat4_mul(mt, ms, model);

    Camera_GetMVP(&model, mvp);
    UpdateMVPUniform(mvp);

    glBindVertexArray(_cube_vao);
    glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // Two triangles per side
}

void DrawStaticCube(vec3 position, vec3 scale){

    mat4 mvp = GLM_MAT4_IDENTITY_INIT;
    mat4 mt = GLM_MAT4_IDENTITY_INIT;
    mat4 ms = GLM_MAT4_IDENTITY_INIT;
    mat4 model = GLM_MAT4_IDENTITY_INIT;

    glm_translate(mt, position);
    glm_scale(ms, scale);

    glm_mat4_mul(mt, ms, model);

    Camera_GetMVP(&model, mvp);
    UpdateMVPUniform(mvp);

    glBindVertexArray(_cube_vao);
    glDrawArrays(GL_TRIANGLES, 0, 12 * 3);
}