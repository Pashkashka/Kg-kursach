#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);

		
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();
		
		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}

	
	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}

	
}

void mouseWheelEvent(OpenGL *ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;

}

void keyDownEvent(OpenGL *ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
}

void keyUpEvent(OpenGL *ogl, int key)
{
	
}



GLuint texId;

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	

	//массив трехбайтных элементов  (R G B)
	RGBTRIPLE *texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char *texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);

	
	
	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}




void Render(OpenGL *ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	//альфаналожение
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут  

	glBegin(GL_LINE_LOOP);
	double A[] = { 7,2,0 };
	double B[] = { 4,9,0 };
	double C[] = { 9,11,0 };
	double D[] = { 8,17,0 };
	double E[] = { 13,17,0 };
	double F[] = { 11,10,0 };
	double G[] = { 18,6,0 };
	double H[] = { 10,8,0 };
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(D);
	glVertex3dv(E);
	glVertex3dv(F);
	glVertex3dv(G);
	glVertex3dv(H);
	glEnd();

	glBegin(GL_LINE_LOOP);
	glColor3d(0.5, 0.5, 0.5);
	double A1[] = { 7,2,6 };
	double B1[] = { 4,9,6 };
	double C1[] = { 9,11,6 };
	double D1[] = { 8,17,6 };
	double E1[] = { 13,17,6 };
	double F1[] = { 11,10,6 };
	double G1[] = { 18,6,6 };
	double H1[] = { 10,8,6 };
	glVertex3dv(A1);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glVertex3dv(D1);
	glVertex3dv(E1);
	glVertex3dv(F1);
	glVertex3dv(G1);
	glVertex3dv(H1);
	glEnd();

	glBegin(GL_QUADS);
	

	glColor3d(1.0, 0.5, 0.5);
	double B112[] = { 4,9,0 };
	double C11[] = { 9,11,0 };
	double C22[] = { 9,11,6 };
	double B222[] = { 4,9,6 };
	//chet(B112, C11, B222);
	glVertex3dv(B112);
	glVertex3dv(C11);
	glVertex3dv(C22);
	glVertex3dv(B222);

	glColor3d(1.0, 0.5, 0.5);
	double C112[] = { 9,11,0 };
	double D11[] = { 8,17,0 };
	double D22[] = { 8,17,6 };
	double C222[] = { 9,11,6 };
	//chet(C112, D11, C222);
	glVertex3dv(C112);
	glVertex3dv(D11);
	glVertex3dv(D22);
	glVertex3dv(C222);

	glColor3d(1.0, 0.5, 0.5);
	double D112[] = { 8,17,0 };
	double E11[] = { 13,17,0 };
	double E22[] = { 13,17,6 };
	double D222[] = { 8,17,6 };
	//chet(D112, E11, D222);
	glVertex3dv(D112);
	glVertex3dv(E11);
	glVertex3dv(E22);
	glVertex3dv(D222);

	glColor3d(1.0, 0.5, 0.5);
	double E112[] = { 13,17,0 };
	double F11[] = { 11,10,0 };
	double F22[] = { 11,10,6 };
	double E222[] = { 13,17,6 };
	//chet(E112, F11, E222);
	glVertex3dv(E112);
	glVertex3dv(F11);
	glVertex3dv(F22);
	glVertex3dv(E222);

	glColor3d(1.0, 0.5, 0.5);
	double F112[] = { 11,10,0 };
	double G11[] = { 18,6,0 };
	double G22[] = { 18,6,6 };
	double F222[] = { 11,10,6 };
	//chet(F112, G11, F222);
	glVertex3dv(F112);
	glVertex3dv(G11);
	glVertex3dv(G22);
	glVertex3dv(F222);

	glColor3d(1.0, 0.5, 0.5);
	double G112[] = { 18,6,0 };
	double H11[] = { 10,8,0 };
	double H22[] = { 10,8,6 };
	double G222[] = { 18,6,6 };
	//chet(G112, H11, G222);
	glVertex3dv(G112);
	glVertex3dv(H11);
	glVertex3dv(H22);
	glVertex3dv(G222);

	glColor3d(1.0, 0.5, 0.5);
	double H112[] = { 10,8,0 };
	double A33[] = { 7,2,0 };
	double A44[] = { 7,2,6 };
	double H222[] = { 10,8,6 };
	//chet(H112, A33, H222);
	glVertex3dv(H112);
	glVertex3dv(A33);
	glVertex3dv(A44);
	glVertex3dv(H222);

	glColor3d(0.5, 0.5, 0.5);
	double A111[] = { 7,2,0 };
	double B111[] = { 4,9,0 };
	double C111[] = { 9,11,0 };
	double H111[] = { 10,8,0 };
	
	glVertex3dv(A111);
	glVertex3dv(B111);
	glVertex3dv(C111);
	glVertex3dv(H111);

	glColor3d(0.5, 0.5, 0.5);
	double A211[] = { 7,2,6 };
	double B211[] = { 4,9,6 };
	double C211[] = { 9,11,6 };
	double H211[] = { 10,8,6 };
	//chet(A211, B211, H211);
	glVertex3dv(A211);
	glVertex3dv(B211);
	glVertex3dv(C211);
	glVertex3dv(H211);

	glColor3d(0.5, 0.5, 0.5);
	double C121[] = { 9,11,0 };
	double D111[] = { 8,17,0 };
	double E111[] = { 13,17,0 };
	double F111[] = { 11,10,0 };
	//chet(C121, D111, F111);
	glVertex3dv(C121);
	glVertex3dv(D111);
	glVertex3dv(E111);
	glVertex3dv(F111);

	glColor3d(0.5, 0.5, 0.5);
	double C221[] = { 9,11,6 };
	double D211[] = { 8,17,6 };
	double E211[] = { 13,17,6 };
	double F211[] = { 11,10,6 };
	//chet(C221, D211, F211);
	glVertex3dv(C221);
	glVertex3dv(D211);
	glVertex3dv(E211);
	glVertex3dv(F211);



	glEnd();

	glBegin(GL_TRIANGLES);

	glColor3d(0.5, 0.5, 0.5);
	double C131[] = { 9,11,0 };
	double H131[] = { 10,8,0 };
	double F131[] = { 11,10,0 };
	glVertex3dv(C131);
	glVertex3dv(H131);
	glVertex3dv(F131);

	glColor3d(0.5, 0.5, 0.5);
	double C231[] = { 9,11,6 };
	double H231[] = { 10,8,6 };
	double F231[] = { 11,10,6 };
	glVertex3dv(C231);
	glVertex3dv(H231);
	glVertex3dv(F231);

	glColor3d(0.5, 0.5, 0.5);
	double G131[] = { 18,6,0 };
	double H331[] = { 10,8,0 };
	double F331[] = { 11,10,0 };
	glVertex3dv(G131);
	glVertex3dv(H331);
	glVertex3dv(F331);

	glColor3d(0.5, 0.5, 0.5);
	double G231[] = { 18,6,6 };
	double H332[] = { 10,8,6 };
	double F332[] = { 11,10,6 };
	glVertex3dv(G231);
	glVertex3dv(H332);
	glVertex3dv(F332);



	glEnd();

	glBegin(GL_QUADS);
	glColor4d(1.0, 0.5, 0.5, 0.3);
	double A11[] = { 7,2,0 };
	double B11[] = { 4,9,0 };
	double В22[] = { 4,9,6 };
	double A22[] = { 7,2,6 };
	//chet(A11, B11, A22);
	glVertex3dv(A11);
	glVertex3dv(B11);
	glVertex3dv(В22);
	glVertex3dv(A22);
	glEnd();


	////Начало рисования квадратика станкина
	//double A[2] = { -4, -4 };
	//double B[2] = { 4, -4 };
	//double C[2] = { 4, 4 };
	//double D[2] = { -4, 4 };

	//glBindTexture(GL_TEXTURE_2D, texId);

	//glColor3d(0.6, 0.6, 0.6);
	//glBegin(GL_QUADS);

	//glNormal3d(0, 0, 1);
	//glTexCoord2d(0, 0);
	//glVertex2dv(A);
	//glTexCoord2d(1, 0);
	//glVertex2dv(B);
	//glTexCoord2d(1, 1);
	//glVertex2dv(C);
	//glTexCoord2d(0, 1);
	//glVertex2dv(D);

	//glEnd();
	//конец рисования квадратика станкина


   //Сообщение вверху экрана

	
	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	                                //(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R="  << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;
	
	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}