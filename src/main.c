#include <stdio.h>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <cglm/cglm.h>

#define WINDOW_W 1920 / 4 * 3
#define WINDOW_H 1080 / 4 * 3

SDL_GLContext ctx = NULL;
SDL_Window* window = NULL;
GLuint shader_program;

int INIT();
const char* GetVertexShader();
const char* GetFragmentShader();

void _print_shader_info_log(GLuint shader_index) {
    int max_length = 2048;
    int actual_length = 0;
    char shader_log[2048];
    glGetShaderInfoLog(shader_index, max_length, &actual_length, shader_log);
    printf("shader info log for GL index %u:\n%s\n", shader_index, shader_log);
}

int main(int argc, char** argv){

    

    if(INIT() == -1){
        return -1;
    }

    SDL_bool running = SDL_TRUE;
    SDL_Event event;

    while(running){
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    running = SDL_FALSE;
                    break;
                case SDL_KEYDOWN:
                    if(event.key.keysym.sym == SDLK_ESCAPE){
                        running = SDL_FALSE;
                    }
                    break;
            }
        }

        glClearColor(1, 0, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        SDL_GL_SwapWindow(window);
    }

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
    glViewport(0, 0, (GLsizei)10, (GLsizei)10);

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


    const char* vertex_shader = GetVertexShader();
    const char* fragment_shader = GetFragmentShader();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, NULL);
    glCompileShader(vs);
    int params = -1;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
    if(GL_TRUE != params){
        printf("ERROR: GL shader index %d did not compile\n", vs);
        _print_shader_info_log(vs);
        return -1;
    }

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragment_shader, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
    if(GL_TRUE != params){
        printf("ERROR: GL shader index %d did not compile\n", fs);
        _print_shader_info_log(fs);
        return -1;
    }

    shader_program = glCreateProgram();
    glAttachShader(shader_program, fs);
    glAttachShader(shader_program, vs);
    glLinkProgram(shader_program);
    glUseProgram(shader_program);

    return 0;
}

const char *GetVertexShader()
{
   static char vertexShader[1024];
   strcpy(vertexShader, 
           "#version 400\n"
           "layout (location = 0) in vec3 vertex_position;\n"
           "layout (location = 1) in vec3 vertex_normal;\n"
           "uniform vec3 cameraloc;  // Camera position \n"
           "uniform vec3 lightdir;   // Lighting direction \n"
           "uniform vec4 lightcoeff; // Lighting coeff, Ka, Kd, Ks, alpha\n"
           "uniform mat4 MVP;\n"
           "out float shading_amount;\n"
           "void main() {\n"
           "  gl_Position = MVP*vec4(vertex_position, 1.0);\n"
           "  shading_amount = lightcoeff[0];\n"

           "  float LdotN = dot(lightdir, vertex_normal);\n"
           "  shading_amount += LdotN < 0 ? 0 : LdotN * lightcoeff[1];\n"

           "  vec3 R = normalize(2 * LdotN * vertex_normal - lightdir);\n"
           "  vec3 V = normalize(cameraloc - vertex_position);\n"
           "  float RdotV = dot(R, V);\n"
           "  float specular = RdotV > 0 ? pow(RdotV, lightcoeff[3]) * lightcoeff[2] : 0.0;\n"
           "  shading_amount += specular;\n"
           "}\n"
         );
   return vertexShader;
}

const char *GetFragmentShader()
{
   static char fragmentShader[1024];
   strcpy(fragmentShader, 
           "#version 400\n"
           "in float shading_amount;\n"
           "uniform vec3 color;\n"
           "out vec4 frag_color;\n"
           "void main() {\n"
           "  frag_color = vec4(color, 1.0);\n"
           "  for(int i = 0; i < 3; i ++){\n"
           "    frag_color[i] = frag_color[i] * shading_amount > 1 ? 1 : frag_color[i] * shading_amount;\n"
           "  }\n"
           "}\n"
         );
   return fragmentShader;
}
