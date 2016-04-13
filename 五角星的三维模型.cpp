#include <gl/glut.h>
#include <stdio.h>
#include <stdlib.h>
#define BMP_Header_Length 54
static GLfloat xRot=0.0f;
static GLfloat yRot=0.0f;

int power_of_two(int n)
{
	if( n <= 0 )
		return 0;
	return (n & (n-1)) == 0;
}


GLuint load_texture(const char* file_name)
{
	GLint width, height, total_bytes;
	GLubyte* pixels = 0;
	GLuint last_texture_ID, texture_ID = 0;
	
	// 打开文件，如果失败，返回
	FILE* pFile = fopen(file_name, "rb");
	if( pFile == 0 )
		return 0;
	
	// 读取文件中图象的宽度和高度
	fseek(pFile, 0x0012, SEEK_SET);
	fread(&width, 4, 1, pFile);
	fread(&height, 4, 1, pFile);
	fseek(pFile, BMP_Header_Length, SEEK_SET);
	
	// 计算每行像素所占字节数，并根据此数据计算总像素字节数
	{
		GLint line_bytes = width * 3;
		while( line_bytes % 4 != 0 )
			++line_bytes;
		total_bytes = line_bytes * height;
	}
	
	// 根据总像素字节数分配内存
	pixels = (GLubyte*)malloc(total_bytes);
	if( pixels == 0 )
	{
		fclose(pFile);
		return 0;
	}
	
	// 读取像素数据
	if( fread(pixels, total_bytes, 1, pFile) <= 0 )
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}
	
	// 在旧版本的OpenGL中
	// 如果图象的宽度和高度不是的整数次方，则需要进行缩放
	// 这里并没有检查OpenGL版本，出于对版本兼容性的考虑，按旧版本处理
	// 另外，无论是旧版本还是新版本，
	// 当图象的宽度和高度超过当前OpenGL实现所支持的最大值时，也要进行缩放
	{
		GLint max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		if( !power_of_two(width)
			|| !power_of_two(height)
			|| width > max
			|| height > max )
		{
			const GLint new_width = 256;
			const GLint new_height = 256; // 规定缩放后新的大小为边长的正方形
			GLint new_line_bytes, new_total_bytes;
			GLubyte* new_pixels = 0;
			
			// 计算每行需要的字节数和总字节数
			new_line_bytes = new_width * 3;
			while( new_line_bytes % 4 != 0 )
				++new_line_bytes;
			new_total_bytes = new_line_bytes * new_height;
			
			// 分配内存
			new_pixels = (GLubyte*)malloc(new_total_bytes);
			if( new_pixels == 0 )
			{
				free(pixels);
				fclose(pFile);
				return 0;
			}
			
			// 进行像素缩放
			gluScaleImage(GL_RGB,
				width, height, GL_UNSIGNED_BYTE, pixels,
				new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);
			
			// 释放原来的像素数据，把pixels指向新的像素数据，并重新设置width和height
			free(pixels);
			pixels = new_pixels;
			width = new_width;
			height = new_height;
		}
	}
	
	// 分配一个新的纹理编号
	glGenTextures(1, &texture_ID);
	if( texture_ID == 0 )
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}
	
	// 绑定新的纹理，载入纹理并设置纹理参数
	// 在绑定前，先获得原来绑定的纹理编号，以便在最后进行恢复
	glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint *)&last_texture_ID);
	glBindTexture(GL_TEXTURE_2D, texture_ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, last_texture_ID);
	
	// 之前为pixels分配的内存可在使用glTexImage2D以后释放
	// 因为此时像素数据已经被OpenGL另行保存了一份（可能被保存到专门的图形硬件中）
	free(pixels);
	return texture_ID;
}

//第三段代码是关于显示的部分，以及main函数。注意，我们只在main函数中读取了两幅纹理，并把它们保存在各自的纹理对象中，以后就再也不载入纹理。每次绘制时使用glBindTexture在不同的纹理对象中切换。另外，我们使用了超过1.0的纹理坐标，由于GL_TEXTURE_WRAP_S和GL_TEXTURE_WRAP_T参数都被设置为GL_REPEAT，所以得到的效果就是纹理像素的重复，有点向地板砖的花纹那样。读者可以试着修改“墙”的纹理坐标，将5.0修改为10.0，看看效果有什么变化。

/* 两个纹理对象的编号
*/
GLuint texGround;
GLuint texWall;


void Initial()
{
	
	
	GLfloat mat_ambient[]={0.2f,0.2f,0.2f,1.0f};
	GLfloat mat_diffuse[]={0.8f,0.8f,0.8f,1.0f};
	GLfloat mat_specular[]={1.0f,1.0f,1.0f,1.0f};
	GLfloat mat_shininess[]={50.0f};
	GLfloat light0_diffuse[]={0.0f,0.0f,1.0f,1.0f};
	GLfloat light0_position[]={1.0f,1.0f,1.0f,0.0f};//无穷远处
	GLfloat light1_ambient[]={0.2f,0.2f,0.2f,1.0f};
	GLfloat light1_diffuse[]={1.0f,0.0f,0.0f,1.0f};
	GLfloat light1_specular[]={1.0f,0.6f,0.6f,1.0f};
	GLfloat light1_position[]={-0.3f,-0.3f,3.0f,1.0f};//局部光源
	GLfloat spot_direction[]={1.0f,1.0f,-1.0f};
	
	//定义材质属性
    glMaterialfv(GL_FRONT,GL_AMBIENT,mat_ambient);//指定材质的环境反射光反射系数
	glMaterialfv(GL_FRONT,GL_DIFFUSE,mat_diffuse);//指定材质的漫反射光反射系数
	glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);//指定材质的镜面反射光反射系数
	glMaterialfv(GL_FRONT,GL_SHININESS,mat_shininess);//指定材质的镜面反射指数值

    //light0为漫反射的蓝色点光源
	glLightfv(GL_LIGHT0,GL_DIFFUSE,light0_diffuse);//指定漫反射光成分
	glLightfv(GL_LIGHT0,GL_POSITION,light0_position);//设置光源的位置
	
	//light1为红色聚光光源
    glLightfv(GL_LIGHT1,GL_AMBIENT,light1_ambient);//指定环境光成分
	glLightfv(GL_LIGHT1,GL_DIFFUSE,light1_diffuse);//指定漫反射光成分
	glLightfv(GL_LIGHT1,GL_SPECULAR,light1_specular);//指定镜面光成分
	glLightfv(GL_LIGHT1,GL_POSITION,light1_position);//指定光源位置
	glLightf(GL_LIGHT1,GL_SPOT_CUTOFF,30.0);//指定聚光截止角
	glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,spot_direction);//指定聚光灯的方向
	
	glEnable(GL_LIGHTING);//启用光源
	glEnable(GL_LIGHT0);//启用光源
	glEnable(GL_LIGHT1);//启用光源
	glEnable(GL_DEPTH_TEST); // 启用深度测试
	
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f );	//背景为白色
}

void ChangeSize(int w, int h)
{
	if(h == 0)	h = 1;
    glViewport(0, 0, w, h);               // 设置视区尺寸
	glMatrixMode(GL_PROJECTION);    // 指定当前操作投影矩阵堆栈
	glLoadIdentity();                   // 重置投影矩阵
	
	GLfloat fAspect;
	fAspect = (float)w/(float)h;            // 计算视区的宽高比
	gluPerspective(90.0, fAspect, 5.0, 500.0); // 指定透视投影的观察空间
	glMatrixMode(GL_MODELVIEW);   
	glLoadIdentity();
}
void wujiaoxing()
{
	glBindTexture(GL_TEXTURE_2D, texGround);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(-9.5106,3.0902,0.0);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0,0.0,4.0);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-2.2452,3.0902,0.0);
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, texWall);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0,10.0,0.0);//A
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0,0.0,4.0);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(2.2452,3.0902,0.0);	//H	
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, texGround);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f);glVertex3f(9.5106,3.0902,0.0);//E
	glTexCoord2f(1.0f, 0.0f);glVertex3f(0.0,0.0,4.0);//O
	glTexCoord2f(0.0f, 0.0f);glVertex3f(2.2452,3.0902,0.0);	//H
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D,texWall);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f);glVertex3f(9.5106,3.0902,0.0);//E
	glTexCoord2f(1.0f, 0.0f);glVertex3f(0.0,0.0,4.0);//O
	glTexCoord2f(0.0f, 0.0f);glVertex3f(3.6328,-1.1804,0.0);	//G
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, texGround);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f);  glVertex3f(0.0,0.0,4.0);//O
	glTexCoord2f(1.0f, 0.0f); glVertex3f(3.6328,-1.1804,0.0);	//G
	glTexCoord2f(0.0f, 0.0f); glVertex3f(5.8779,-8.0902,0.0);	//D
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, texWall);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f);  glVertex3f(0.0,0.0,4.0);//O
	glTexCoord2f(1.0f, 0.0f); glVertex3f(5.8779,-8.0902,0.0);	//D
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0,-3.8197,0.0);	//F
	glEnd();
	
	
	glBindTexture(GL_TEXTURE_2D, texGround);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0,0.0,4.0);//O
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-5.8779,-8.0902,0.0);	//C
	glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0,-3.8197,0.0);	//F
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, texWall);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.0,0.0,4.0);//O
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-5.8779,-8.0902,0.0);	//C
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-3.6328,-1.1804,0.0);	//J
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, texGround);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f);  glVertex3f(0.0,0.0,4.0);//O
	glTexCoord2f(1.0f, 0.0f); glVertex3f(-9.5106,3.0902,0.0);//B
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-3.6328,-1.1804,0.0);	//J
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, texWall);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f);glVertex3f(-9.5106,3.0902,0.0);//B
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0,0.0,-4.0);//O
	glVertex3f(-2.2452,3.0902,0.0);//I		
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, texGround);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f);glVertex3f(0.0,10.0,0.0);//A
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.0,0.0,-4.0);//O
	glTexCoord2f(0.0f, 0.0f);glVertex3f(-2.2452,3.0902,0.0);//B		
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, texWall);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f);glVertex3f(0.0,10.0,0.0);//A
	glTexCoord2f(1.0f, 0.0f);glVertex3f(0.0,0.0,-4.0);//O
	glTexCoord2f(0.0f, 0.0f);glVertex3f(2.2452,3.0902,0.0);	//H	
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, texGround);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f);glVertex3f(9.5106,3.0902,0.0);//E
	glTexCoord2f(1.0f, 0.0f);glVertex3f(0.0,0.0,-4.0);//O
	glTexCoord2f(0.0f, 0.0f);glVertex3f(2.2452,3.0902,0.0);	//H	
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, texWall);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f);glVertex3f(9.5106,3.0902,0.0);//E
	glTexCoord2f(1.0f, 0.0f);glVertex3f(0.0,0.0,-4.0);//O
	glTexCoord2f(0.0f, 0.0f);glVertex3f(3.6328,-1.1804,0.0);	//G	
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, texGround);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f);glVertex3f(0.0,0.0,-4.0);//O
	glTexCoord2f(1.0f, 0.0f);glVertex3f(3.6328,-1.1804,0.0);	//G	
	glTexCoord2f(0.0f, 0.0f);glVertex3f(5.8779,-8.0902,0.0);	//D	
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, texWall);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f);glVertex3f(0.0,0.0,-4.0);//O
	glTexCoord2f(1.0f, 0.0f);glVertex3f(5.8779,-8.0902,0.0);	//D
	glTexCoord2f(0.0f, 0.0f);glVertex3f(0.0,-3.8197,0.0);	//F
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, texGround);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f);glVertex3f(0.0,0.0,-4.0);//O
	glTexCoord2f(1.0f, 0.0f);glVertex3f(-5.8779,-8.0902,0.0);	//C
	glTexCoord2f(0.0f, 0.0f);glVertex3f(0.0,-3.8197,0.0);	//F
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, texWall);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f);glVertex3f(0.0,0.0,-4.0);//O
	glTexCoord2f(1.0f, 0.0f);glVertex3f(-5.8779,-8.0902,0.0);	//C
	glTexCoord2f(0.0f, 0.0f);glVertex3f(-3.6328,-1.1804,0.0);	//J
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, texGround);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(1.0f, 1.0f);glVertex3f(0.0,0.0,-4.0);//O
	glTexCoord2f(1.0f, 0.0f);glVertex3f(-9.5106,3.0902,0.0);//B
	glTexCoord2f(0.0f, 0.0f);glVertex3f(-3.6328,-1.1804,0.0);	//J
	glEnd();
	
	
	
}
void Display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //清除颜色和深度缓冲区
	
	glMatrixMode(GL_MODELVIEW);  // 指定当前操作模型视图矩阵堆栈
	glLoadIdentity();                  // 重置模型视图矩阵
	glTranslatef(0.0f, 0.0f, -14.0f);	   //将图形沿z轴负向移动
	
    glRotatef(xRot, 1.0f, 0.0f, 0.0f);
	glRotatef(yRot, 0.0f, 1.0f, 0.0f);
	
	wujiaoxing();//画出第一颗五角星
	
    glPopMatrix(); 
	glutSwapBuffers();
}


void SpecialKeys(int key,int x,int y)
{//键盘控制旋转
	if(key==GLUT_KEY_UP) xRot-=5.0f;
    if(key==GLUT_KEY_DOWN) xRot+=5.0f;
    if(key==GLUT_KEY_LEFT) yRot-=5.0f;
	if(key==GLUT_KEY_RIGHT) yRot+=5.0f;
	
	if(xRot>356.0f) xRot=0.0f;
	if(xRot<-1.0f) xRot=355.0f;
	if(yRot>356.0f) yRot=0.0f;
	if(yRot<-1.0f) yRot=355.0f;
	
	glutPostRedisplay();
	
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);//申请z缓存
	glutCreateWindow("201321143076_庄小佳");
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(Display);
    glutSpecialFunc(SpecialKeys);
	Initial();
	
	glEnable(GL_TEXTURE_2D);
	texGround = load_texture("sky.bmp");//图像路径可以是绝对路径，也可以是相对路径
	texWall = load_texture("moon.bmp");
	
	glutMainLoop();
	return 0;
}
