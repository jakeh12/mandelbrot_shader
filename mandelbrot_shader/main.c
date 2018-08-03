#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl3.h>
#include <GLUT/glut.h>

#define SIZE 600
#define MAXITER_INIT 256

double center_x, center_y, zoom;
int maxiter, color;
GLuint program;

void display()
{
    
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays (GL_TRIANGLES, 0, 6);
    glutSwapBuffers();
}


void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        center_x = center_x + ((((double)x-SIZE/2.0)/SIZE)*4.0)*zoom;
        center_y = center_y - ((((double)y-SIZE/2.0)/SIZE)*4.0)*zoom;
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
    {
        zoom = zoom / 2.0;
    }
    glUniform2d(glGetUniformLocation(program, "center"), center_x, center_y);
    glUniform1d(glGetUniformLocation(program, "zoom"), zoom);
    
    glutPostRedisplay();
    printf("ZOOM:%lldX  X:%.10f  Y:%.10f  MAX_ITER:%d\n", (long long)(1.0/zoom), center_x, center_y, maxiter);
}


void keyboard(unsigned char key, int x, int y)
{
    switch(key)
    {
        case 27:
            exit(0);
            break;
        case 'r':
            zoom = 1.0;
            center_x = 0.0;
            center_y = 0.0;
            break;
        case 'z':
            if (zoom > 0.0)
            {
                zoom = zoom/2.0;
            }
            break;
        case 'x':
            zoom = zoom*2.0;
            break;
        case 'i':
            maxiter = maxiter * 2;
            break;
        case 'o':
            if (maxiter > 1)
            {
                maxiter = maxiter / 2;
            }
            break;
        case 'c':
            if (color == 0)
            {
                color = 1;
            }
            else
            {
                color = 0;
            }
            break;
        default:
            break;
    }

    glUniform2d(glGetUniformLocation(program, "center"), center_x, center_y);
    glUniform1d(glGetUniformLocation(program, "zoom"), zoom);
    glUniform1i(glGetUniformLocation(program, "maxiter"), maxiter);
    
    glutPostRedisplay();
    printf("ZOOM:%lldX  X:%.10f  Y:%.10f  MAX_ITER:%d\n", (long long)(1.0/zoom), center_x, center_y, maxiter);

}


int main(int argc, char** argv)
{
    
    center_x = 0.0f;
    center_y = 0.0f;
    zoom = 1.0f;
    maxiter = 256;
    color = 0;
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(SIZE, SIZE);
    glutCreateWindow("GPU Mandelbrot");
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);

    const GLchar* vertex_shader_source =
    "#version 410\n"
    "in vec3 vp;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = vec4(vp, 1.0);\n"
    "}";
    
    const GLchar* fragment_shader_source =
    "#version 410 core\n"
    "uniform dvec2 center;\n"
    "uniform double zoom;\n"
    "uniform double size;\n"
    "uniform int maxiter;\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "    dvec2 z, c, temp;\n"
    "    uint i;\n"
    "\n"
    "    c.x = 4.0 * (gl_FragCoord.x / size) * zoom - 2.0 * zoom + center.x;\n"
    "    c.y = 4.0 * (gl_FragCoord.y / size) * zoom - 2.0 * zoom + center.y;\n"
    "    z.x = 0.0;\n"
    "    z.y = 0.0;\n"
    "\n"
    "    for(i = 0; i < maxiter-1 && (z.x * z.x + z.y * z.y) < 4.0; i++)\n"
    "    {\n"
    "        temp.x = z.x * z.x - z.y * z.y + c.x;\n"
    "        temp.y = 2.0 * z.x * z.y + c.y;\n"
    "        z.x = temp.x;\n"
    "        z.y = temp.y;\n"
    "    }\n"
    "    if (maxiter < 256)\n"
    "    {\n"
    "        i = i * (256 / maxiter);\n"
    "    }\n"
    "    i = i % 256;\n"
    "    float m = float(i)/255.0;\n"
    "    color = vec4(m, m, m, 1.0);\n"
    "}\n";
    
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    GLint success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        GLint length;
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &length);
        GLchar* info = (GLchar*)calloc(length, sizeof(GLchar));
        glGetShaderInfoLog(vertex_shader, length, &length, info);
        fprintf(stderr, "glCompileShader failed:\n%s\n", info);
        free(info);
        glDeleteShader(vertex_shader);
        vertex_shader = 0;
    }
    
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        GLint length;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &length);
        GLchar* info = (GLchar*)calloc(length, sizeof(GLchar));
        glGetShaderInfoLog(fragment_shader, length, &length, info);
        fprintf(stderr, "glCompileShader failed:\n%s\n", info);
        free(info);
        glDeleteShader(fragment_shader);
        fragment_shader = 0;
    }
    
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        GLchar* info = (GLchar*)calloc(length, sizeof(GLchar));
        glGetProgramInfoLog(program, length, NULL, info);
        fprintf(stderr, "glLinkProgram failed:\n%s\n", info);
        free(info);
        glDetachShader(program, vertex_shader);
        glDetachShader(program, fragment_shader);

    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    
    
    float points[] =
    {
        -1.0f,  1.0f,  0.0f,
        -1.0f,  -1.0f,  0.0f,
        1.0f,  -1.0f,  0.0f,
        
        -1.0f,  1.0f,  0.0f,
        1.0f,  -1.0f,  0.0f,
        1.0f,  1.0f,  0.0f,
    };
    
    GLuint vbo = 0;
    glGenBuffers (1, &vbo);
    glBindBuffer (GL_ARRAY_BUFFER, vbo);
    glBufferData (GL_ARRAY_BUFFER, 2 * 9 * sizeof (float), points, GL_STATIC_DRAW);
    
    GLuint vao = 0;
    glGenVertexArrays (1, &vao);
    glBindVertexArray (vao);
    glEnableVertexAttribArray (0);
    glBindBuffer (GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
    glUseProgram(program);
    
    glBindVertexArray (vao);
    
    
    glUniform2d(glGetUniformLocation(program, "center"), center_x, center_y);
    glUniform1d(glGetUniformLocation(program, "zoom"), zoom);
    glUniform1d(glGetUniformLocation(program, "size"), (float)SIZE);
    glUniform1i(glGetUniformLocation(program, "maxiter"), maxiter);
    printf("ZOOM:%lldX  X:%.10f  Y:%.10f  MAX_ITER:%d\n", (long long)(1.0/zoom), center_x, center_y, maxiter);

    
    glutMainLoop();
}
