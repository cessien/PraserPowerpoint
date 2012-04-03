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
int   main_window;
int current_slide_index;
unsigned char *current_slide;
unsigned char * tdata;

int width = 1024;
int height = 600;
/**** properties ****/
bool presenter_layer = true;
bool powerpoint_layer = true;
volatile bool recording = false;

extern float *pos;

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
float cx,cy;
float * boundaries;
//unsigned char * tdata;

void myGlutDisplay( void ) {
  glClearColor( .0f, .0f, .0f, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  if(powerpoint_layer){
	  glColor4f(1,1,1,1);
	  glPushMatrix();
	  //Powerpoint Layer
	  glEnable(GL_TEXTURE_2D);
	  glBindTexture(GL_TEXTURE_2D, 10);
	  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
	  //glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	  glColor4f(1,1,1,1);
	  glPushMatrix();
	  //Kinect Layer
	  glEnable(GL_TEXTURE_2D);
	  glBindTexture(GL_TEXTURE_2D, 1);
	  glEnable (GL_BLEND);
	  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	  kinectDisplay();
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
		  cx = (center + size)*ratio + ((center - size)*ratio - (center + size)*ratio)/2.0f;
		  cy = -1.0f + (-1.0f + size * 2 - -1.0f)/2.0f;

	  glEnd();
	  glPopMatrix();
	  glDisable (GL_BLEND);
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
	glBegin(GL_LINES);
	  	glColor4f(1,0,0,1);
	  	glVertex3f(1,1,0);
	  	glVertex3f(-1,-1,0);
	  	//glVertex3f(,0,0);
	  	//glVertex3f(1,1,0);
	glEnd();
	glPopMatrix();


	glPushMatrix();
		glColor4f(0, 0, 1, 1);
		glPointSize(20);
		glBegin(GL_POINTS);
		//printf("DEBUG: %f %f\n", cx,cy);
//		glVertex2f(cx,cy);

		//<DEBUG>
			//Bottom left
			glVertex2f((center + size)*ratio, -1.0);
			//Top right
			glVertex2f((center - size)*ratio, -1.0 + size * 2);

			//Center
			glColor4f(1, 0, 1, 1);
			glVertex2f(cx,cy);
		//</DEBUG>
		glEnd();

		glPointSize(10);
		glBegin(GL_POINTS);
			glColor4f(0, 1, 0, 1);
			//Com.X center offset from k_Orgin translate to offset from p_Orgin
			//translate
			float pX,pY;
			pX = (center + size)*ratio + ((center - size)*ratio - (center + size)*ratio)*((pos[0]/640.0f - right)/(left - right));
			pY = -1.0f + (-1.0f + size * 2.0f - -1.0f)*((pos[1]/480.0f - bottom)/(top - bottom));

			glVertex2f(pX,pY);
//			glVertex2f(center + left*2.0f-1.0f, center + -2.0f*(top -0.5f));
			printf("X: %f, Y: %f\n", pX,pY);
		glEnd();
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
	case 102:
//		current_slide_index;
		current_slide = getSlide(current_slide_index);
		break;
	}
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
	new GLUI_Checkbox( temp, "Presenter", &presenter, 1, controlCB ); //presenter
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

	new GLUI_Button(MediaBar, "Record",100,buttonCB);
	new GLUI_Column(MediaBar);
	new GLUI_Button(MediaBar, "Stop",101,buttonCB);
	new GLUI_Column(MediaBar);
	new GLUI_Spinner(MediaBar, "Slide", GLUI_SPINNER_INT, &current_slide_index, 102, buttonCB);

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

/**************************************** main() ********************/
int main(int argc, char* argv[])
{
  glutInit(&argc, argv);
  current_slide_index = 1;
  current_slide = getSlide(current_slide_index);
//  tdata = (unsigned char *)malloc(sizeof(unsigned char)*width*height*4);
  start();


  using namespace boost;
  thread thread1(task1);
  thread thread2(task2);
//  thread thread3(task3);

//  thread threaddd4(task1);

//  task1();
  thread1.join();
//  thread2.join();
//  thread3.join();
 // glutMainLoop();
  return EXIT_SUCCESS;
}
