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
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/bind.hpp>
#include <queue>

//DEBUG
boost::recursive_mutex io_mutex;
extern volatile int rQueue_scope;
//</DEBUG>

int   main_window;
int current_slide_index;
unsigned char *current_slide;

int width = 1024;
int height = 600;
volatile int tdata_scope = 0;
/**** properties ****/
bool presenter_layer = true;
bool powerpoint_layer = true;
volatile bool recording = false;


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
unsigned char * tdata;
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
	  glEnd();
	  glPopMatrix();
	  glDisable(GL_TEXTURE_2D);
  }

  glutSwapBuffers();
}

void task1(){
	glutMainLoop();
}

void task2(){

	while(1){
		//std::cout << recording;
		//printf("processing queue\n");
		processQueue();
	}
}

void task3(){


	while(1){
		boost::this_thread::sleep(boost::posix_time::milliseconds(40));

		if(tdata_scope == 0){
			tdata_scope = 1;
			tdata = (unsigned char *)malloc(sizeof(unsigned char)*width*height*4);
			glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE, tdata);
			recordFrame(tdata);
			tdata_scope = 0;
		}





		if((int)recording == 1){
			//printf("Hi\n");
			//printf("DEBUG: %X\n",*(tdata  + 300));
			//recordFrame(tdata);
		  //processQueue();
		}
	}
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

boost::thread thread2;
void buttonCB(int button){
	switch(button){
	case 100:
		recording = true;
		initRecord();
		break;
	case 101:
		recording = false;
		endRecord();
		//thread2.detach();
		break;
	}

}


/**************************************** main() ********************/
int main(int argc, char* argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowPosition( 50, 50 );
  glutInitWindowSize( 1024, 600 );
  glViewport(0,0,width,height);

  current_slide = getSlide(1);
  tdata = (unsigned char *)malloc(sizeof(unsigned char)*width*height*4);
  start();

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

  new GLUI_Button(MediaBar, "Record",100,buttonCB);
  new GLUI_Column(MediaBar);
  new GLUI_Button(MediaBar, "Stop",101,buttonCB);


  MediaBar->set_main_gfx_window( main_window );
  PropertyBar->set_main_gfx_window( main_window );

  /* We register the idle callback with GLUI, *not* with GLUT */
  GLUI_Master.set_glutIdleFunc( myGlutIdle );

  using namespace boost;
  thread thread1 = thread(task1);
  thread2 = thread(task2);
  thread thread3 = thread(task3);
  thread1.join();
  thread2.join();
  thread3.join();
  return EXIT_SUCCESS;
}


