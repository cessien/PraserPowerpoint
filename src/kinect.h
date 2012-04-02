/*
 * kinect.h
 *
 *  Created on: Feb 15, 2012
 *      Author: simon
 */

#ifndef KINECT_H_
#define KINECT_H_

#include <XnCppWrapper.h>
#include <GL/gl.h>
#include <GL/glut.h>

float * getBoundary();
int start();
void StopCapture();
void motorAngle(int angle);

void CleanupExit();

void StartCapture();

XnBool AssignPlayer(XnUserID user);
void XN_CALLBACK_TYPE NewUser(xn::UserGenerator& generator, XnUserID user, void* pCookie);
void FindPlayer();
void LostPlayer();
void XN_CALLBACK_TYPE LostUser(xn::UserGenerator& generator, XnUserID user, void* pCookie);
void XN_CALLBACK_TYPE PoseDetected(xn::PoseDetectionCapability& pose, const XnChar* strPose, XnUserID user, void* cxt);
void XN_CALLBACK_TYPE CalibrationStarted(xn::SkeletonCapability& skeleton, XnUserID user, void* cxt);

void XN_CALLBACK_TYPE CalibrationEnded(xn::SkeletonCapability& skeleton, XnUserID user, XnBool bSuccess, void* cxt);

void XN_CALLBACK_TYPE CalibrationCompleted(xn::SkeletonCapability& skeleton, XnUserID user, XnCalibrationStatus eStatus, void* cxt);

void DrawProjectivePoints(XnPoint3D& ptIn, int width, double r, double g, double b);
// this function is called each frame
void kinectDisplay (void);

void glutIdle (void);

void glutKeyboard (unsigned char key, int x, int y);
void glInit (int * pargc, char ** argv);



#endif /* KINECT_H_ */
