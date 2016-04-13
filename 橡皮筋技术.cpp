#include <gl/glut.h>
#include <iostream>
using namespace std;
int iPointNum=0;           //��ȷ�������Ŀ
double x1=0,x2=0,y1=0,y2=0;    //ȷ���ĵ�����
int winWidth=400,winHeight=300;//���ڵĿ�Ⱥ͸߶�

void Initial(void)
{
	glClearColor(1.0f,1.0f,1.0f,1.0f);//���ô��ڱ�����ɫ
}

void ChangeSize(GLint w,GLint h)
{
	winWidth=w;//���浱ǰ���ڵĴ�С
	winHeight=h;
	GLdouble left,right,down,top;
	left=-2.0;
	right=2.0;
	down=-(double)h/w*(right-left)/2;
	top=(double)h/w*(right-left)/2;//��ֹ��Ϊ���죬���Ŵ��ڵ���ͼ���α�
	glViewport(0,0,w,h);//ָ��������ʾ����
	glMatrixMode(GL_PROJECTION);//ָ������ͶӰ����
	glLoadIdentity();//���õ�λ����ȥ����ǰ��ͶӰ����λ��
    gluOrtho2D(left,right,down,top);//����ͶӰ����	

}

void Display(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();//��λ�任
	glClear(GL_COLOR_BUFFER_BIT);//�õ�ǰ����ɫ��䴰��
	
	if(iPointNum>=1)
	{//��Ӧ������
		glBegin(GL_QUADS);//�����ı���
		glColor3f(1.0f,0.0f,0.0f);//ָ����ǰ�Ļ�ͼ��ɫ
        glVertex2f(x1,y1);
		glColor3f(0.0f,0.0f,1.0f);
		glVertex2f(x1,y2);
		glColor3f(0.0f,1.0f,1.0f);
		glVertex2f(x2,y2);
		glColor3f(0.60f,0.40f,0.70f);
		glVertex2f(x2,y1);
		
		glEnd();
	}
	else
	{//��Ӧ����Ҽ�
		glColor3f(0.75f,0.75f,0.75f);
		glTranslated(x2,y2,0.0);//�����ƵĲ��ƽ�Ƶ�(x2,y2)��λ��
		glutSolidTeapot(0.2);//���Ʋ��
	}
	glutSwapBuffers();//����������
}

void MousePlot(GLint button,GLint action,GLint xMouse,GLint yMouse)
{
	if(button==GLUT_LEFT_BUTTON&&action==GLUT_DOWN)
	{
		cout<<xMouse<<","<<yMouse<<endl;
		if(iPointNum==0||iPointNum==2)
		{
			iPointNum=1;
			x1=(xMouse-winWidth/2.0)/100;
			y1=(winHeight/2.0-yMouse)/100;//ȷ���ı��εĵ�һ���˵�
		}
		else
		{
			iPointNum=2;
			x2=(xMouse-winWidth/2.0)/100;
			y2=(winHeight/2.0-yMouse)/100;//ȷ���ı��εĵڶ����˵�
			glutPostRedisplay();//ָ���������»���
		}
	}
	if(button==GLUT_RIGHT_BUTTON&&action==GLUT_DOWN)
    {
        iPointNum=0;	
		glutPostRedisplay();
	}
}

void PassiveMouseMove(GLint xMouse,GLint yMouse)
{
	if(iPointNum==1)
	{
		x2=(xMouse-winWidth/2.0)/100;
		y2=(winHeight/2.0-yMouse)/100;//����ǰ���λ��ָ��Ϊֱ�ߵ�δ�̶��˵�
		glutPostRedisplay();
	}
}

int main(int argc,char *argv[])
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);//ʹ��˫���漰RGBģ��
	glutInitWindowSize(400,300);//ָ�����ڵĳߴ�
	glutInitWindowPosition(100,100);//ָ����������Ļ�ϵ�λ��
	glutCreateWindow("��Ƥ���");
	glutDisplayFunc(Display);
	glutReshapeFunc(ChangeSize);//ָ�����������λص�����
	glutMouseFunc(MousePlot);//ָ�������Ӧ����
	glutPassiveMotionFunc(PassiveMouseMove);//ָ������ƶ���Ӧ����
	Initial();
	glutMainLoop();//������GLUT�¼�����ѭ��
	return 0;
}
