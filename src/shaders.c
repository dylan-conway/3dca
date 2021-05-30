
#include "shaders.h"

static GLuint _program_id, _vshader_id, _fshader_id;
static GLuint _mvp_uniform_location, _camera_uniform_location;
static GLuint _light_direction_uniform_location, _lighting_coefficients_uniform_location;
static GLuint _color_uniform_location;

void _print_shader_info_log(GLuint shader_index) {
    int max_length = 2048;
    int actual_length = 0;
    char shader_log[2048];
    glGetShaderInfoLog(shader_index, max_length, &actual_length, shader_log);
    printf("shader info log for GL index %u:\n%s\n", shader_index, shader_log);
}

const char *GetVertexShader()
{
   static char vertex_shader[1024];
   strcpy(vertex_shader, 
           "#version 400\n"
           "layout (location = 0) in vec3 vertex_position;\n"
           "layout (location = 1) in vec3 vertex_normal;\n"
           "uniform mat4 mvp;\n"
           "uniform vec3 camera_pos;\n"
           "uniform vec3 light_direction;\n"
           "uniform vec4 lighting_coefficients;\n"
           "out float shading;\n"
           "void main() {\n"
           "  gl_Position = mvp * vec4(vertex_position, 1.0);\n"
           "  shading = lighting_coefficients[0];\n"
           "  float ldotn = dot(light_direction, vertex_normal);\n"
           "  shading += ldotn < 0 ? 0 : ldotn * lighting_coefficients[1];\n"

        //    "  vec3 r = normalize(2 * ldotn * vertex_normal - light_direction);\n"
        //    "  vec3 v = normalize(camera_pos - vertex_position);\n"
        //    "  float rdotv = dot(r, v);\n"
        //    "  float specular = rdotv > 0 ? pow(rdotv, lighting_coefficients[3]) * lighting_coefficients[2] : 0.0;\n"
        //    "  shading += specular;\n"
           "}\n"
         );
   return vertex_shader;
}

const char *GetFragmentShader()
{
   static char fragment_shader[1024];
   strcpy(fragment_shader, 
           "#version 400\n"
           "in float shading;\n"
           "uniform vec4 color;\n"
           "out vec4 frag_color;\n"
           "void main() {\n"
           "  frag_color = color;\n"
           "  for(int i = 0; i < 3; i ++){\n"
           "    frag_color[i] = frag_color[i] * shading > 1 ? 1 : frag_color[i] * shading;\n"
           "  }\n"
           "}\n"
         );
   return fragment_shader;
}

int CreateShaders(){

    const char* vertex_shader = GetVertexShader();
    const char* fragment_shader = GetFragmentShader();

    int params = -1;

    _vshader_id = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(_vshader_id, 1, &vertex_shader, NULL);
    glCompileShader(_vshader_id);
    glGetShaderiv(_vshader_id, GL_COMPILE_STATUS, &params);
    if(GL_TRUE != params){
        printf("ERROR: GL shader index %d did not compile\n", _vshader_id);
        _print_shader_info_log(_vshader_id);
        return -1;
    }

    _fshader_id = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(_fshader_id, 1, &fragment_shader, NULL);
    glCompileShader(_fshader_id);
    glGetShaderiv(_fshader_id, GL_COMPILE_STATUS, &params);
    if(GL_TRUE != params){
        printf("ERROR: GL shader index %d did not compile\n", _fshader_id);
        _print_shader_info_log(_fshader_id);
        return -1;
    }

    _program_id = glCreateProgram();
    glAttachShader(_program_id, _fshader_id);
    glAttachShader(_program_id, _vshader_id);
    glLinkProgram(_program_id);
    glUseProgram(_program_id);

    _mvp_uniform_location = glGetUniformLocation(_program_id, "mvp");
    _camera_uniform_location = glGetUniformLocation(_program_id, "camera_pos");
    _light_direction_uniform_location = glGetUniformLocation(_program_id, "light_direction");
    _lighting_coefficients_uniform_location = glGetUniformLocation(_program_id, "lighting_coefficients");
    _color_uniform_location = glGetUniformLocation(_program_id, "color");

    vec4 lighting_coefficients;
    lighting_coefficients[0] = 0.3f;
    lighting_coefficients[1] = 0.7f;
    lighting_coefficients[2] = 2.8f;
    lighting_coefficients[3] = 50.5f;
    
    glUniform4fv(_lighting_coefficients_uniform_location, 1, lighting_coefficients);

    return 0;
}

void DeleteShaders(){
    glDetachShader(_program_id, _vshader_id);
    glDeleteShader(_vshader_id);
    glDetachShader(_program_id, _fshader_id);
    glDeleteShader(_fshader_id);
    glDeleteProgram(_program_id);
}

void UpdateMVPUniform(mat4 mvp){
    glUniformMatrix4fv(_mvp_uniform_location, 1, GL_FALSE, mvp[0]);
}

void UpdateCameraUniform(vec3 camera_position){
    glUniform3fv(_camera_uniform_location, 1, camera_position);
}

void UpdateLightDirectionUniform(vec3 light_direction){
    glUniform3fv(_light_direction_uniform_location, 1, light_direction);
}

void UpdateColorUniform(vec4 color){
    glUniform4fv(_color_uniform_location, 1, color);
}