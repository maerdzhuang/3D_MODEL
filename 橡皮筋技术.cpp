#include <gl/glut.h>
#include <iostream>
using namespace std;
int iPointNum=0;           //已确定点的数目
double x1=0,x2=0,y1=0,y2=0;    //确定的点坐标
int winWidth=400,winHeight=300;//窗口的宽度和高度

void Initial(void)
{
	glClearColor(1.0f,1.0f,1.0f,1.0f);//设置窗口背景颜色
}

void ChangeSize(GLint w,GLint h)
{
	winWidth=w;//保存当前窗口的大小
	winHeight=h;
	GLdouble left,right,down,top;
	left=-2.0;
	right=2.0;
	down=-(double)h/w*(right-left)/2;
	top=(double)h/w*(right-left)/2;//防止因为拉伸，缩放窗口导致图形形变
	glViewport(0,0,w,h);//指定窗口显示区域
	glMatrixMode(GL_PROJECTION);//指定设置投影参数
	glLoadIdentity();//调用单位矩阵，去掉以前的投影参数位置
    gluOrtho2D(left,right,down,top);//设置投影参数	

}

void Display(void)
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();//单位变换
	glClear(GL_COLOR_BUFFER_BIT);//用当前背景色填充窗口
	
	if(iPointNum>=1)
	{//响应鼠标左键
		glBegin(GL_QUADS);//绘制四边形
		glColor3f(1.0f,0.0f,0.0f);//指定当前的绘图颜色
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
	{//响应鼠标右键
		glColor3f(0.75f,0.75f,0.75f);
		glTranslated(x2,y2,0.0);//将绘制的茶壶平移到(x2,y2)的位置
		glutSolidTeapot(0.2);//绘制茶壶
	}
	glutSwapBuffers();//交换缓冲区
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
			y1=(winHeight/2.0-yMouse)/100;//确定四边形的第一个端点
		}
		else
		{
			iPointNum=2;
			x2=(xMouse-winWidth/2.0)/100;
			y2=(winHeight/2.0-yMouse)/100;//确定四边形的第二个端点
			glutPostRedisplay();//指定窗口重新绘制
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
		y2=(winHeight/2.0-yMouse)/100;//将当前鼠标位置指定为直线的未固定端点
		glutPostRedisplay();
	}
}

int main(int argc,char *argv[])
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);//使用双缓存及RGB模型
	glutInitWindowSize(400,300);//指定窗口的尺寸
	glutInitWindowPosition(100,100);//指定窗口在屏幕上的位置
	glutCreateWindow("橡皮筋技术");
	glutDisplayFunc(Display);
	glutReshapeFunc(ChangeSize);//指定窗口再整形回调函数
	glutMouseFunc(MousePlot);//指定鼠标响应函数
	glutPassiveMotionFunc(PassiveMouseMove);//指定鼠标移动响应函数
	Initial();
	glutMainLoop();//启动主GLUT事件处理循环
	return 0;
}
