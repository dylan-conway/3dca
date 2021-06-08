"#version 400\n"

"in float shading;\n"

"uniform vec4 color;\n"

"out vec4 frag_color;\n"

"void main() {\n"
    "frag_color = color;\n"
    "for(int i = 0; i < 3; i ++){\n"
        "frag_color[i] = frag_color[i] * shading > 1 ? 1 : frag_color[i] * shading;\n"
    "}\n"
    "if(color == vec4(1.0f, 1.0f, 1.0f, 1.0f)){\n"
        "frag_color = color;\n"
    "}\n"
"}"