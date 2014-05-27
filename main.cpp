/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "openglwindow.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>

#include <QtCore/qmath.h>

//! [1]
class CubeWindow : public OpenGLWindow
{
public:
    CubeWindow();

    void initialize();
    void render();

private:
    GLuint loadShader(GLenum type, const char *source);

    GLuint m_posAttr;
    GLuint m_colAttr;

    GLint m_model;
    GLint m_view;
    GLint m_projection;

    GLuint glprogram;
    GLuint glVShader, glFShader;
    GLint prog_stat_ret;

    GLfloat xangle;
    GLfloat yangle;
    GLfloat zangle;

    QOpenGLShaderProgram *m_program;
    int m_frame;
};

CubeWindow::CubeWindow()
    : m_program(0)
    , m_frame(0)
{
}
//! [1]

//! [2]
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat format;
    format.setRedBufferSize(1);
    format.setGreenBufferSize(1);
    format.setBlueBufferSize(1);
    format.setDepthBufferSize(0);
    format.setRenderableType(QSurfaceFormat::OpenGLES);

    CubeWindow window;
    window.setFormat(format);
    window.resize(1280, 800);
    window.show();

    window.setAnimating(true);

    return app.exec();
}
//! [2]


//! [3]

static const GLchar *vShaderSrc = 	"attribute vec3 Position; \
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

static const GLchar *fShaderSrc = 	"precision mediump float;\
varying vec4 FSColor; \
        \
void main() { \
    gl_FragColor = FSColor; \
}";
//! [3]
static const GLfloat vertices[] = {
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
static const GLfloat colors[] = {
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

//! [4]
GLuint CubeWindow::loadShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
    return shader;
}

void CubeWindow::initialize()
{
    glClearColor(1.0,0.6,0.3,1.0);
    glprogram = glCreateProgram();
    glVShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(glVShader, 1, (const GLchar **)&vShaderSrc, NULL);
    glCompileShader(glVShader);
    glGetShaderiv(glVShader, GL_COMPILE_STATUS, &prog_stat_ret);
    if(!prog_stat_ret){
        qFatal("Cannot Compile Vertex shader");
    }
    glAttachShader(glprogram, glVShader);
    glFShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(glFShader, 1, (const GLchar **)&fShaderSrc, NULL);
    glCompileShader(glFShader);
    glGetShaderiv(glFShader, GL_COMPILE_STATUS, &prog_stat_ret);
    if(!prog_stat_ret){
        qFatal("Cannot Compile Fragment shader");
    }
    glAttachShader(glprogram, glFShader);
    glLinkProgram(glprogram);
    glGetProgramiv(glprogram, GL_LINK_STATUS, &prog_stat_ret);
    if(!prog_stat_ret){
        qFatal("Cannot link program");
    }
    glUseProgram(glprogram);

    GLuint glvbo;
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
    glViewport(0,0, width(), height());
    glEnable(GL_CULL_FACE);
//	glEnable(GL_BLEND);
//	glBlendFunc(GL_ONE, GL_ONE);

    /* Now the draw loop					*/
    /* No window event, close etc handling done. simple 	*/
    /* while(1)						*/
    /* Close window when eyes hurt				*/
    xangle = 0.0*M_PI/180.0;
    yangle = 45.0*M_PI/180.0;
    zangle = 10.0*M_PI/180.0;
}
//! [4]
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


//! [5]
void CubeWindow::render()
{
//    qDebug() << Q_FUNC_INFO << m_frame;

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
    GLfloat aspect = (GLfloat)width()/height();
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
    ++m_frame;
}
//! [5]
