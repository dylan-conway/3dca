"#version 400\n"

"layout (location = 0) in vec3 vertex_position;\n"
"layout (location = 1) in vec3 vertex_normal;\n"

"uniform mat4 mvp;\n"
"uniform vec3 camera_pos;\n"
"uniform vec3 light_direction;\n"
"uniform vec4 lighting_coefficients;\n"

"out float shading;\n"

"void main() {\n"
    "gl_Position = mvp * vec4(vertex_position, 1.0);\n"
    "shading = lighting_coefficients[0];\n"
    "float ldotn = dot(light_direction, vertex_normal);\n"
    "shading += ldotn < 0 ? 0 : ldotn * lighting_coefficients[1];\n"

    // "vec3 r = normalize(2 * ldotn * vertex_normal - light_direction);\n"
    // "vec3 v = normalize(camera_pos - vertex_position);\n"
    // "float rdotv = dot(r, v);\n"
    // "float specular = rdotv > 0 ? pow(rdotv, lighting_coefficients[3]) * lighting_coefficients[2] : 0.0;\n"
    // "shading += specular;\n"
"}"