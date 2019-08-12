/*Flappy Bird like mini-game		written with openGL
It's my first time using openGL
I spent too much time on reading bmp images but there are still some problems which I don't think I can solve them in time.
However, except the image problems, this mini game contains all of menu, score calculation, restart posiibility, and high score displaying
*/


#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <string>
#include <istream>
#include <stdarg.h>
#define _CRT_SECURE_NO_WARNINGS
#define GAP_LENGTH 200
#define WIDTH 400 //window width
#define HEIGHT 600 //window height
#define TUBE_WIDTH 70
#define FLY_HEIGHT 30
using namespace std;

typedef struct {
	double x;
	double y;
}Point, *pPoint;

typedef struct Tube {
	Point pt[4];
}Tube, *pTube;

typedef struct {
	Point pt[4];
}Bird, *pBird;

Bird bird;
pTube tube[2]; //at most 2 tube in screen
GLuint TexId[17];//amount of images
bool isCollide = false;
bool dead = true; 
bool onescore = false;//each tube carrys one score
double velosity;
int score = 0;
int highestScore;
GLuint base;

//function prototypes
int Rand_y();
void InitTube();
void Compute_Tube();
void Compute_Bird();
void Render();
void TimeFunc(int id);
unsigned char *ReadBmpFile(char *fname, int *w, int *h);
void InitOpenGL();
void DrawTube();
void DrawBird(int i);
void KeyboardFunc(unsigned char key, int x, int y);
void DrawBackground();
void Collide();
void Update_Player();
void Update_Tube();
void restart();
void Init();
void DrawScore(int i);
void DrawHighScore();
void DrawMenu();

int main(int argc, char **argv)
{
	//read highest score
	fstream f;
	f.open("highscore.txt", ios::in);
	string s;
	getline(f, s);
	highestScore = std::stoi(s);
	f.close();

	srand((unsigned)time(NULL));
	glutInit(&argc, argv);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(300, 100);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutCreateWindow("Flappy Bird");
	InitOpenGL();
	if (!dead) {
		Compute_Bird();
	}
	glutDisplayFunc(Render);
	glutTimerFunc(100, TimeFunc, 0);
	glutKeyboardFunc(KeyboardFunc);
	Init();
	glutMainLoop();
	return 0;
}

void Init() {//link image coordinates to window size
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glColor3f(0.0, 0.0, 0.0);
	glPointSize(2.0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, (GLdouble)WIDTH, 0, (GLdouble)HEIGHT);
}

int Rand_y() {// random length of the first tube in window height - gap height
	int i = (int)(rand() % (HEIGHT - GAP_LENGTH - 10));
	return i;
}
void InitTube() {
	for (int i = 0; i < 2; i++) {
		tube[i] = NULL;
	}
}
//set position : leftButton-rightButton-rightTop-leftTop
void Compute_Tube() {
	onescore = true;
		int a = -1;
		for (int i = 0; i < 2; i++) {
			if (tube[i] == NULL) {
				a = i;
				break;
			}
		}
		if (a == -1) return;
		//buttom tube when a = 0/2, top tube when a =1/3
		if (a % 2 == 0) {//buttom
			tube[a] = (pTube)malloc(sizeof(Tube));
			tube[a]->pt[0].x = WIDTH;
			tube[a]->pt[0].y = 0;
			tube[a]->pt[1].x = tube[a]->pt[0].x + TUBE_WIDTH;
			tube[a]->pt[1].y = tube[a]->pt[0].y;
			tube[a]->pt[2].x = tube[a]->pt[1].x;
			tube[a]->pt[2].y = (int)Rand_y();
			tube[a]->pt[3].x = tube[a]->pt[0].x;
			tube[a]->pt[3].y = tube[a]->pt[2].y;
		}
		else {//top
			tube[a] = (pTube)malloc(sizeof(Tube));
			tube[a]->pt[0].x = tube[a == 1 ? 0 : 2]->pt[0].x;
			tube[a]->pt[0].y = tube[a == 1 ? 0 : 2]->pt[2].y + GAP_LENGTH;
			tube[a]->pt[1].x = tube[a]->pt[0].x + TUBE_WIDTH;
			tube[a]->pt[1].y = tube[a]->pt[0].y;
			tube[a]->pt[2].x = tube[a]->pt[1].x;
			tube[a]->pt[2].y = HEIGHT;
			tube[a]->pt[3].x = tube[a]->pt[0].x;
			tube[a]->pt[3].y = tube[a]->pt[2].y;
		}
}
void Compute_Bird() {
	bird.pt[0].x = 20;
	bird.pt[0].y = HEIGHT/2;
	bird.pt[1].x = bird.pt[0].x + 48;
	bird.pt[1].y = bird.pt[0].y;
	bird.pt[2].x = bird.pt[1].x;
	bird.pt[2].y = bird.pt[0].y + 48;
	bird.pt[3].x = bird.pt[0].x;
	bird.pt[3].y = bird.pt[2].y;
}

void Render()
{
	static int x = 0;
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	if (dead) {
		//DrawHighScore();
		DrawMenu();
	}
	DrawScore(score);
	DrawBird(x);
	Compute_Tube();
	DrawTube();
	DrawBackground();
	glutSwapBuffers();
	x = (x + 1) % 3 + 10;//let bird wave its wings
}

void TimeFunc(int id)
{
	if (id == 0)
	{
		Collide();
		Update_Player();
		Update_Tube();
		glutPostRedisplay();
		if (dead == true) {
			fstream f;
			f.open("highscore.txt", ios::out);
			f << highestScore;
			f.close();
		}
		glutTimerFunc(100, TimeFunc, 0);
	}
}
//load image

unsigned char *ReadBmpFile(char *fname, int *w, int *h)//this function I searched from internet
{
	FILE *fp = fopen(fname, "rb");
	unsigned char *pData; // The pointer to the memory zone in which we will load the texture
						  // windows.h gives us these types to work with the Bitmap files
	BITMAPFILEHEADER fileheader;
	BITMAPINFOHEADER infoheader;
	RGBTRIPLE rgb;

	// Read the fileheader
	fread(&fileheader, sizeof(fileheader), 1, fp);
	fseek(fp, sizeof(fileheader), SEEK_SET); // Jump the fileheader
	fread(&infoheader, sizeof(infoheader), 1, fp); // and read the infoheader

	*w = infoheader.biWidth;
	*h = infoheader.biHeight;
	int size = infoheader.biWidth * infoheader.biHeight;

	// Now we need to allocate the memory for our image (width * height * color deep)
	pData = new byte[size * 4];
	// And fill it with zeros
	memset(pData, 0, size * 4);
	for (int i = 0, j = 0; i < size; ++i)
	{
		fread(&rgb, sizeof(rgb), 1, fp); // load RGB value
		pData[j + 0] = rgb.rgbtRed;
		pData[j + 1] = rgb.rgbtGreen;
		pData[j + 2] = rgb.rgbtBlue;
		pData[j + 3] = 255; // Alpha value
		j += 4;
	}
	fclose(fp);
	return pData;
}



void InitOpenGL() {
	glEnable(GL_DEPTH_TEST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glGenTextures(17, TexId);
	int w, h;
	unsigned char *pImage;

	pImage = ReadBmpFile("./FBImage/0.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;

	pImage = ReadBmpFile("./FBImage/1.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;

	pImage = ReadBmpFile("./FBImage/2.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;

	pImage = ReadBmpFile("./FBImage/3.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[3]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;

	pImage = ReadBmpFile("./FBImage/4.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[4]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;

	pImage = ReadBmpFile("./FBImage/5.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[5]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;

	pImage = ReadBmpFile("./FBImage/6.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[6]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;

	pImage = ReadBmpFile("./FBImage/7.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[7]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;

	pImage = ReadBmpFile("./FBImage/8.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[8]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;

	pImage = ReadBmpFile("./FBImage/9.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[9]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;

	pImage = ReadBmpFile("./FBImage/bird1.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[10]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;

	pImage = ReadBmpFile("./FBImage/bird2.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[11]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;

	pImage = ReadBmpFile("./FBImage/bird3.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[12]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;

	pImage = ReadBmpFile("./FBImage/tubeTop.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[13]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;

	pImage = ReadBmpFile("./FBImage/tubeButtom.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[14]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;

	pImage = ReadBmpFile("./FBImage/background.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[15]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;

	pImage = ReadBmpFile("./FBImage/score.bmp", &w, &h);
	glBindTexture(GL_TEXTURE_2D, TexId[16]);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pImage);
	delete[]pImage;
}

//load image
void DrawBackground() {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, TexId[15]);
	glBegin(GL_QUADS);
	//glTexCoord2d: buttomLeft(0,0),topRight(1,1)
	glTexCoord2d(0.0, 0.0); glVertex2d(0, 0);
	glTexCoord2d(1.0, 0.0); glVertex2d(WIDTH, 0);
	glTexCoord2d(1.0, 1.0); glVertex2d(WIDTH, HEIGHT);
	glTexCoord2d(0.0, 1.0); glVertex2d(0, HEIGHT);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void DrawTube() {
	for (int i = 0; i < 2; i++) {
		if (tube[i] != NULL) {
			int x = (i%2 == 0? 14:13);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, TexId[x]);

				glBegin(GL_QUADS);
				glTexCoord2d(0, 0);
				glVertex2d(tube[i]->pt[0].x, tube[i]->pt[0].y);

				glTexCoord2d(1, 0);
				glVertex2d(tube[i]->pt[1].x, tube[i]->pt[1].y);

				glTexCoord2d(1, 1);
				glVertex2d(tube[i]->pt[2].x, tube[i]->pt[2].y);

				glTexCoord2d(0, 1);
				glVertex2d(tube[i]->pt[3].x, tube[i]->pt[3].y);
				glEnd();
				glDisable(GL_TEXTURE_2D);
		}
	}
}

void DrawBird(int i) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, TexId[i]);
	glTranslatef(0, 0, 0);

	glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glNormal3i(0, 0, 1);
	glVertex3d(bird.pt[0].x, bird.pt[0].y, 0);
	glTexCoord2i(1, 0);
	glNormal3i(0, 0, 1);
	glVertex3d(bird.pt[1].x, bird.pt[1].y, 0);
	glTexCoord2i(1, 1);
	glNormal3i(0, 0, 1);
	glVertex3d(bird.pt[2].x, bird.pt[2].y, 0);
	glTexCoord2i(0, 1);
	glNormal3i(0, 0, 1);
	glVertex3d(bird.pt[3].x, bird.pt[3].y, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void DrawScore(int i) {
	for (int a = 1; a < 3; a++) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, TexId[a%2==0?(score/10):(score%10)]);
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glNormal3i(0, 0, 1);
		glVertex3i(WIDTH - 40*a, HEIGHT - 40, 0);
		glTexCoord2i(1, 0);
		glNormal3i(0, 0, 1);
		glVertex3i(WIDTH - 40*(a-1), HEIGHT - 40, 0);
		glTexCoord2i(1, 1);
		glNormal3i(0, 0, 1);
		glVertex3i(WIDTH - 40 * (a - 1), HEIGHT, 0);
		glTexCoord2i(0, 1);
		glNormal3i(0, 0, 1);
		glVertex3i(WIDTH - 40*a, HEIGHT, 0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
}

void DrawHighScore() {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, TexId[16]);
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glNormal3i(0, 0, 1);
	glVertex3i(WIDTH/2-80, HEIGHT/2 - 80, 0);
	glTexCoord2i(1, 0);
	glNormal3i(0, 0, 1);
	glVertex3i(WIDTH/2+80, HEIGHT/2 - 80, 0);
	glTexCoord2i(1, 1);
	glNormal3i(0, 0, 1);
	glVertex3i(WIDTH/2+80, HEIGHT/2+80, 0);
	glTexCoord2i(0, 1);
	glNormal3i(0, 0, 1);
	glVertex3i(WIDTH/2-80, HEIGHT/2+80, 0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void RenderBitmapString(float x, float y, void *font, char *string) {
	char *c;
	::glRasterPos2f(x, y);
	for (c = string; *c != '\0'; c++) {
		::glutBitmapCharacter(font, *c);
	}
	::glRasterPos2f(x + 1, y);
	for (c = string; *c != '\0'; c++) {
		::glutBitmapCharacter(font, *c);
	}
}

void DrawMenu() {
	int* pFont = (int*)GLUT_BITMAP_TIMES_ROMAN_24;

	GLint viewport[4];
	::glGetIntegerv(GL_VIEWPORT, viewport);
	const int win_w = viewport[2];
	const int win_h = viewport[3];

	glPushMatrix();
	glLoadIdentity();
	glScalef(1, -1, 1);
	glTranslatef(0, -win_h, 0);

	char s_tmp[256];
	int interval = 15;
	glColor3d(1.0, 0.0, 0.0);


	sprintf(s_tmp, "%d", score);
	char other_string[64] = "Your current score is: "; 
	strcat(other_string, s_tmp);
	RenderBitmapString(WIDTH/4, HEIGHT/3, pFont, other_string);

	sprintf(s_tmp, "%d", highestScore);
	char other_str[64] = "Highest score is: ";
	strcat(other_str, s_tmp);
	RenderBitmapString(WIDTH / 4, HEIGHT / 3+24, pFont, other_str);


	strcpy(s_tmp, "Press space to start");
	RenderBitmapString(WIDTH / 4, HEIGHT / 3 + 24 * 3, pFont, s_tmp);

	strcpy(s_tmp, "Press space to fly");
	RenderBitmapString(WIDTH / 4, HEIGHT / 3 + 24 * 4, pFont, s_tmp);


	strcpy(s_tmp, "Press esc to exit");
	RenderBitmapString(WIDTH / 4, HEIGHT / 3 + 24 * 5, pFont, s_tmp);

	strcpy(s_tmp, "	          Menu");
	RenderBitmapString(WIDTH / 4, HEIGHT / 3 - 48, pFont, s_tmp);
	glPopMatrix();
}

void KeyboardFunc(unsigned char key, int x, int y) {
	switch(key) {
	case 27:  //27 for esc
		exit(0);
		break;
	case ' ': //space to fly and restart if died
		if (!dead) {
			if (bird.pt[3].y < HEIGHT) {
				for (int i = 0; i < 4; i++) {
					velosity = FLY_HEIGHT;
				}
				printf("fly!\n");
			}
		}
		else { restart(); }
		break;
	default:
		break;
	}
}

void Update_Player() {
	//bird keeps falling down if it does not reach buttom
	if (bird.pt[0].y >= 5) {
		for (int i = 0; i < 4; i++) {
			bird.pt[i].y += velosity;
		}

		velosity -= 7;
	}
	else { velosity = 0; }
	
	if (bird.pt[0].y <= 5 || bird.pt[2].y >= HEIGHT-5) {//die if reach top or buttom
		dead = true;
	}

	if (!dead) {//get score
		if (onescore) {
			if (bird.pt[0].x >= tube[0]->pt[1].x) {
				onescore = false;
				score = score + 1;
				if (score > highestScore) {
					highestScore = score;
				}
				cout <<"score is: " << score << endl;
				cout << "highest score is: " << highestScore << endl;
			}
		}
	}
}

void Update_Tube() {
	if (!dead) {
		for (int i = 0; i < 2; i++) {
			if (tube[i] != NULL) {
				for (int j = 0; j < 4; j++) {
					tube[i]->pt[j].x -= 20;
				}
				if (tube[i]->pt[1].x < -1) {
					free(tube[i]);
					tube[i] = NULL;
				}
			}
		}
	}
}

void Collide() {
	if (tube[0] != NULL) {
		if ((bird.pt[1].x >= tube[0]->pt[0].x) && (bird.pt[0].x <= tube[0]->pt[2].x) && ((bird.pt[1].y <= tube[0]->pt[2].y) || (bird.pt[2].y >= tube[1]->pt[1].y))) {
			dead = true;
		}
	}
}

void restart() {
	isCollide = false;
	dead = false;
	InitTube();
	Compute_Tube();
	Compute_Bird();
	velosity = 0.0;
	score = 0;
}

