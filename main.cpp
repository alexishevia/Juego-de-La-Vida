/* Juego de la Vida de Conway
 * Reglas:
 * 1) Cualquier celda 'muerta' con exactamente 3 vecinos se convierte en una celda 'viva' 
 * 2) Cualquier celda 'viva' con un número de vecinos distinto a 2 ó 3, se convierte en una celda 'muerta'
 * 3) Las celdas vivas siempre inician en estado embriónico. 
 * 4) Cualquier celda que sobreviva pasa al siguiente estado (embriónico, larva, adulto).
 * 
 * El tablero se representa como una matriz bidimensional de enteros. Cada celda puede tener alguno de los siguientes valores:
 * 0 - celda muerta
 * 1 - estado embriónico 
 * 2 - estado larva
 * 3 - estado adulto
*/

#include <iostream>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "imageloader.h"

const bool manual = false;          //Determina si el cambio de generacion lo controla el usuario
const int milisegundos = 500;       //Tiempo entre cada actualización de pantalla
GLuint texturaVidrio;               //OpenGL id de la textura de vidrio
GLuint texturaLadrillo;             //OpenGL id de la textura de ladrillo

//Tamaño del juego
const int filas = 30;
const int columnas = 30;
struct dimension {
  int val[filas][columnas];
};
dimension tablero, aux;

//Base
const float mosaico_tamX = 3;
const float mosaico_tamY = 0.6;
const float mosaico_tamZ = 3;
const float espacio_X = 0.3;
const float espacio_Z = 0.3;

//Embrión
const float casa_tamX = 1.3;
const float casa_tamY = 1.1;
const float casa_tamZ = 1.3;
const float techo_tamX = 1.45;
const float techo_tamY = 0.65;
const float techo_tamZ = 1.55;

//Larva
const float larva_tamX = 1.7;
const float larva_tamY = 4;
const float larva_tamZ = 1.7;

//Adulto
const float adulto_tamX = 2;
const float adulto_tamY = 8;
const float adulto_tamZ = 2;

//Posición de la Escena
float posx = 2.0;
float posy = 8.0;
float posz = -100.0;
float rotx = 25.0;
float roty = 35.0;
float rotz = 0.0;
const int inc = 2.0;              //Incremento de la camara con cada pulsación del teclado

GLuint loadTexture(Image* image) {
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D,
                0,
                GL_RGB,
                image->width, image->height,
                0,
                GL_RGB,
                GL_UNSIGNED_BYTE,
                image->pixels);
    return textureId;
}

void initRendering() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_FLAT);
    
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE ) ;
    
    Image* image = loadBMP("vidrio.bmp");
    texturaVidrio = loadTexture(image);
    
    image = loadBMP("ladrillo.bmp");
    texturaLadrillo = loadTexture(image);
    delete image;
    
    //Estado inicial aleatorio
    for(int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            if(rand()%4 == 1)
                tablero.val[i][j] = random() % 3 + 1;
            else
                tablero.val[i][j] = 0;
        }
    }
}

int numVecinos(int fila, int columna) {
    int vecinos = 0;
    for(int i=fila-1; i<=fila+1; i++) {
        for(int j=columna-1; j<=columna+1; j++) {
            if(i>=0 and i<filas and j>=0 and j<columnas and !(i==fila and j==columna)) {
                if(tablero.val[i][j]>0)
                    vecinos++;
            }
        }
    }
    return vecinos;
}

void dibujarCajaTextura(float tamX, float tamY, float tamZ, GLuint textura) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textura);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBegin(GL_QUADS);
        //Cara Superior        
        glNormal3f(0.0, 1.0f, 0.0f);
        glVertex3f(-tamX/2, tamY/2, tamZ/2);
        glVertex3f(tamX/2, tamY/2, tamZ/2);
        glVertex3f(tamX/2, tamY/2, -tamZ/2);
        glVertex3f(-tamX/2, tamY/2, -tamZ/2);
        //Cara Inferior
        glNormal3f(0.0, -1.0f, 0.0f);
        glVertex3f(-tamX/2, -tamY/2, tamZ/2);
        glVertex3f(tamX/2, -tamY/2, tamZ/2);
        glVertex3f(tamX/2, -tamY/2, -tamZ/2);
        glVertex3f(-tamX/2, -tamY/2, -tamZ/2);
        //Cara Izq
        glNormal3f(-1.0, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-tamX/2, -tamY/2, -tamZ/2);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-tamX/2, -tamY/2, tamZ/2);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-tamX/2, tamY/2, tamZ/2);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-tamX/2, tamY/2, -tamZ/2);
        //Cara Der
        glNormal3f(-1.0, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(tamX/2, -tamY/2, -tamZ/2);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(tamX/2, -tamY/2, tamZ/2);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(tamX/2, tamY/2, tamZ/2);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(tamX/2, tamY/2, -tamZ/2);
        //Cara Frontal
        glNormal3f(0.0, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-tamX/2, -tamY/2, tamZ/2); 
        glTexCoord2f(1.0f, 0.0f); glVertex3f(tamX/2, -tamY/2, tamZ/2);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(tamX/2, tamY/2, tamZ/2);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-tamX/2, tamY/2, tamZ/2);
        //Cara Posterior
        glNormal3f(0.0, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-tamX/2, -tamY/2, -tamZ/2); 
        glTexCoord2f(1.0f, 0.0f); glVertex3f(tamX/2, -tamY/2, -tamZ/2);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(tamX/2, tamY/2, -tamZ/2);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-tamX/2, tamY/2, -tamZ/2);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
}

void dibujarCaja(float tamX, float tamY, float tamZ) {
    glBegin(GL_QUADS);
        //Cara Superior        
        glNormal3f(0.0, 1.0f, 0.0f);
        glVertex3f(-tamX/2, tamY/2, tamZ/2);
        glVertex3f(tamX/2, tamY/2, tamZ/2);
        glVertex3f(tamX/2, tamY/2, -tamZ/2);
        glVertex3f(-tamX/2, tamY/2, -tamZ/2);
        //Cara Inferior
        glNormal3f(0.0, -1.0f, 0.0f);
        glVertex3f(-tamX/2, -tamY/2, tamZ/2);
        glVertex3f(tamX/2, -tamY/2, tamZ/2);
        glVertex3f(tamX/2, -tamY/2, -tamZ/2);
        glVertex3f(-tamX/2, -tamY/2, -tamZ/2);
        //Cara Izq
        glNormal3f(-1.0, 0.0f, 0.0f);
        glVertex3f(-tamX/2, -tamY/2, -tamZ/2);
        glVertex3f(-tamX/2, -tamY/2, tamZ/2);
        glVertex3f(-tamX/2, tamY/2, tamZ/2);
        glVertex3f(-tamX/2, tamY/2, -tamZ/2);
        //Cara Der
        glNormal3f(-1.0, 0.0f, 0.0f);
        glVertex3f(tamX/2, -tamY/2, -tamZ/2);
        glVertex3f(tamX/2, -tamY/2, tamZ/2);
        glVertex3f(tamX/2, tamY/2, tamZ/2);
        glVertex3f(tamX/2, tamY/2, -tamZ/2);
        //Cara Frontal
        glNormal3f(0.0, 0.0f, 1.0f);
        glVertex3f(-tamX/2, -tamY/2, tamZ/2); 
        glVertex3f(tamX/2, -tamY/2, tamZ/2);
        glVertex3f(tamX/2, tamY/2, tamZ/2);
        glVertex3f(-tamX/2, tamY/2, tamZ/2);
        //Cara Posterior
        glNormal3f(0.0, 0.0f, 1.0f);
        glVertex3f(-tamX/2, -tamY/2, -tamZ/2); 
        glVertex3f(tamX/2, -tamY/2, -tamZ/2);
        glVertex3f(tamX/2, tamY/2, -tamZ/2);
        glVertex3f(-tamX/2, tamY/2, -tamZ/2);
    glEnd();
}

void dibujarTecho(float tamX, float tamY, float tamZ) {
    glBegin(GL_TRIANGLES);
        //Triangulo Izquierdo
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(-tamX/2, -tamY/2, -tamZ/2);
        glVertex3f(-tamX/2, -tamY/2, tamZ/2);
        glVertex3f(-tamX/2, tamY/2, 0);
        //Triangulo Derecho
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(tamX/2, -tamY/2, -tamZ/2);
        glVertex3f(tamX/2, -tamY/2, tamZ/2);
        glVertex3f(tamX/2, tamY/2, 0);
    glEnd();
    glBegin(GL_QUADS);
        //Cara Frontal
        glNormal3f(0.0, 1.0f, 1.0f);
        glVertex3f(-tamX/2, -tamY/2, tamZ/2);
        glVertex3f(tamX/2, -tamY/2, tamZ/2);
        glVertex3f(tamX/2, tamY/2, 0);
        glVertex3f(-tamX/2, tamY/2, 0);
        //Cara Posterior
        glNormal3f(0.0, 1.0f, -1.0f);
        glVertex3f(-tamX/2, -tamY/2, -tamZ/2);
        glVertex3f(tamX/2, -tamY/2, -tamZ/2);
        glVertex3f(tamX/2, tamY/2, 0);
        glVertex3f(-tamX/2, tamY/2, 0);
    glEnd();
}

void dibujarEmbrion() {
    glPushMatrix();
    
    //Casa
    float ambiente[] = {0.3,0.3,0.3,1.0};
    float difusa[] = {0.2,0.2,0.2,1.0};
    float especular[] = {0.3,0.3,0.3,1.0};
    float emision[] = {0.0,0.0,0.0,1.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambiente);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, difusa);
    glMaterialfv(GL_FRONT, GL_SPECULAR, especular);
    glMaterialfv(GL_FRONT, GL_EMISSION, emision);
    
    glTranslatef(0.0f, casa_tamY/2, 0.0f);    
    dibujarCaja(casa_tamX, casa_tamY, casa_tamZ);
    
    //Techo
    float ambienteTecho[] = {0.2,0.0,0.0,1.0};
    float difusaTecho[] = {0.2,0.0,0.0,1.0};
    float especularTecho[] = {0.2,0.0,0.0,1.0};
    float emisionTecho[] = {0.0,0.0,0.0,1.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambienteTecho);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, difusaTecho);
    glMaterialfv(GL_FRONT, GL_SPECULAR, especularTecho);
    glMaterialfv(GL_FRONT, GL_EMISSION, emisionTecho);
    
    glTranslatef(0.0f, (casa_tamY+techo_tamY)/2, 0.0f);    
    dibujarTecho(techo_tamX, techo_tamY, techo_tamZ);
    glPopMatrix();
}

void dibujarLarva() {
    glPushMatrix();
    
    float ambiente[] = {0.50,0.32,0.27,1.0};
    float difusa[] = {0.13,0.05,0,1.0};
    float especular[] = {0.13,0.34,0.01,1.0};
    float emision[] = {0.0,0.0,0.0,1.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambiente);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, difusa);
    glMaterialfv(GL_FRONT, GL_SPECULAR, especular);
    glMaterialfv(GL_FRONT, GL_EMISSION, emision);
    
    glTranslatef(0.0f, larva_tamY/2, 0.0f);    
    dibujarCajaTextura(larva_tamX, larva_tamY, larva_tamZ, texturaLadrillo);
    glPopMatrix();
}

void dibujarAdulto() {
    glPushMatrix();
    
    float ambiente[] = {0.22,0.28,0.37,1.0};
    float difusa[] = {0.11,0.23,0.22,1.0};
    float especular[] = {0.45,0.45,0.45,1.0};
    float emision[] = {0.0,0.0,0.0,1.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambiente);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, difusa);
    glMaterialfv(GL_FRONT, GL_SPECULAR, especular);
    glMaterialfv(GL_FRONT, GL_EMISSION, emision);
    
    glTranslatef(0.0f, adulto_tamY/2, 0.0f);    
    dibujarCajaTextura(adulto_tamX, adulto_tamY, adulto_tamZ,texturaVidrio);
    glPopMatrix();
}

void dibujarMosaico() {
    float ambiente[] = {0.2,0.2,0.2,1.0};
    float difusa[] = {0.3,0.3,0.3,1.0};
    float especular[] = {0.4,0.4,0.4,1.0};
    float emision[] = {0.0,0.0,0.0,1.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambiente);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, difusa);
    glMaterialfv(GL_FRONT, GL_SPECULAR, especular);
    glMaterialfv(GL_FRONT, GL_EMISSION, emision);
    
    dibujarCaja(mosaico_tamX, mosaico_tamY, mosaico_tamZ);
}

void dibujarTablero() {
    glPushMatrix();
   
    glTranslatef(-columnas*(mosaico_tamX+espacio_X)/2, 0.0f, filas*(mosaico_tamZ+espacio_Z)/2);
    for(int i = 0; i < filas; i++) {
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, -i*(mosaico_tamZ+espacio_Z));
        for (int j = 0; j < columnas; j++) {
            glPushMatrix();
            glTranslatef(j*(mosaico_tamX+espacio_X), 0.0f, 0.0f);
            dibujarMosaico();
            if(tablero.val[i][j] > 0) {
                if(tablero.val[i][j] == 1) { //embrion
                    dibujarEmbrion();
                }
                else if(tablero.val[i][j] == 2) { //larva
                    dibujarLarva();
                }
                else { //adulto
                    dibujarAdulto();
                }
            }
            glPopMatrix();
        }
        glPopMatrix();
    }
    glPopMatrix();
}

void update(int value) {
    aux = tablero;
    for(int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            int estado = tablero.val[i][j];
            int vecinos = numVecinos(i,j);
            if(estado==0) {
                if(vecinos==3)
                    aux.val[i][j] = 1;
            }
            else {
                if(vecinos!=2 and vecinos !=3)
                    aux.val[i][j] = 0;
                else {
                    if(estado<3)
                        aux.val[i][j] = estado + 1;
                    else
                        aux.val[i][j] = 3;
                }
            } 
        }
    }
    tablero = aux;
    glutPostRedisplay();
    if(!manual)
        glutTimerFunc(milisegundos, update, 0);
}

void drawScene() {
    glClearColor(0.0f,0.0f,0.0f,1.0f); //fondo negro
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    //Mover la escena
    glTranslatef(posx, posy, posz);
    
    //Rotar la escena
    glRotatef(rotx, 1.0, 0.0, 0.0);
    glRotatef(roty, 0.0, 1.0, 0.0);
    glRotatef(rotz, 0.0, 0.0, 1.0);
    
    // Iluminación 
    GLfloat ambientLight[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat diffuseLight[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat specularLight[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lightPos[] = {10.0, 0.5, 15.0, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);   
    
    //Dibujar
    dibujarTablero();
    glutSwapBuffers();
}

void handleSpecialKeys (int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_RIGHT:
            roty += inc;
            glutPostRedisplay();
            break;
            
        case GLUT_KEY_LEFT:
            roty -= inc;
            glutPostRedisplay();
            break;
        
        case GLUT_KEY_UP:
            rotx -= inc;
            if(rotx<0)
                rotx = 0;
            glutPostRedisplay();
            break;
            
        case GLUT_KEY_DOWN:
            rotx += inc;
            if(rotx >90)
                rotx = 90;
            glutPostRedisplay();
            break;
    }
}

void handleNormalKeys(unsigned char key, int x, int y) {
    switch(key) {
        case 27: //ESC
            exit(0);
            break;

        case 'a':
            posx += inc;
            glutPostRedisplay();
            break;
        case 's':
            posz -= inc;
            glutPostRedisplay();
            break;
        case 'w':
            posz += inc;
            glutPostRedisplay();
            break;
        case 'd':
            posx -= inc;
            glutPostRedisplay();
            break;
        case 'n':
            if(manual)
                glutTimerFunc(milisegundos, update, 0);
            break;
    }
}

void handleResize(int w, int h) {
    /** Projection **/
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 1.0, 500.0);
    
    /** ViewPort **/
    glViewport(0, 0, w, h);
}

int main(int argc, char** argv) {
    int semilla;
    if(argc > 1)
        semilla = atoi(argv[1]); //Toma el primer parámetro enviado desde la línea de comando como semilla
    else
        semilla = 25;
    srand(semilla);
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    
    glutCreateWindow("Juego de la Vida");
    initRendering();
    
    glutDisplayFunc(drawScene);
    glutReshapeFunc(handleResize);
    glutKeyboardFunc(handleNormalKeys);
    glutSpecialFunc(handleSpecialKeys);
    if(!manual)
        glutTimerFunc(milisegundos, update, 0);
    
    glutMainLoop();
    return 0;
}
