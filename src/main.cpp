/****************************************************************************
Praser Application
- A demo application showing the capabilities of the Praser platform

  -----------------------------------------------------------------------
	2/14/2012  Simon ma, Charles Essien
****************************************************************************/

#include <string.h>
#include <GL/glui.h>
#include <iostream>
#include "kinect.h"
#include "powerpoint.h"
#include "revelrecorder.h"
#include <boost/thread/thread.hpp>
//#include <boost/thread/mutex.hpp>
//#include <boost/bind.hpp>
#include <queue>

void task3();
void DrawLine(float ax, float ay, float bx, float by, float width, float r, float g, float b, float a);
void Circle( float x, float y, float rad,  float r, float g, float b, float a );
int   main_window;
int current_slide_index;
unsigned char *current_slide;
unsigned char * tdata;

int width = 1024;
int height = 600;
/**** properties ****/
bool presenter_layer = false;
bool powerpoint_layer = true;
volatile bool recording = false;

/**** GLUI Controls****/
int motor;

//boost::thread thread1;
//boost::thread thread2;
//boost::thread thread3;
/***************************************** myGlutIdle() ***********/

void myGlutIdle( void )
{
  /* According to the GLUT specification, the current window is
     undefined during an idle callback.  So we need to explicitly change
     it if necessary */
  if ( glutGetWindow() != main_window )
    glutSetWindow(main_window);

  glutPostRedisplay();
}


void Reshape( int x, int y )
{
  int tx, ty, tw, th;
  GLUI_Master.get_viewport_area( &tx, &ty, &tw, &th );
  glViewport( tx, ty, tw, th );

  //xy_aspect = (float)tw / (float)th;

  glutPostRedisplay();
}

/***************************************** myGlutDisplay() *****************/
float size = 0.5;
float center;
float ratio = 1.0;
float left,right,top,bottom;
float * boundaries;
//unsigned char * tdata;

void myGlutDisplay( void ) {
  glClearColor( .0f, .0f, .0f, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  if(powerpoint_layer){
	  glPushMatrix();
	  //Powerpoint Layer
	  glEnable(GL_TEXTURE_2D);
	  glBindTexture(GL_TEXTURE_2D, 10);
	  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 960, 720, 0, GL_RGBA, GL_UNSIGNED_BYTE, current_slide);
	  //glColor4f(0.5f,0.0f,0.0f,1.0f);
	  glBegin(GL_POLYGON);
		  glTexCoord2f(0.0,1.0); glVertex3f(-1.0, 1.0, 0.0);
		  glTexCoord2f(1.0,1.0); glVertex3f(1.0, 1.0, 0.0);
		  glTexCoord2f(1.0,0.0); glVertex3f(1.0, -1.0, 0.0);
		  glTexCoord2f(0.0,0.0); glVertex3f(-1.0, -1.0, 0.0);
	  glEnd();
	  glDisable(GL_TEXTURE_2D);
	  glPopMatrix();
  }

  if(presenter_layer){
	  glPushMatrix();
	  //Kinect Layer
	  glEnable(GL_TEXTURE_2D);
	  glBindTexture(GL_TEXTURE_2D, 1);
	  glEnable (GL_BLEND);
	  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	  kinectDisplay();
	  //glColor4f(0.0f,0.0f,0.5f,1.0f);
	  boundaries = getBoundary();
	  left = boundaries[0];
	  right = boundaries[1];
	  bottom = boundaries[2];
	  top = boundaries[3];
	  ratio = (left - right)/(top - bottom);
	  center = boundaries[4];
	  //printf("%f\n",boundaries[4]);
	  //printf("%f %f %f %f\n",boundaries[0],boundaries[1],boundaries[2],boundaries[3]);

	  delete boundaries;
	  glBegin(GL_POLYGON);
		  glTexCoord2f(right,bottom); glVertex3f((center + size)*ratio, -1.0, 0.1);
		  glTexCoord2f(left,bottom); glVertex3f((center - size)*ratio, -1.0, 0.1);
		  glTexCoord2f(left,top); glVertex3f((center - size)*ratio, -1.0 + size * 2, 0.1);
		  glTexCoord2f(right,top); glVertex3f((center + size)*ratio, -1.0 + size * 2, 0.1);
	  glEnd();
	  glPopMatrix();
	  glDisable(GL_TEXTURE_2D);
  }


	if(recording == 1){
		if(tdata != NULL)
			free(tdata);
		tdata = (unsigned char *)malloc(sizeof(unsigned char)*width*height*4);

//		boost::this_thread::sleep(boost::posix_time::milliseconds(33));
		glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE, tdata);
		recordFrame(tdata);
	}
	glPushMatrix();

//	for (float x=-1.0f; x<1.1f; x+=0.05f){
//		for (float y=-1.0f; y<1.1f; y+=0.05f){
//			//glutSwapBuffers();
//			DrawLine(0,0,x+0.1f,y+0.1f,1,0,0,1,1);
//		}
//	}

	//Circle( 0.5f, 0.5f, 0.5f, 1.0f ,0.0f ,0.0f ,1.0f );


//	DrawLine(1,1,-1,-1,5,0,0,1,1);
//	DrawLine(-1,-1,1,-1,5,0,1,1,1);
//	DrawLine(1,-1,0,0,5,1,0,1,1);
	glPopMatrix();
  glutSwapBuffers();
}

int laser = 1;
int presenter = 1;
int powerpoint = 1;

void controlCB(int control){
	switch(control){
	case 0:
		break;
	case 1:
		if (presenter == 1){
			presenter_layer = true;
		} else {
			presenter_layer = false;
		}
		break;
	case 2:
		if (powerpoint == 1){
			powerpoint_layer = true;
		} else {
			powerpoint_layer = false;
		}
		break;
	}
}


void buttonCB(int button){
	switch(button){
	case 1:
		printf("Split Initiate! \n");
		//method of split
		break;
	case 3:
		motorAngle(motor);
		break;
	case 100:
		recording = true;
		initRecord();
		break;
	case 101:
		recording = false;
		endRecord();
		break;

	}
}
void Split(){

}
void task1(){
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowPosition( 50, 50 );
	glutInitWindowSize( 1024, 600 );
	glViewport(0,0,width,height);
	main_window = glutCreateWindow( "Kinect PowerPoint Prototype" );
	glutDisplayFunc(myGlutDisplay);

	GLUI_Master.set_glutReshapeFunc( Reshape );

	/***************** GLUI window components ***********************/
	GLUI *PropertyBar = GLUI_Master.create_glui_subwindow(main_window, GLUI_SUBWINDOW_RIGHT);
	GLUI *MediaBar = GLUI_Master.create_glui_subwindow( main_window , GLUI_SUBWINDOW_BOTTOM);

	GLUI_Panel *temp = new GLUI_Panel(PropertyBar,"Layers", GLUI_PANEL_EMBOSSED);
	new GLUI_Checkbox( temp, "Laser", &laser, 0, controlCB ); //laser
	GLUI_Checkbox *presenterbox = new GLUI_Checkbox( temp, "Presenter", &presenter, 1, controlCB ); //presenter
	presenterbox->set_int_val(0);
	new GLUI_Checkbox( temp, "PowerPoint", &powerpoint,  2, controlCB ); //powerpoint

	new GLUI_Separator(PropertyBar);

	temp = new GLUI_Panel(PropertyBar,"Mode", GLUI_PANEL_EMBOSSED);
	new GLUI_Button(temp, "Combined", 0, buttonCB);
	new GLUI_Separator(temp);
	new GLUI_Button(temp, "Split", 1, buttonCB);

	new GLUI_Separator(PropertyBar);
	GLUI_Spinner *t = new GLUI_Spinner(PropertyBar, "Motor", GLUI_SPINNER_INT, &motor, 3, buttonCB);
	t->set_int_limits(-27,27,GLUI_LIMIT_CLAMP);
	t->set_speed(2);

	new GLUI_Separator(PropertyBar);
	t = new GLUI_Spinner(PropertyBar, "Slides", GLUI_SPINNER_INT, &motor, 4, buttonCB);
	t->set_int_limits(-27,27,GLUI_LIMIT_CLAMP);
	t->set_speed(2);

	new GLUI_Button(MediaBar, "Record",100,buttonCB);
	new GLUI_Column(MediaBar);
	new GLUI_Button(MediaBar, "Stop",101,buttonCB);

	MediaBar->set_main_gfx_window( main_window );
	PropertyBar->set_main_gfx_window( main_window );

	/* We register the idle callback with GLUI, *not* with GLUT */
	GLUI_Master.set_glutIdleFunc( myGlutIdle );

	glutMainLoop();
}

//boost::mutex _rqueue;
//boost::shared_mutex mutex;
//boost::shared_lock<boost::shared_mutex> ReadLock;
//boost::unique_lock<boost::shared_mutex> WriteLock;
boost::mutex mutex;


void task2(){

//	while(1){
//		//std::cout << recording;
//		//printf("processing queue\n");
//		processQueue();
//	}

//	boost::shared_lock< boost::mutex > lock(_rqueue);
//	boost::mutex::scoped_lock scoped_lock(_rqueue);
//	ReadLock read(mutex);
	while(1){
//		boost::lock_guard<boost::mutex> lock(mutex);
		if(recording == true)
			processQueue();
	}
}

void task3(){

//		boost::shared_lock< boost::mutex > lock(_rqueue);

	while(1){
//		boost::this_thread::sleep(boost::posix_time::milliseconds(40));
//		boost::lock_guard<boost::mutex> lock(mutex);

		if(recording == 1){
			unsigned char * tdata = (unsigned char *)malloc(sizeof(unsigned char)*width*height*4);
			boost::this_thread::sleep(boost::posix_time::milliseconds(30));
			glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE, tdata);
			printf("tdata: %i %i %i %i\n", (int)*(tdata + 300), (int)*(tdata + 301), (int)*(tdata + 302), (int)*(tdata + 303));
			recordFrame(tdata);

			//printf("Hi\n");
			//printf("DEBUG: %X\n",*(tdata  + 300));
			//recordFrame(tdata);
			//processQueue();
		}
	}
}
void DrawLine( float ax, float ay, float bx, float by, float width, float r, float g, float b, float a )
{
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f( r, g, b, a);

    glLineWidth(width);
    glBegin(GL_LINES);
    glVertex2f( ax, ay);
    glVertex2f( bx, by);
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glColor4f( 1, 1, 1, 1);
}
void Circle( float x, float y, float rad, float r, float g, float b, float a )
{
	glColor4f( r, g, b, a);
	glBegin( GL_TRIANGLE_FAN );
        glVertex2f( x, y );
        float i;
        for(i = 0.0f; i <= 2.0f * M_PI + 0.1f; i += 0.001f )
        {
            glVertex2f( x + 480.0f/640.0f*sin( i ) * rad, y + cos( i ) * rad );
            //glutSwapBuffers();
        }
    glEnd();
    glColor4f( 1, 1, 1, 1);
}

/**************************************** main() ********************/
int main(int argc, char* argv[])
{
  glutInit(&argc, argv);
  current_slide_index = 1;
  current_slide = getSlide(current_slide_index);
//  tdata = (unsigned char *)malloc(sizeof(unsigned char)*width*height*4);
//  start();




  using namespace boost;
//  thread thread1(task1);
//  thread thread2(task2);
//  thread thread3(task3);

//  thread threaddd4(task1);

//  task1();
//  thread1.join();
  task1();
//  thread2.join();
//  thread3.join();
 // glutMainLoop();
  printf("END");
  return EXIT_SUCCESS;
}
