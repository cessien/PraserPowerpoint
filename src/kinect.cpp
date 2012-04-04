/*******************************************************************************
*                                                                              *
*   PrimeSense NITE 1.3 - Players Sample                                       *
*   Copyright (C) 2010 PrimeSense Ltd.                                         *
*                                                                              *
*******************************************************************************/

#include <XnOpenNI.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>
#include <GL/glut.h>
#include "SceneDrawer.h"
#include <cmath>
#include "kinect.h"
#include "XnVNite.h"
#include "powerpoint.h"

xn::Context g_Context;
xn::ScriptNode g_ScriptNode;
xn::DepthGenerator g_DepthGenerator;
xn::ImageGenerator g_ImageGenerator;
xn::UserGenerator g_UserGenerator;
xn::Recorder* g_pRecorder;
xn::HandsGenerator g_HandsGenerator;
xn::GestureGenerator g_GestureGenerator;
XnVPointDrawer* g_pDrawer;


XnVSessionManager* g_pSessionManager;
XnVBroadcaster* broadcaster;

XnVSwipeDetector *swipeDetector;
XnVCircleDetector *circleDetector;
XnVPushDetector *pushDetector;

extern int current_slide_index;
extern unsigned char *current_slide;
extern int num_slides;
int LEFT, RIGHT, TOP, BOTTOM;
float CENTER;

//Returns an array of the boundaries in (left, right, top, bottom).
float * getBoundary(){
	float * temp = (float *)malloc(sizeof(float) * 5);

	if(LEFT != 0){
		temp[0] = (float)LEFT/640;
		//printf("Left: %f  ", temp[0]);
	} else {
		temp[0] = 1.0;
	}

	if(RIGHT != 640){
		temp[1] = (float)RIGHT/640;
		//printf("Right: %f  ", temp[1]);
	} else {
		temp[1] = 0.0;
	}

	if(TOP != 0){
		temp[2] = (float)TOP/480;
		//printf("Top: %f  ", temp[2]);
	} else {
		temp[2] = 1.0;
	}

	if(BOTTOM != 480){
		temp[3] = (float)BOTTOM/480;
		//printf("Bottom: %f  ", temp[3]);
	} else {
		temp[3] = 0.0;
	}

	temp[4] = CENTER/-480.0;
	//printf("\n");
	return temp;
}

XnUserID g_nPlayer = 0;
XnBool g_bCalibrated = FALSE;

#define GL_WIN_SIZE_X 720
#define GL_WIN_SIZE_Y 480

#define START_CAPTURE_CHECK_RC(rc, what)												\
	if (nRetVal != XN_STATUS_OK)														\
{																					\
	printf("Failed to %s: %s\n", what, xnGetStatusString(rc));				\
	StopCapture();															\
	return ;																	\
}

XnBool g_bPause = false;
XnBool g_bRecord = false;

XnBool g_bQuit = false;
void StopCapture()
{
	g_bRecord = false;
	if (g_pRecorder != NULL)
	{
		g_pRecorder->RemoveNodeFromRecording(g_DepthGenerator);
		g_pRecorder->Release();
		delete g_pRecorder;
	}
	g_pRecorder = NULL;
}

void CleanupExit()
{
	if (g_pRecorder)
		g_pRecorder->RemoveNodeFromRecording(g_DepthGenerator);
	StopCapture();

	exit (1);
}

void StartCapture()
{
	char recordFile[256] = {0};
	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
        XnUInt32 size;
        xnOSStrFormat(recordFile, sizeof(recordFile)-1, &size,
                 "%d_%02d_%02d[%02d_%02d_%02d].oni",
                timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	if (g_pRecorder != NULL)
	{
		StopCapture();
	}

	XnStatus nRetVal = XN_STATUS_OK;
	g_pRecorder = new xn::Recorder;

	g_Context.CreateAnyProductionTree(XN_NODE_TYPE_RECORDER, NULL, *g_pRecorder);
	START_CAPTURE_CHECK_RC(nRetVal, "Create recorder");

	nRetVal = g_pRecorder->SetDestination(XN_RECORD_MEDIUM_FILE, recordFile);
	START_CAPTURE_CHECK_RC(nRetVal, "set destination");
	nRetVal = g_pRecorder->AddNodeToRecording(g_DepthGenerator, XN_CODEC_16Z_EMB_TABLES);
	START_CAPTURE_CHECK_RC(nRetVal, "add node");
	g_bRecord = true;
}

XnBool AssignPlayer(XnUserID user)
{
	if (g_nPlayer != 0)
		return FALSE;

	XnPoint3D com;
	g_UserGenerator.GetCoM(user, com);
	if (com.Z == 0)
		return FALSE;

	printf("Matching for existing calibration\n");
	g_UserGenerator.GetSkeletonCap().LoadCalibrationData(user, 0);
	g_UserGenerator.GetSkeletonCap().StartTracking(user);
	g_nPlayer = user;
	return TRUE;

}
void XN_CALLBACK_TYPE NewUser(xn::UserGenerator& generator, XnUserID user, void* pCookie)
{
	if (!g_bCalibrated) // check on player0 is enough
	{
		printf("Look for pose\n");
		g_UserGenerator.GetPoseDetectionCap().StartPoseDetection("Psi", user);
		return;
	}

	AssignPlayer(user);
 	if (g_nPlayer == 0)
 	{
 		printf("Assigned user\n");
 		g_UserGenerator.GetSkeletonCap().LoadCalibrationData(user, 0);
 		g_UserGenerator.GetSkeletonCap().StartTracking(user);
 		g_nPlayer = user;
 	}

}
void FindPlayer()
{
	if (g_nPlayer != 0)
	{
		return;
	}
	XnUserID aUsers[1];
	XnUInt16 nUsers = 1;
	g_UserGenerator.GetUsers(aUsers, nUsers);

	for (int i = 0; i < nUsers; ++i)
	{
		if (AssignPlayer(aUsers[i]))
			return;
	}
}
void LostPlayer()
{
	g_nPlayer = 0;
	FindPlayer();

}
void XN_CALLBACK_TYPE LostUser(xn::UserGenerator& generator, XnUserID user, void* pCookie)
{
	printf("Lost user %d\n", user);
	if (g_nPlayer == user)
	{
		LostPlayer();
	}
}
void XN_CALLBACK_TYPE PoseDetected(xn::PoseDetectionCapability& pose, const XnChar* strPose, XnUserID user, void* cxt)
{
	printf("Found pose \"%s\" for user %d\n", strPose, user);
	g_UserGenerator.GetSkeletonCap().RequestCalibration(user, TRUE);
	g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(user);
}

void XN_CALLBACK_TYPE CalibrationStarted(xn::SkeletonCapability& skeleton, XnUserID user, void* cxt)
{
	printf("Calibration started\n");
}

void XN_CALLBACK_TYPE CalibrationEnded(xn::SkeletonCapability& skeleton, XnUserID user, XnBool bSuccess, void* cxt)
{
	printf("Calibration done [%d] %ssuccessfully\n", user, bSuccess?"":"un");
	if (bSuccess)
	{
		if (!g_bCalibrated)
		{
			g_UserGenerator.GetSkeletonCap().SaveCalibrationData(user, 0);
			g_nPlayer = user;
			g_UserGenerator.GetSkeletonCap().StartTracking(user);
			g_bCalibrated = TRUE;
		}

		XnUserID aUsers[10];
		XnUInt16 nUsers = 10;
		g_UserGenerator.GetUsers(aUsers, nUsers);
		for (int i = 0; i < nUsers; ++i)
			g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(aUsers[i]);
	}
}

void XN_CALLBACK_TYPE CalibrationCompleted(xn::SkeletonCapability& skeleton, XnUserID user, XnCalibrationStatus eStatus, void* cxt)
{
	printf("Calibration done [%d] %successfully\n", user, (eStatus == XN_CALIBRATION_STATUS_OK)?"":"un");
	if (eStatus == XN_CALIBRATION_STATUS_OK)
	{
		if (!g_bCalibrated)
		{
			g_UserGenerator.GetSkeletonCap().SaveCalibrationData(user, 0);
			g_nPlayer = user;
			g_UserGenerator.GetSkeletonCap().StartTracking(user);
			g_bCalibrated = TRUE;
		}

		XnUserID aUsers[10];
		XnUInt16 nUsers = 10;
		g_UserGenerator.GetUsers(aUsers, nUsers);
		for (int i = 0; i < nUsers; ++i)
			g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(aUsers[i]);
	}
}

/*void DrawProjectivePoints(XnPoint3D& ptIn, int width, double r, double g, double b)
{
	static XnFloat pt[3];

	pt[0] = ptIn.X;
	pt[1] = ptIn.Y;
	pt[2] = 0;
	glColor4f(r,
		g,
		b,
		1.0f);
	glPointSize(width);
	glVertexPointer(3, GL_FLOAT, 0, pt);
	glDrawArrays(GL_POINTS, 0, 1);

	glFlush();

}*/

unsigned char *image;
xn::SceneMetaData sceneMD;
xn::DepthMetaData depthMD;
xn::ImageMetaData imageMD;
XnPoint3D com2;
// this function is called each frame
void kinectDisplay (void)
{
	// Setup the OpenGL viewpoint
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();


	g_DepthGenerator.GetMetaData(depthMD);
	g_ImageGenerator.GetMetaData(imageMD);
	glOrtho(0, depthMD.XRes(), depthMD.YRes(), 0, -1.0, 1.0);


		// Read next available data
		g_Context.WaitAnyUpdateAll();
		g_pSessionManager->Update(&g_Context);

		// Process the data
		//DRAW
		g_DepthGenerator.GetMetaData(depthMD);
		g_ImageGenerator.GetMetaData(imageMD);
		g_UserGenerator.GetUserPixels(0, sceneMD);

		DrawDepthMap(depthMD, sceneMD, g_nPlayer, imageMD);

		if (g_nPlayer != 0)
		{
			g_UserGenerator.GetCoM(g_nPlayer, com2);
			if (com2.Z == 0)
			{
				g_nPlayer = 0;
				FindPlayer();
			}
		}
		glPopMatrix();
}

void glutIdle (void)
{
	if (g_bQuit) {
		CleanupExit();
	}

	// Display the frame
	glutPostRedisplay();
}

void glutKeyboard (unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		CleanupExit();
	case'p':
		g_bPause = !g_bPause;
		break;
	case 'k':
		if (g_pRecorder == NULL)
			StartCapture();
		else
			StopCapture();
		printf("Record turned %s\n", g_pRecorder ? "on" : "off");
		break;
	}
}






void glInit (int * pargc, char ** argv)
{
//	image  = raw_texture_load("slide.png",640,480);
//	glutInit(pargc, argv);
//	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
//	glutInitWindowSize(GL_WIN_SIZE_X, GL_WIN_SIZE_Y);
//	glutCreateWindow ("PrimeSense Nite Players Viewer");
//	glutFullScreen();
	glutSetCursor(GLUT_CURSOR_NONE);

	glutKeyboardFunc(glutKeyboard);
//	glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutIdle);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}


#define SAMPLE_XML_PATH "Sample-User.xml"

#define CHECK_RC(rc, what)											\
	if (rc != XN_STATUS_OK)											\
	{																\
		printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
		return rc;													\
	}

#define CHECK_ERRORS(rc, errors, what)		\
	if (rc == XN_STATUS_NO_NODE_PRESENT)	\
{										\
	XnChar strError[1024];				\
	errors.ToString(strError, 1024);	\
	printf("%s\n", strError);			\
	return (rc);						\
}

void XN_CALLBACK_TYPE GestureIntermediateStageCompletedHandler(xn::GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pPosition, void* pCookie)
{
//	printf("Gesture %s: Intermediate stage complete (%f,%f,%f)\n", strGesture, pPosition->X, pPosition->Y, pPosition->Z);
}
void XN_CALLBACK_TYPE GestureReadyForNextIntermediateStageHandler(xn::GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pPosition, void* pCookie)
{
//	printf("Gesture %s: Ready for next intermediate stage (%f,%f,%f)\n", strGesture, pPosition->X, pPosition->Y, pPosition->Z);
}
void XN_CALLBACK_TYPE GestureProgressHandler(xn::GestureGenerator& generator, const XnChar* strGesture, const XnPoint3D* pPosition, XnFloat fProgress, void* pCookie)
{
//	printf("Gesture %s progress: %f (%f,%f,%f)\n", strGesture, fProgress, pPosition->X, pPosition->Y, pPosition->Z);
}
// callback for session start
void XN_CALLBACK_TYPE SessionStarting(const XnPoint3D& ptPosition, void* UserCxt)
{
	printf("Session start: (%f,%f,%f)\n", ptPosition.X, ptPosition.Y, ptPosition.Z);
//	g_SessionState = IN_SESSION;
}
// Callback for session end
void XN_CALLBACK_TYPE SessionEnding(void* UserCxt)
{
	printf("Session end\n");
//	g_SessionState = NOT_IN_SESSION;
}
void XN_CALLBACK_TYPE NoHands(void* UserCxt)
{
//	if (g_SessionState != NOT_IN_SESSION)
//	{
//		printf("Quick refocus\n");
//		g_SessionState = QUICK_REFOCUS;
//	}
}
// Callback for when the focus is in progress
void XN_CALLBACK_TYPE FocusProgress(const XnChar* strFocus, const XnPoint3D& ptPosition, XnFloat fProgress, void* UserCxt)
{
	printf("Focus progress: %s @(%f,%f,%f): %f\n", strFocus, ptPosition.X, ptPosition.Y, ptPosition.Z, fProgress);
}

void XN_CALLBACK_TYPE Swipe_SwipeUp(XnFloat fVelocity, XnFloat fAngle, void* cxt)
{
	printf("SWIPED Up!\n");

}

void XN_CALLBACK_TYPE Swipe_SwipeLeft(XnFloat fVelocity, XnFloat fAngle, void* cxt)
{
	if(current_slide_index > 1){
		printf("SWIPED Left\n");
		current_slide_index--;
		current_slide = getSlide(current_slide_index);
	}
}

void XN_CALLBACK_TYPE Swipe_SwipeRight(XnFloat fVelocity, XnFloat fAngle, void* cxt)
{
	if(current_slide_index < 10){
		printf("SWIPED Right\n");
		current_slide_index++;
		current_slide = getSlide(current_slide_index);
	}
}

void XN_CALLBACK_TYPE Swipe_SwipeDown(XnFloat fVelocity, XnFloat fAngle, void* cxt)
{
	printf("SWIPED Down!\n");
}

int quickSlide;
bool status_circle = false;
void XN_CALLBACK_TYPE CircleCB(XnFloat fTimes, XnBool bConfident, const XnVCircle* pCircle, void* pUserCxt)
{
	glColor4f(1,0,1,1);
	glRasterPos2i(20, 20);
	XnChar strLabel[20];
	int num_slidess = 10;
	quickSlide = (fTimes * 10 / 1);
//	printf("ftimes:: %f\n", fTimes);
	quickSlide = (quickSlide % num_slidess) + 1;

	sprintf(strLabel, "%d", quickSlide);//fmod((double)fTimes, 1.0) * 2 * XnVMathCommon::PI);
	status_circle = true;
	glPrintString(GLUT_BITMAP_HELVETICA_18, strLabel);
}

void XN_CALLBACK_TYPE NoCircleCB(XnFloat fLastValue, XnVCircleDetector::XnVNoCircleReason reason, void * pUserCxt)
{
	printf("<<DEBUG>> CIRCLE STOPPED\n");
	current_slide_index = quickSlide;
	current_slide = getSlide(current_slide_index);
	status_circle = false;
}

void XN_CALLBACK_TYPE Circle_PrimaryCreate(const XnVHandPointContext *cxt, const XnPoint3D& ptFocus, void * pUserCxt)
{
//	printf("<<DEBUG>> CIRCLE CREATED!!\n");
}

void XN_CALLBACK_TYPE Circle_PrimaryDestroy(XnUInt32 nID, void * pUserCxt)
{
//	SetVisualFeedbackFrame(true, 0.2, 0.7, 0.2);
}

void XN_CALLBACK_TYPE onPush(XnFloat fVelocity, XnFloat fAngle, void* UserCxt)
{
//	printf("PUSH\n");
	if (status_circle){
		printf("Velocity: %f\n" , fVelocity);
		circleDetector->Reset();
	}
}

void XN_CALLBACK_TYPE onStable(XnFloat fVelocity, void* UserCxt)
{
//	printf("STABLE???\n");
}

extern unsigned char* pDepthTexBuf;
extern XnUserID* aUsers;

#include <XnUSB.h>
#define VID_MICROSOFT 0x45e
#define PID_NUI_MOTOR 0x02b0
XN_USB_DEV_HANDLE dev;

int start(){
	pDepthTexBuf = (unsigned char *)malloc(sizeof(char)*(640*480*4));
	aUsers  = (XnUserID *)malloc(sizeof(XnUserID)*15);

	XnStatus rc = XN_STATUS_OK;
	xn::EnumerationErrors errors;

	rc = g_Context.InitFromXmlFile(SAMPLE_XML_PATH, g_ScriptNode, &errors);
	CHECK_ERRORS(rc, errors, "InitFromXmlFile");
	CHECK_RC(rc, "InitFromXml");

	rc = g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
	CHECK_RC(rc, "Find depth generator");
	rc = g_Context.FindExistingNode(XN_NODE_TYPE_IMAGE, g_ImageGenerator);
	CHECK_RC(rc, "Find image generator");
	rc = g_Context.FindExistingNode(XN_NODE_TYPE_USER, g_UserGenerator);
	CHECK_RC(rc, "Find user generator");
	rc = g_Context.FindExistingNode(XN_NODE_TYPE_HANDS, g_HandsGenerator);
	CHECK_RC(rc, "Find hands generator");
	rc = g_Context.FindExistingNode(XN_NODE_TYPE_GESTURE, g_GestureGenerator);
	CHECK_RC(rc, "Find gesture generator");

	XnCallbackHandle hGestureIntermediateStageCompleted, hGestureProgress, hGestureReadyForNextIntermediateStage;
	g_GestureGenerator.RegisterToGestureIntermediateStageCompleted(GestureIntermediateStageCompletedHandler, NULL, hGestureIntermediateStageCompleted);
	g_GestureGenerator.RegisterToGestureReadyForNextIntermediateStage(GestureReadyForNextIntermediateStageHandler, NULL, hGestureReadyForNextIntermediateStage);
	g_GestureGenerator.RegisterGestureCallbacks(NULL, GestureProgressHandler, NULL, hGestureProgress);

	g_pSessionManager = new XnVSessionManager;
	rc = g_pSessionManager->Initialize(&g_Context, "Click,Wave", "RaiseHand");
	CHECK_RC(rc, "SessionManager::Initialize");

	g_pSessionManager->RegisterSession(NULL, SessionStarting, SessionEnding, FocusProgress);

	if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON) ||
			!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
	{
		printf("User generator doesn't support either skeleton or pose detection.\n");
		return XN_STATUS_ERROR;
	}

	g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

	broadcaster = new XnVBroadcaster;

	//SwipeDetector
	swipeDetector = new XnVSwipeDetector();
	swipeDetector->RegisterSwipeUp(NULL, &Swipe_SwipeUp);
	swipeDetector->RegisterSwipeDown(NULL, &Swipe_SwipeDown);
	swipeDetector->RegisterSwipeLeft(NULL, &Swipe_SwipeLeft);
	swipeDetector->RegisterSwipeRight(NULL, &Swipe_SwipeRight);
	broadcaster->AddListener(swipeDetector);

<<<<<<< HEAD
	g_pDrawer = new XnVPointDrawer(20, g_DepthGenerator);
	broadcaster->AddListener(g_pDrawer);
	g_pSessionManager->AddListener(broadcaster);
=======
	//CircleDetector
	circleDetector = new XnVCircleDetector();
	circleDetector->RegisterCircle(NULL, &CircleCB);
	circleDetector->RegisterNoCircle(NULL, &NoCircleCB);
	circleDetector->RegisterPrimaryPointCreate(NULL, &Circle_PrimaryCreate);
	circleDetector->RegisterPrimaryPointDestroy(NULL, &Circle_PrimaryDestroy);
	broadcaster->AddListener(circleDetector);

	//PushDetector
	pushDetector = new XnVPushDetector();
	pushDetector->RegisterPush(NULL, &onPush);
	pushDetector->RegisterStabilized(NULL, &onStable);
	broadcaster->AddListener(pushDetector);
>>>>>>> b5cfdb0b4319faf6056455e9fc73ab508850f729

	g_pSessionManager->AddListener(broadcaster);
	rc = g_Context.StartGeneratingAll();
	CHECK_RC(rc, "StartGenerating");

	XnCallbackHandle hUserCBs, hCalibrationStartCB, hCalibrationCompleteCB, hPoseCBs;
	g_UserGenerator.RegisterUserCallbacks(NewUser, LostUser, NULL, hUserCBs);
	rc = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(CalibrationStarted, NULL, hCalibrationStartCB);
	CHECK_RC(rc, "Register to calibration start");
	rc = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(CalibrationCompleted, NULL, hCalibrationCompleteCB);
	CHECK_RC(rc, "Register to calibration complete");
	rc = g_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(PoseDetected, NULL, hPoseCBs);
	CHECK_RC(rc, "Register to pose detected");

	motorAngle(6);

}

void motorAngle(int angle){
	printf("%i\n",angle);
	xnUSBInit();
	xnUSBOpenDevice(VID_MICROSOFT, PID_NUI_MOTOR, NULL, NULL, &dev);
	uint8_t empty[0x1];
	angle = angle * 2;
	xnUSBSendControl(dev,
				  	  XN_USB_CONTROL_TYPE_VENDOR,
					  0x31,
					  (XnUInt16)angle,
					  0x0,
					  empty,
					  0x0, 0);
	xnUSBCloseDevice(dev);
}




