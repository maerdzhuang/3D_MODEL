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
	
	// ���ļ������ʧ�ܣ�����
	FILE* pFile = fopen(file_name, "rb");
	if( pFile == 0 )
		return 0;
	
	// ��ȡ�ļ���ͼ��Ŀ�Ⱥ͸߶�
	fseek(pFile, 0x0012, SEEK_SET);
	fread(&width, 4, 1, pFile);
	fread(&height, 4, 1, pFile);
	fseek(pFile, BMP_Header_Length, SEEK_SET);
	
	// ����ÿ��������ռ�ֽ����������ݴ����ݼ����������ֽ���
	{
		GLint line_bytes = width * 3;
		while( line_bytes % 4 != 0 )
			++line_bytes;
		total_bytes = line_bytes * height;
	}
	
	// �����������ֽ��������ڴ�
	pixels = (GLubyte*)malloc(total_bytes);
	if( pixels == 0 )
	{
		fclose(pFile);
		return 0;
	}
	
	// ��ȡ��������
	if( fread(pixels, total_bytes, 1, pFile) <= 0 )
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}
	
	// �ھɰ汾��OpenGL��
	// ���ͼ��Ŀ�Ⱥ͸߶Ȳ��ǵ������η�������Ҫ��������
	// ���ﲢû�м��OpenGL�汾�����ڶ԰汾�����ԵĿ��ǣ����ɰ汾����
	// ���⣬�����Ǿɰ汾�����°汾��
	// ��ͼ��Ŀ�Ⱥ͸߶ȳ�����ǰOpenGLʵ����֧�ֵ����ֵʱ��ҲҪ��������
	{
		GLint max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		if( !power_of_two(width)
			|| !power_of_two(height)
			|| width > max
			|| height > max )
		{
			const GLint new_width = 256;
			const GLint new_height = 256; // �涨���ź��µĴ�СΪ�߳���������
			GLint new_line_bytes, new_total_bytes;
			GLubyte* new_pixels = 0;
			
			// ����ÿ����Ҫ���ֽ��������ֽ���
			new_line_bytes = new_width * 3;
			while( new_line_bytes % 4 != 0 )
				++new_line_bytes;
			new_total_bytes = new_line_bytes * new_height;
			
			// �����ڴ�
			new_pixels = (GLubyte*)malloc(new_total_bytes);
			if( new_pixels == 0 )
			{
				free(pixels);
				fclose(pFile);
				return 0;
			}
			
			// ������������
			gluScaleImage(GL_RGB,
				width, height, GL_UNSIGNED_BYTE, pixels,
				new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);
			
			// �ͷ�ԭ�����������ݣ���pixelsָ���µ��������ݣ�����������width��height
			free(pixels);
			pixels = new_pixels;
			width = new_width;
			height = new_height;
		}
	}
	
	// ����һ���µ�������
	glGenTextures(1, &texture_ID);
	if( texture_ID == 0 )
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}
	
	// ���µ������������������������
	// �ڰ�ǰ���Ȼ��ԭ���󶨵������ţ��Ա��������лָ�
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
	
	// ֮ǰΪpixels������ڴ����ʹ��glTexImage2D�Ժ��ͷ�
	// ��Ϊ��ʱ���������Ѿ���OpenGL���б�����һ�ݣ����ܱ����浽ר�ŵ�ͼ��Ӳ���У�
	free(pixels);
	return texture_ID;
}

//�����δ����ǹ�����ʾ�Ĳ��֣��Լ�main������ע�⣬����ֻ��main�����ж�ȡ�����������������Ǳ����ڸ��Ե���������У��Ժ����Ҳ����������ÿ�λ���ʱʹ��glBindTexture�ڲ�ͬ������������л������⣬����ʹ���˳���1.0���������꣬����GL_TEXTURE_WRAP_S��GL_TEXTURE_WRAP_T������������ΪGL_REPEAT�����Եõ���Ч�������������ص��ظ����е���ذ�ש�Ļ������������߿��������޸ġ�ǽ�����������꣬��5.0�޸�Ϊ10.0������Ч����ʲô�仯��

/* �����������ı��
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
	GLfloat light0_position[]={1.0f,1.0f,1.0f,0.0f};//����Զ��
	GLfloat light1_ambient[]={0.2f,0.2f,0.2f,1.0f};
	GLfloat light1_diffuse[]={1.0f,0.0f,0.0f,1.0f};
	GLfloat light1_specular[]={1.0f,0.6f,0.6f,1.0f};
	GLfloat light1_position[]={-0.3f,-0.3f,3.0f,1.0f};//�ֲ���Դ
	GLfloat spot_direction[]={1.0f,1.0f,-1.0f};
	
	//�����������
    glMaterialfv(GL_FRONT,GL_AMBIENT,mat_ambient);//ָ�����ʵĻ�������ⷴ��ϵ��
	glMaterialfv(GL_FRONT,GL_DIFFUSE,mat_diffuse);//ָ�����ʵ�������ⷴ��ϵ��
	glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);//ָ�����ʵľ��淴��ⷴ��ϵ��
	glMaterialfv(GL_FRONT,GL_SHININESS,mat_shininess);//ָ�����ʵľ��淴��ָ��ֵ

    //light0Ϊ���������ɫ���Դ
	glLightfv(GL_LIGHT0,GL_DIFFUSE,light0_diffuse);//ָ���������ɷ�
	glLightfv(GL_LIGHT0,GL_POSITION,light0_position);//���ù�Դ��λ��
	
	//light1Ϊ��ɫ�۹��Դ
    glLightfv(GL_LIGHT1,GL_AMBIENT,light1_ambient);//ָ��������ɷ�
	glLightfv(GL_LIGHT1,GL_DIFFUSE,light1_diffuse);//ָ���������ɷ�
	glLightfv(GL_LIGHT1,GL_SPECULAR,light1_specular);//ָ�������ɷ�
	glLightfv(GL_LIGHT1,GL_POSITION,light1_position);//ָ����Դλ��
	glLightf(GL_LIGHT1,GL_SPOT_CUTOFF,30.0);//ָ���۹��ֹ��
	glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,spot_direction);//ָ���۹�Ƶķ���
	
	glEnable(GL_LIGHTING);//���ù�Դ
	glEnable(GL_LIGHT0);//���ù�Դ
	glEnable(GL_LIGHT1);//���ù�Դ
	glEnable(GL_DEPTH_TEST); // ������Ȳ���
	
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f );	//����Ϊ��ɫ
}

void ChangeSize(int w, int h)
{
	if(h == 0)	h = 1;
    glViewport(0, 0, w, h);               // ���������ߴ�
	glMatrixMode(GL_PROJECTION);    // ָ����ǰ����ͶӰ�����ջ
	glLoadIdentity();                   // ����ͶӰ����
	
	GLfloat fAspect;
	fAspect = (float)w/(float)h;            // ���������Ŀ�߱�
	gluPerspective(90.0, fAspect, 5.0, 500.0); // ָ��͸��ͶӰ�Ĺ۲�ռ�
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //�����ɫ����Ȼ�����
	
	glMatrixMode(GL_MODELVIEW);  // ָ����ǰ����ģ����ͼ�����ջ
	glLoadIdentity();                  // ����ģ����ͼ����
	glTranslatef(0.0f, 0.0f, -14.0f);	   //��ͼ����z�Ḻ���ƶ�
	
    glRotatef(xRot, 1.0f, 0.0f, 0.0f);
	glRotatef(yRot, 0.0f, 1.0f, 0.0f);
	
	wujiaoxing();//������һ�������
	
    glPopMatrix(); 
	glutSwapBuffers();
}


void SpecialKeys(int key,int x,int y)
{//���̿�����ת
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
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);//����z����
	glutCreateWindow("201321143076_ׯС��");
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(Display);
    glutSpecialFunc(SpecialKeys);
	Initial();
	
	glEnable(GL_TEXTURE_2D);
	texGround = load_texture("sky.bmp");//ͼ��·�������Ǿ���·����Ҳ���������·��
	texWall = load_texture("moon.bmp");
	
	glutMainLoop();
	return 0;
}
