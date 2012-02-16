/****************************************************************************
Praser Application
- A demo application showing the capabilities of the Praser platform

  -----------------------------------------------------------------------
	2/14/2012  Simon ma, Charles Essien
****************************************************************************/

#include <string.h>
#include <GL/glui.h>
//#include "kinect.h"
#include "powerpoint.h"

int   main_window;
int current_slide_index;
unsigned char *current_slide;


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

void myGlutDisplay( void )
{
  glClearColor( .0f, .0f, .0f, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glPushMatrix();
  //Powerpoint Layer
  glEnable(GL_TEXTURE_2D);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glBindTexture(GL_TEXTURE_2D, 10);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, current_slide);
  //glColor4f(0.5f,0.0f,0.0f,1.0f);
  glBegin(GL_POLYGON);
  	  glTexCoord2f(0.0,0.5); glVertex3f(-1.0, 0.0, 0.0);
      glTexCoord2f(0.5,0.5); glVertex3f(0.0, 0.0, 0.0);
      glTexCoord2f(0.5,0.0); glVertex3f(0.0, 1.0, 0.0);
      glTexCoord2f(0.0,0.0); glVertex3f(-1.0, 1.0, 0.0);
  glEnd();
  glDisable(GL_TEXTURE_2D);
  glPopMatrix();


  glPushMatrix();
  //Kinect Layer
  glEnable(GL_TEXTURE_2D);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glBindTexture(GL_TEXTURE_2D, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, current_slide);
  //glColor4f(0.0f,0.0f,0.5f,1.0f);
  glBegin(GL_POLYGON);
  	  glTexCoord2f(-1.0,1.0); glVertex3f(0.0, 0.0, 0.0);
  	  glTexCoord2f(0.0,1.0); glVertex3f(1.0, 0.0, 0.0);
  	  glTexCoord2f(0.0,0.0); glVertex3f(1.0, 1.0, 0.0);
  	  glTexCoord2f(-1.0,0.0); glVertex3f(0.0, 1.0, 0.0);
  glEnd();
  glPopMatrix();

  glDisable(GL_TEXTURE_2D);

  glutSwapBuffers();
}

void controlCB(int control){
	current_slide = getSlide(1);
}

void buttonCB(int button){
	current_slide = getSlide(1);
}

/**************************************** main() ********************/
int main(int argc, char* argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowPosition( 50, 50 );
  glutInitWindowSize( 1024, 600 );
  glViewport(0,0,100,100);

  //start();
//  glutDisplayFunc(glutDisplay);

  main_window = glutCreateWindow( "Kinect PowerPoint Prototype" );
//  glutDisplayFunc( myGlutDisplay );
  //glInit(&argc, argv);
  //glutDisplayFunc(glutDisplay);
  glutDisplayFunc(myGlutDisplay);
  GLUI_Master.set_glutReshapeFunc( Reshape );

  int laser = 1;
  int presenter = 1;
  int powerpoint = 1;

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

  glutMainLoop();

  return EXIT_SUCCESS;
}


