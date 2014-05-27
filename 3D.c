#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <math.h>
#include <string.h>

//#define WINDOW_WIDTH 800
//#define WINDOW_HEIGHT 600
int WINDOW_WIDTH=1520,WINDOW_HEIGHT=600;

Display *display;
int screen;
Window root;
int depth;
Visual *visual;
Colormap colormap;
XSetWindowAttributes winattr;
XWindowAttributes getattr;
int mask;
Window win;

EGLDisplay egldisplay;
EGLSurface eglsurface;
EGLint major, minor;
EGLint ret_count;
EGLint desired_config_attribs[]={
				EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
				EGL_RED_SIZE, 1,
				EGL_GREEN_SIZE, 1,
				EGL_BLUE_SIZE, 1,
				EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
				EGL_NONE
				};
EGLint desired_context_attribs[]={
				EGL_CONTEXT_CLIENT_VERSION, 2,
				EGL_NONE
				};
EGLConfig config;
EGLContext context;


GLuint glprogram;
GLuint glVShader, glFShader;
GLint prog_stat_ret;
GLchar *vShaderSrc = 	"attribute vec3 Position; \
			attribute vec4 Color; \
			uniform mat4 Model; \
			uniform mat4 View; \
			uniform mat4 Projection; \
			varying vec4 FSColor; \
					\
			void main() { \
				gl_Position = Projection * View * Model * vec4(Position, 1.0); \
				FSColor = Color;\
			 }";

GLchar *fShaderSrc = 	"precision mediump float;\
			varying vec4 FSColor; \
					\
			void main() { \
				gl_FragColor = FSColor; \
			}";	
GLuint glvbo;
GLfloat vertices[] = {
			-1.0,-1.0,1.0,     //front face
			1.0,-1.0,1.0,
			-1.0,1.0,1.0,
			1.0,1.0,1.0,
			1.0,-1.0,-1.0,     //back face
			-1.0,-1.0,-1.0,
			1.0,1.0,-1.0,
			-1.0,1.0,-1.0,
			1.0,-1.0,1.0,      //right face
			1.0,-1.0,-1.0,
			1.0,1.0,1.0,
			1.0,1.0,-1.0,
			-1.0,-1.0,-1.0,    //left face
			-1.0,-1.0,1.0,
			-1.0,1.0,-1.0,
			-1.0,1.0,1.0,
			-1.0,1.0,1.0,      //top face
			1.0,1.0,1.0,
			-1.0,1.0,-1.0,
			1.0,1.0,-1.0,
			1.0,-1.0,1.0,      //bottom face
			-1.0,-1.0,1.0,
			1.0,-1.0,-1.0,
			-1.0,-1.0,-1.0
			};
GLfloat colors[] = {
			1.0,0.0,0.0,0.3,   //red
			1.0,0.0,0.0,0.3,
			1.0,0.0,0.0,0.3,
			1.0,0.0,0.0,0.3,
			0.0,0.0,1.0,1.0,   //blue
			0.0,0.0,1.0,1.0,
			0.0,0.0,1.0,1.0,
			0.0,0.0,1.0,1.0,
			0.0,0.0,0.0,1.0,   //black
			0.0,0.0,0.0,1.0,
			0.0,0.0,0.0,1.0,
			0.0,0.0,0.0,1.0,
			1.0,1.0,1.0,1.0,   //white
			1.0,1.0,1.0,1.0,
			1.0,1.0,1.0,1.0,
			1.0,1.0,1.0,1.0,
			1.0,0.0,1.0,1.0,   //magenta
			1.0,0.0,1.0,1.0,
			1.0,0.0,1.0,1.0,
			1.0,0.0,1.0,1.0,
			1.0,1.0,0.0,1.0,   //yellow
			1.0,1.0,0.0,1.0,
			1.0,1.0,0.0,1.0,
			1.0,1.0,0.0,1.0,
			};

void swap(GLfloat *matrix, int index1, int index2){
	GLfloat temp;
	temp = *(matrix+index1);
	*(matrix+index1) = *(matrix+index2);
	*(matrix+index2) = temp;
}

void transpose(GLfloat *matrix){
	swap(matrix,1,4);
	swap(matrix,2,8);
	swap(matrix,3,12);
	swap(matrix,6,9);
	swap(matrix,7,13);
	swap(matrix,11,14);
}

void multiply(GLfloat *rmatrix, GLfloat *lmatrix){
	GLfloat a, b, c, d;
	int i;
	for(i=0; i<4; i++){
		a = lmatrix[0]*rmatrix[0+i] + lmatrix[1]*rmatrix[4+i] + lmatrix[2]*rmatrix[8+i] + lmatrix[3]*rmatrix[12+i];
		b = lmatrix[4]*rmatrix[0+i] + lmatrix[5]*rmatrix[4+i] + lmatrix[6]*rmatrix[8+i] + lmatrix[7]*rmatrix[12+i];
		c = lmatrix[8]*rmatrix[0+i] + lmatrix[9]*rmatrix[4+i] + lmatrix[10]*rmatrix[8+i] + lmatrix[11]*rmatrix[12+i];
		d = lmatrix[12]*rmatrix[0+i] + lmatrix[13]*rmatrix[4+i] + lmatrix[14]*rmatrix[8+i] + lmatrix[15]*rmatrix[12+i];
		rmatrix[0+i] = a;
		rmatrix[4+i] = b;
		rmatrix[8+i] = c;
		rmatrix[12+i] = d;
	}
}

static void printEGLConfig(EGLDisplay d, EGLConfig config)
{
    EGLint id, size, level;
    EGLint red, green, blue, alpha;
    EGLint depth, stencil;
    EGLint surfaces;
    EGLint doubleBuf = 1, stereo = 0;
    EGLint vid;
    EGLint samples, sampleBuffers;
    char surfString[100] = "";

    eglGetConfigAttrib(d, config, EGL_CONFIG_ID, &id);
    eglGetConfigAttrib(d, config, EGL_BUFFER_SIZE, &size);
    eglGetConfigAttrib(d, config, EGL_LEVEL, &level);

    eglGetConfigAttrib(d, config, EGL_RED_SIZE, &red);
    eglGetConfigAttrib(d, config, EGL_GREEN_SIZE, &green);
    eglGetConfigAttrib(d, config, EGL_BLUE_SIZE, &blue);
    eglGetConfigAttrib(d, config, EGL_ALPHA_SIZE, &alpha);
    eglGetConfigAttrib(d, config, EGL_DEPTH_SIZE, &depth);
    eglGetConfigAttrib(d, config, EGL_STENCIL_SIZE, &stencil);
    eglGetConfigAttrib(d, config, EGL_NATIVE_VISUAL_ID, &vid);
    eglGetConfigAttrib(d, config, EGL_SURFACE_TYPE, &surfaces);

    eglGetConfigAttrib(d, config, EGL_SAMPLES, &samples);
    eglGetConfigAttrib(d, config, EGL_SAMPLE_BUFFERS, &sampleBuffers);

    if (surfaces & EGL_WINDOW_BIT)
        strcat(surfString, "win,");
    if (surfaces & EGL_PBUFFER_BIT)
        strcat(surfString, "pb,");
    if (surfaces & EGL_PIXMAP_BIT)
        strcat(surfString, "pix,");
    if (strlen(surfString) > 0)
        surfString[strlen(surfString) - 1] = 0;

    printf("0x%02x %2d %2d %c  %c %2d %2d %2d %2d %2d %2d %2d%2d  0x%02x   %-12s\n",
           id, size, level,
           doubleBuf ? 'y' : '.',
           stereo ? 'y' : '.',
           red, green, blue, alpha,
           depth, stencil,
           samples, sampleBuffers, vid, surfString);
}


int main(int argc, char **argv){

	display = XOpenDisplay(0);
	if(!display){
		printf("X11 display could not be opened\n");
		exit(-1);
	}
    printf("Starting.. \n");
	screen = XDefaultScreen(display);
	root = RootWindow(display, screen);
	XGetWindowAttributes(display, root, &getattr);
	WINDOW_WIDTH=getattr.width;
	WINDOW_HEIGHT=getattr.height;
	depth = DefaultDepth(display, screen);
	visual = DefaultVisual(display, screen);
	colormap = XCreateColormap(display, root, visual, AllocNone);
	winattr.colormap = colormap;
	winattr.border_pixel = 0;
	winattr. background_pixel = 0;
	mask = CWBackPixel | CWBorderPixel | CWColormap;
	win=XCreateWindow(display, root, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, CopyFromParent, CopyFromParent, CopyFromParent, mask, &winattr);
//	win = root;
	XMapWindow(display, win);
	XFlush(display);
	egldisplay = eglGetDisplay((EGLNativeDisplayType)display);
	if(!eglInitialize(egldisplay, &major, &minor)){
		printf("failed to initialize EGL\n");
		exit(-1);
	}
	if(!eglBindAPI(EGL_OPENGL_ES_API)){
		printf("failed to bind api\n");
		exit(-1);
	}
	if(!eglChooseConfig(egldisplay, desired_config_attribs, &config, 1, &ret_count)){
		printf("failed to choose config\n");
		exit(-1);
	}
	if(ret_count < 1){
		printf("No configs returned\n");
		exit(-1);
	}
    printEGLConfig(egldisplay, config);
	if(EGL_NO_SURFACE == (eglsurface = eglCreateWindowSurface(egldisplay, config, (EGLNativeWindowType)win, NULL))){
		printf("failed to create EGL surface\n");
		exit(-1);
	}
	if(NULL == (context = eglCreateContext(egldisplay, config, EGL_NO_CONTEXT, desired_context_attribs))){
		printf("failed to create context\n");
		exit(-1);
	}
	eglMakeCurrent(egldisplay, eglsurface, eglsurface, context);
	glClearColor(1.0,0.6,0.3,1.0);
	glprogram = glCreateProgram();
	glVShader = glCreateShader(GL_VERTEX_SHADER);
  	glShaderSource(glVShader, 1, (const GLchar **)&vShaderSrc, NULL);
	glCompileShader(glVShader);
	glGetShaderiv(glVShader, GL_COMPILE_STATUS, &prog_stat_ret);
	if(!prog_stat_ret){
		int infoLen, totWritten;
		char *msg;
		glGetShaderiv(glVShader, GL_INFO_LOG_LENGTH, &infoLen);
		msg = malloc(infoLen+1);
		glGetShaderInfoLog(glVShader, infoLen, &totWritten, msg);
		printf("failed to compile vertex shader. Reason : %s\n",msg);
		free(msg);
		return -1;
	}
	glAttachShader(glprogram, glVShader);
	glFShader = glCreateShader(GL_FRAGMENT_SHADER);
  	glShaderSource(glFShader, 1, (const GLchar **)&fShaderSrc, NULL);
	glCompileShader(glFShader);
	glGetShaderiv(glFShader, GL_COMPILE_STATUS, &prog_stat_ret);
	if(!prog_stat_ret){
		int infoLen, totWritten;
		char *msg;
		glGetShaderiv(glFShader, GL_INFO_LOG_LENGTH, &infoLen);
		msg = malloc(infoLen+1);
		glGetShaderInfoLog(glFShader, infoLen, &totWritten, msg);
		printf("failed to compile fragment shader. Reason : %s\n",msg);
		free(msg);
		return -1;
	}
	glAttachShader(glprogram, glFShader);
	glLinkProgram(glprogram);
	glGetProgramiv(glprogram, GL_LINK_STATUS, &prog_stat_ret);
	if(!prog_stat_ret){
		int infoLen, totWritten;
		char *msg;
		glGetProgramiv(glprogram, GL_INFO_LOG_LENGTH, &infoLen);
		msg = malloc(infoLen+1);
		glGetProgramInfoLog(glprogram, infoLen, &totWritten, msg);
		printf("failed to link program. Reason : %s\n",msg);
		return -1;
	}
	glUseProgram(glprogram);

	glGenBuffers(1, &glvbo);
	glBindBuffer(GL_ARRAY_BUFFER, glvbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(colors), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), &colors[0]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(vertices));
	glBindAttribLocation(glprogram, 0, "Position");
	glBindAttribLocation(glprogram, 1, "Color");

	/* set the viewport and enable culling back faces	*/
	glViewport(0,0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glEnable(GL_CULL_FACE);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_ONE, GL_ONE);

	/* Now the draw loop					*/
	/* No window event, close etc handling done. simple 	*/
	/* while(1)						*/
	/* Close window when eyes hurt				*/
	GLfloat xangle = 0.0*M_PI/180.0;
	GLfloat yangle = 45.0*M_PI/180.0;
	GLfloat zangle = 10.0*M_PI/180.0;
	while(1){
		glClear(GL_COLOR_BUFFER_BIT);
	/* In this experiment we are adding 			*/
	/* model-view-perspective. Lets create the model matrix */
	/* first						*/
		GLfloat modelmatrix[] = {
				1,0,0,0,
				0,1,0,0,
				0,0,1,0,
				0,0,0,1
				};
		GLfloat xrotationmatrix[] = {
				1,0,0,0,
				0,cosf(xangle),-sinf(xangle),0,
				0,sinf(xangle),cosf(xangle),0,
				0,0,0,1
				};
		multiply(&modelmatrix[0],&xrotationmatrix[0]);
		GLfloat yrotationmatrix[] = {
				cosf(yangle),0,sinf(yangle),0,
				0,1,0,0,
				-sinf(yangle),0,cosf(yangle),0,
				0,0,0,1
				};
		multiply(&modelmatrix[0],&yrotationmatrix[0]);
		GLfloat zrotationmatrix[] = {
				cosf(zangle),sinf(zangle),0,0,
				-sinf(zangle),cosf(zangle),0,0,
				0,0,1,0,
				0,0,0,1
				};
		multiply(&modelmatrix[0],&zrotationmatrix[0]);
		GLfloat translatematrix[] = {
				0,0,0,10,
				0,0,0,0,
				0,0,0,0,
				0,0,0,1
				};
		//multiply(&modelmatrix[0],&translatematrix[0]);
		transpose(&modelmatrix[0]);
		xangle+=(0.25*M_PI/180.0);
		yangle+=(-0.5*M_PI/180.0);
		zangle+=(0.15*M_PI/180.0);
	/* Now the view matrix. We are moving camera away from	*/
	/* (0,0,0) to (0,0,8). That means we do a view 		*/
	/* translation by [0*i+0*j+(-8)*j]			*/
		GLfloat viewmatrix[] = {
				1,0,0,0,
				0,1,0,0,
				0,0,1,0,
				0,0,-3,1
				};
	/* create a perspective matrix 				*/
		GLfloat aspect = (GLfloat)WINDOW_WIDTH/WINDOW_HEIGHT;
		GLfloat fov = 50.0338*M_PI/180;
		GLfloat nearz = 1.0;
		GLfloat farz = 10.0;
		GLfloat projectionmatrix[] = {
					1.0/(aspect*tanf(fov/2)), 0, 0, 0,
					0, 1.0/(tanf(fov/2)), 0, 0,
					0, 0, (-nearz-farz)/(farz-nearz), -1,
					0, 0, (2*farz*nearz)/(nearz-farz), 0
					};
		//esTranslate(&modelview,0,0,-8);
		//esRotate(&modelview, 45+(0.25*count),1,0,0);
		//esRotate(&modelview, 45-(0.5*count),0,1,0);
		//esRotate(&modelview, 10+(0.15*count),0,0,1);
		

		//ESMatrix projection;
		//esMatrixLoadIdentity(&projection);
		//esFrustum(&projection, -2.6f*aspect, +2.6f*aspect, -2.6f, +2.6f, 1.0f, 10.0f);

		//ESMatrix modelviewprojection;
		//esMatrixLoadIdentity(&modelviewprojection);
		//esMatrixMultiply(&modelviewprojection, &modelview, &projection);
		GLint model = glGetUniformLocation(glprogram, "Model");
		GLint view = glGetUniformLocation(glprogram, "View");
		GLint projection = glGetUniformLocation(glprogram, "Projection");
		glUniformMatrix4fv(model, 1, GL_FALSE, (GLfloat *)&modelmatrix[0]);
		glUniformMatrix4fv(view, 1, GL_FALSE, (GLfloat *)&viewmatrix[0]);
		glUniformMatrix4fv(projection, 1, GL_FALSE, (GLfloat *)&projectionmatrix[0]);
	/* Ok draw the vertex array now. 6 points = 2 triangles	*/
	/* The shader will take care of vertices and coloring	*/
		glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);
		glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);
		glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);
		glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);
	/* Offscreen drawing done. Swap buffers.		*/
		eglSwapBuffers(egldisplay, eglsurface);
	}


}
