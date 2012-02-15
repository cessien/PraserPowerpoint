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

#define MAX_DEPTH 10000
float g_pDepthHist[MAX_DEPTH];

unsigned int getClosestPowerOfTwo(unsigned int n)
{
	unsigned int m = 2;
	while(m < n) m<<=1;

	return m;
}
GLuint initTexture(void** buf, int& width, int& height)
{
	GLuint texID = 0;
	glGenTextures(1,&texID);

	width = getClosestPowerOfTwo(width);
	height = getClosestPowerOfTwo(height); 
	*buf = new unsigned char[width*height*4];
	glBindTexture(GL_TEXTURE_2D,texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return texID;
}

GLfloat texcoords[8];
void DrawRectangle(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY)
{
	GLfloat verts[8] = {	topLeftX, topLeftY,
		topLeftX, bottomRightY,
		bottomRightX, bottomRightY,
		bottomRightX, topLeftY
	};
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	//TODO: Maybe glFinish needed here instead - if there's some bad graphics crap
	glFlush();
}
void DrawTexture(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY)
{
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

	DrawRectangle(topLeftX, topLeftY, bottomRightX, bottomRightY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

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


void DrawDepthMap(const xn::DepthMetaData& dmd, const xn::SceneMetaData& smd, XnUserID player, xn::ImageMetaData& imd, unsigned char *image)
{
	static bool bInitialized = false;
	static GLuint depthTexID, pptID;
	static unsigned char* pDepthTexBuf;
	static int texWidth, texHeight;


	float topLeftX;
	float topLeftY;
	float bottomRightY;
	float bottomRightX;
	float texXpos;
	float texYpos;

	if(!bInitialized)
	{

		texWidth =  getClosestPowerOfTwo(dmd.XRes());
		texHeight = getClosestPowerOfTwo(dmd.YRes());

//		printf("Initializing depth texture: width = %d, height = %d\n", texWidth, texHeight);
		depthTexID = initTexture((void**)&pDepthTexBuf,texWidth, texHeight) ;
//		printf("Initialized depth texture: width = %d, height = %d\n", texWidth, texHeight);
		bInitialized = true;

		topLeftX = dmd.XRes();
		topLeftY = 0;
		bottomRightY = dmd.YRes();
		bottomRightX = 0;
		texXpos =(float)dmd.XRes()/texWidth;
		texYpos  =(float)dmd.YRes()/texHeight;

		memset(texcoords, 0, 8*sizeof(float));
		texcoords[0] = texXpos, texcoords[1] = texYpos, texcoords[2] = texXpos, texcoords[7] = texYpos;

	}
	unsigned int nValue = 0;
	unsigned int nHistValue = 0;
	unsigned int nIndex = 0;
	unsigned int nX = 0;
	unsigned int nY = 0;
	unsigned int nNumberOfPoints = 0;
	XnUInt16 g_nXRes = dmd.XRes();
	XnUInt16 g_nYRes = dmd.YRes();

	unsigned char* pDestImage = pDepthTexBuf;

	const XnDepthPixel* pDepth = dmd.Data();
	const XnRGB24Pixel* pixel = imd.RGB24Data();
	const XnLabel* pLabels = smd.Data();

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

	pDepth = dmd.Data();
	{
		XnUInt32 nIndex = 0;
		// Prepare the texture map
		for (nY=0; nY<g_nYRes; nY++)
		{
			for (nX=0; nX < g_nXRes; nX++, nIndex++)
			{
				nValue = *pDepth;
				XnLabel label = *pLabels;
				XnUInt32 nColorID = label % nColors;
				if (label == 0)
				{
					nColorID = nColors;
					pDestImage[0] = image[0];
					pDestImage[1] = image[1];
					pDestImage[2] = image[2];
//					pDestImage[0] = 0;
//					pDestImage[1] = 0;
//					pDestImage[2] = 0;
					pDestImage[3] = 0;
				} else {
					pDestImage[0] = pixel->nRed;
					pDestImage[1] = pixel->nGreen;
					pDestImage[2] = pixel->nBlue;
					pDestImage[3] = 0;
				}

/*
				if (nValue != 0)
				{
					nHistValue = g_pDepthHist[nValue];

					pDestImage[0] = nHistValue * Colors[nColorID][0];
					pDestImage[1] = nHistValue * Colors[nColorID][1];
					pDestImage[2] = nHistValue * Colors[nColorID][2];
				}
				else
				{
					//pDestImage[0] = 0;
					//pDestImage[1] = 0;
					//pDestImage[2] = 0;
				}
*/

				pixel++;
				pDepth++;
				pLabels++;
				image+=4;
				pDestImage+=4;
			}

			pDestImage += (texWidth - g_nXRes) *4;
		}
	}

	//delete[] image;

	//**************************
	//pptID = raw_texture_load("slide.png",640,480);
	// Display the OpenGL texture map
	glEnable(GL_TEXTURE_2D);
	//glEnable(GL_BLEND);
	//glBindTexture(GL_TEXTURE_2D, pptID);
	//DrawTexture(dmd.XRes(),200,200,dmd.YRes());

	//glColor4f(0.0f,0.0f,1.0f,0.0f);
	glBindTexture(GL_TEXTURE_2D, depthTexID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pDepthTexBuf);

	//glBlendFunc(GL_ONE_MINUS_DST_ALPHA,GL_DST_ALPHA);
	//glBlendFunc (GL_ONE, GL_ONE);
	//DrawTexture(dmd.XRes() - 200,dmd.YRes() - 200,0,0);
	DrawTexture(dmd.XRes(),dmd.YRes(),0,0);
	//glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	// Display the OpenGL texture map
	//glColor4f(0.75,0.75,0.75,1.0f);
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
		glPrintString(GLUT_BITMAP_HELVETICA_18, strLabel);
	}

	// Draw skeleton of user
	if (player != 0)
	{
		glBegin(GL_LINES);
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
		glEnd();
	}
}
