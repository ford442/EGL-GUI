#include <float.h>
#include <vector>
#include <algorithm>
#include <string>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <SDL2/SDL.h>
using std::string;
#define __SHADER_H__

GLuint shaderProgLoad(const char *vertFilename,const char *fragFilename);
void shaderProgDestroy(GLuint shaderProg);
static size_t fileGetLength(FILE *file){
size_t length;
size_t currPos=ftell(file);
fseek(file,0,SEEK_END);
length=ftell(file);
fseek(file,currPos,SEEK_SET);
return length;
}
static GLuint shaderLoad(const char *filename,GLenum shaderType){
FILE *file=fopen(filename,"r");
size_t length=fileGetLength(file);
GLchar *shaderSrc=(GLchar *)calloc(length+1,1);
fread(shaderSrc,1,length,file);
fclose(file);
file=NULL;
GLuint shader=glCreateShader(shaderType);
glShaderSource(shader,1,(const GLchar**)&shaderSrc,NULL);
free(shaderSrc);
shaderSrc=NULL;
glCompileShader(shader);
return shader;
}

static void shaderDestroy(GLuint shaderID){
glDeleteShader(shaderID);
}

GLuint shaderProgLoad(const char *vertFilename,const char *fragFilename){
GLuint vertShader=shaderLoad(vertFilename,GL_VERTEX_SHADER);
GLuint fragShader=shaderLoad(fragFilename,GL_FRAGMENT_SHADER);
GLuint shaderProg=glCreateProgram();
glAttachShader(shaderProg,vertShader);
glAttachShader(shaderProg,fragShader);
glLinkProgram(shaderProg);
shaderDestroy(vertShader);
shaderDestroy(fragShader);
return shaderProg;
}
 
void shaderProgDestroy(GLuint shaderProg) {
glDeleteProgram(shaderProg);
}
 
typedef struct Vertex_s {
float position[2];
} Vertex;
 
GLuint vboCreate(const Vertex *vertices,GLuint numVertices);
void vboFree(GLuint vbo);
GLuint vboCreate(const Vertex *vertices, GLuint numVertices) {
GLuint vbo;
int nBuffers=1;
glGenBuffers(nBuffers,&vbo);
glBindBuffer(GL_ARRAY_BUFFER,vbo);
glBufferData(GL_ARRAY_BUFFER,sizeof(Vertex) * numVertices, vertices, GL_STATIC_DRAW);
glBindBuffer(GL_ARRAY_BUFFER,0);
SDL_Log("VBO: %d\n, vbo");
return vbo;
}

void vboFree(GLuint vbo) {
glDeleteBuffers(1, &vbo);
}

static EGLDisplay display;
static EGLContext contextegl;
static EGLSurface surface;
static EmscriptenWebGLContextAttributes attr;
static struct{SDL_AudioSpec spec;Uint8* snd;Uint32 slen;int pos;}wave;
SDL_Window *win;
SDL_GLContext *glCtx;
static GLuint shader_program;
static GLfloat mouseX=0.0f;
static GLfloat mouseY=0.0f;
static GLfloat mouseLPressed=0.0f;
static GLfloat mouseRPressed=0.0f;

static void renderFrame(){
Uint32 buttons;
int x,y;
SDL_PumpEvents();
buttons=SDL_GetMouseState(&x,&y);
if((buttons & SDL_BUTTON_LMASK)!=0){
mouseLPressed=1.0f;
glClearColor(0.0f,1.0f,0.0f,1.0f);
}else{
mouseLPressed=0.0f;
glClearColor(1.0f,0.0f,0.0f,1.0f);
}
glClear(GL_COLOR_BUFFER_BIT);
glUseProgram(shaderProg);
const Vertex vertices[]={
{0.0f,-0.9f},
{0.9f,0.9f},
{-0.9f,0.9f}
};
GLsizei vertSize=sizeof(vertices[0]);
GLsizei numVertices=sizeof(vertices)/vertSize;
GLuint triangleVBO=vboCreate(vertices,numVertices);
GLuint positionIdx=0;
glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
glVertexAttribPointer(positionIdx,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(const GLvoid*)0);
glEnableVertexAttribArray(positionIdx);
glDrawArrays(GL_TRIANGLES,0,numVertices);
eglSwapBuffers(display,surface);
}

static const EGLint attribut_list[]={
EGL_GL_COLORSPACE_KHR,EGL_GL_COLORSPACE_SRGB_KHR,
EGL_NONE
};

static const EGLint attribute_list[]={
EGL_COLOR_COMPONENT_TYPE_EXT,EGL_COLOR_COMPONENT_TYPE_FLOAT_EXT,
EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR,EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR,
EGL_RED_SIZE,32,
EGL_GREEN_SIZE,32,
EGL_BLUE_SIZE,32,
EGL_ALPHA_SIZE,32,
EGL_STENCIL_SIZE,32,
EGL_DEPTH_SIZE,32,
EGL_BUFFER_SIZE,64,
EGL_NONE
};

static void strt(){
SDL_GL_SetAttribute(SDL_GL_RED_SIZE,32);
SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,32);
SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,32);
SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE,32);
SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE,32);
SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE,32);
SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,32);
SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,32);
SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
attr.alpha=1;
attr.stencil=0;
attr.depth=0;
attr.antialias=0;
attr.premultipliedAlpha=0;
attr.preserveDrawingBuffer=0;
emscripten_webgl_init_context_attributes(&attr);
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx=emscripten_webgl_create_context("#canvas",&attr);
EGLConfig eglconfig=NULL;
EGLint config_size,major,minor;
display=eglGetDisplay(EGL_DEFAULT_DISPLAY);
eglInitialize(display,&major,&minor);
if(eglChooseConfig(display,attribute_list,&eglconfig,1,&config_size)==EGL_TRUE && eglconfig!=NULL){
if(eglBindAPI(EGL_OPENGL_ES_API)!=EGL_TRUE){
}
EGLint anEglCtxAttribs2[]={
EGL_CONTEXT_CLIENT_VERSION,3,
EGL_NONE};
contextegl=eglCreateContext(display,eglconfig,EGL_NO_CONTEXT,anEglCtxAttribs2);
if(contextegl==EGL_NO_CONTEXT){
}
else{
surface=eglCreateWindowSurface(display,eglconfig,NULL,attribut_list);
eglMakeCurrent(display,surface,surface,contextegl);
}}
emscripten_webgl_make_context_current(ctx);
int h=EM_ASM_INT({return parseInt(document.getElementById('pmhig').innerHTML,10);});
int w=h;
win=SDL_CreateWindow("Shadertoy",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,w,h,0);
glCtx=&contextegl;
SDL_SetWindowTitle(win,"1ink.us - GUI");
GLuint shaderProg=shaderProgLoad("Simple2D.vert","Simple2D.frag");
SDL_Log("GL_VERSION: %s",glGetString(GL_VERSION));
SDL_Log("GLSL_VERSION: %s",glGetString(GL_SHADING_LANGUAGE_VERSION));
SDL_Init(SDL_INIT_EVENTS);
glClearColor(0.0f,0.0f,0.0f,1.0f);
emscripten_set_main_loop((void (*)())renderFrame,0,0);
}
extern "C" {

void str(){
strt();
}}
int main(){
EM_ASM({
FS.mkdir("/");
});
return 1;
}
