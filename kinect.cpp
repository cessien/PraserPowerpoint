/*
 * kinect.cpp
 *
 *  Created on: Dec 1, 2011
 *      Author: Charles Essien
 */

#include <XnOpenNI.h>
#include <XnCodecIDs.h>
#include <XnCppWrapper.h>

#define SAMPLE_XML_PATH "Sample-User.xml"

xn::Context g_Context;
xn::ScriptNode g_ScriptNode;
xn::DepthGenerator g_DepthGenerator;
xn::ImageGenerator g_ImageGenerator;
xn::UserGenerator g_UserGenerator;

void checkErrors(){
	xn::EnumerationErrors errors;

	g_Context.InitFromXmlFile(SAMPLE_XML_PATH, g_ScriptNode, &errors);
	g_Context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_DepthGenerator);
	g_Context.FindExistingNode(XN_NODE_TYPE_IMAGE, g_ImageGenerator);
	g_Context.FindExistingNode(XN_NODE_TYPE_USER, g_UserGenerator);

	XnCallbackHandle hUserCBs, hCalibrationStartCB, hCalibrationCompleteCB, hPoseCBs;
	//g_UserGenerator.RegisterUserCallbacks(NewUser, LostUser, NULL, hUserCBs);
	//g_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(CalibrationStarted, NULL, hCalibrationStartCB);
	//g_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(CalibrationCompleted, NULL, hCalibrationCompleteCB);
	//g_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(PoseDetected, NULL, hPoseCBs);
}


