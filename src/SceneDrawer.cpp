/*******************************************************************************
*                                                                              *
*   PrimeSense NITE 1.3 - Players Sample                                       *
*   Copyright (C) 2010 PrimeSense Ltd.                                         *
*                                                                              *
*******************************************************************************/

#include "SceneDrawer.h"
#include <GL/gl.h>
#include <GL/glut.h>

extern xn::UserGenerator g_UserGenerator;
extern xn::DepthGenerator g_DepthGenerator;
extern int LEFT, RIGHT, TOP, BOTTOM;
extern float CENTER;

#define MAX_DEPTH 10000
float g_pDepthHist[MAX_DEPTH];


XnFloat Colors[][3] =
{
	{0,1,1},
	{0,0,1},
	{0,1,0},
	{1,1,0},
	{1,0,0},
	{1,.5,0},
	{.5,1,0},
	{0,.5,1},
	{.5,0,1},
	{1,1,.5},
	{1,1,1}
};
XnUInt32 nColors = 10;

void glPrintString(void *font, char *str)
{
	size_t i,l = strlen(str);

	for(i=0; i<l; i++)
	{
		glutBitmapCharacter(font,*str++);
	}
}

void DrawLimb(XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2)
{
	if (!g_UserGenerator.GetSkeletonCap().IsCalibrated(player))
	{
		printf("not calibrated!\n");
		return;
	}
	if (!g_UserGenerator.GetSkeletonCap().IsTracking(player))
	{
		printf("not tracked!\n");
		return;
	}

	XnSkeletonJointPosition joint1, joint2;
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint1, joint1);
	g_UserGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint2, joint2);

	if (joint1.fConfidence < 0.75 || joint2.fConfidence < 0.75)
	{
		return;
	}

	XnPoint3D pt[2];
	pt[0] = joint1.position;
	pt[1] = joint2.position;

	g_DepthGenerator.ConvertRealWorldToProjective(2, pt, pt);
	glVertex3i(pt[0].X, pt[0].Y, 0);
	glVertex3i(pt[1].X, pt[1].Y, 0);
}

unsigned char* pDepthTexBuf;
static int texWidth, texHeight;
unsigned int nValue, nIndex, nX, nY, nNumberOfPoints;
XnUInt16 g_nXRes, g_nYRes;
unsigned char* pDestImage;
const XnDepthPixel* pDepth;
const XnRGB24Pixel* pixel;
const XnLabel* pLabels;
XnLabel label;
XnUserID* aUsers;
XnUInt16 nUsers;
XnPoint3D com;

void DrawDepthMap(const xn::DepthMetaData& dmd, const xn::SceneMetaData& smd, XnUserID player, xn::ImageMetaData& imd)
{

	texWidth = 640;
	texHeight = 480;


	LEFT = 0; RIGHT = 640;
	TOP = 0; BOTTOM = 480;

	nValue = 0;
	nIndex = 0;
	nX = 0; nY = 0;
	nNumberOfPoints = 0;
	g_nXRes = dmd.XRes();
	g_nYRes = dmd.YRes();

	pDestImage = pDepthTexBuf;

	pDepth = dmd.Data();
	pixel = imd.RGB24Data();
	pLabels = smd.Data();

	// Calculate the accumulative histogram
	memset(g_pDepthHist, 0, MAX_DEPTH*sizeof(float));
	for (nY=0; nY<g_nYRes; nY++)
	{
		for (nX=0; nX<g_nXRes; nX++)
		{
			nValue = *pDepth;

			if (nValue != 0)
			{
				g_pDepthHist[nValue]++;
				nNumberOfPoints++;
			}

			pDepth++;
		}
	}

	for (nIndex=1; nIndex<MAX_DEPTH; nIndex++)
	{
		g_pDepthHist[nIndex] += g_pDepthHist[nIndex-1];
	}
	if (nNumberOfPoints)
	{
		for (nIndex=1; nIndex<MAX_DEPTH; nIndex++)
		{
			g_pDepthHist[nIndex] = (unsigned int)(256 * (1.0f - (g_pDepthHist[nIndex] / nNumberOfPoints)));
		}
	}

	pDepth = (short unsigned int*)dmd.Data();
	///{
		// Prepare the texture map
		for (nY=0; nY<g_nYRes; nY++)
		{
			for (nX=0; nX < g_nXRes; nX++)
			{

				nValue = *pDepth;
				label = *pLabels;
//				XnUInt32 nColorID = label % nColors;
				if (label == 0)
				{
//					nColorID = nColors;
					pDestImage[0] = 0;
					pDestImage[1] = 0;
					pDestImage[2] = 0;
//					pDestImage[0] = pixel->nRed;
//					pDestImage[1] = pixel->nGreen;
//					pDestImage[2] = pixel->nBlue;
					pDestImage[3] = 0;
				} else {
					pDestImage[0] = pixel->nRed;
					pDestImage[1] = pixel->nGreen;
					pDestImage[2] = pixel->nBlue;
					pDestImage[3] = 255;

					//find max/min values for width and height boundaries
					if (nX > (unsigned int)LEFT) {
						LEFT = nX;
					}

					if (nX < (unsigned int)RIGHT) {
						RIGHT = nX;
					}

					if (nY > (unsigned int)TOP) {
						TOP = nY;
					}

					if (nY < (unsigned int)BOTTOM) {
						BOTTOM = nY;
					}
				}

				pixel++;
				pDepth++;
				pLabels++;
				pDestImage+=4;
			}

			pDestImage += (texWidth - g_nXRes) *4;
		}
	//}*/

	// Display the OpenGL texture map
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pDepthTexBuf);

	nUsers = 15;
	g_UserGenerator.GetUsers(aUsers, nUsers);
	g_UserGenerator.GetCoM(aUsers[0], com);


	//CENTER = com.X;

	//glDisable(GL_BLEND);
	//glDisable(GL_TEXTURE_2D);
/*
	char strLabel[20] = "";
	XnUserID aUsers[15];
	XnUInt16 nUsers = 15;
	g_UserGenerator.GetUsers(aUsers, nUsers);
	for (int i = 0; i < nUsers; ++i)
	{
		XnPoint3D com;
		g_UserGenerator.GetCoM(aUsers[i], com);
		g_DepthGenerator.ConvertRealWorldToProjective(1, &com, &com);

		if (aUsers[i] == player)
			sprintf(strLabel, "%d (Player)", aUsers[i]);
		else
			sprintf(strLabel, "%d", aUsers[i]);

		//glColor4f(1-Colors[i%nColors][0], 1-Colors[i%nColors][1], 1-Colors[i%nColors][2], 1);

		glRasterPos2i(com.X, com.Y);
		glPrintString(GLUT_BITMAP_HELVETICA_18, strLabel); */
//	}

	// Draw skeleton of user
//	if (player != 0)
//	{
//		glBegin(GL_LINES);
		//glColor4f(1-Colors[player%nColors][0], 1-Colors[player%nColors][1], 1-Colors[player%nColors][2], 1);
		/*DrawLimb(player, XN_SKEL_HEAD, XN_SKEL_NECK);

		DrawLimb(player, XN_SKEL_NECK, XN_SKEL_LEFT_SHOULDER);
		DrawLimb(player, XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW);
		DrawLimb(player, XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND);

		DrawLimb(player, XN_SKEL_NECK, XN_SKEL_RIGHT_SHOULDER);
		DrawLimb(player, XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW);
		DrawLimb(player, XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND);

		DrawLimb(player, XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO);
		DrawLimb(player, XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO);

		DrawLimb(player, XN_SKEL_TORSO, XN_SKEL_LEFT_HIP);
		DrawLimb(player, XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE);
		DrawLimb(player, XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT);

		DrawLimb(player, XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP);
		DrawLimb(player, XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE);
		DrawLimb(player, XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT);*/
//		glEnd();
//	}
}
