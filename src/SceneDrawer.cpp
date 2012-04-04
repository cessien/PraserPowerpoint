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

float *pos;

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
	printf("Points: %f, %f, %f \n");
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

	printf("ID: %i\n",player);
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
				if (label != 1)
				{
//					nColorID = nColors;
					pDestImage[0] = 0;
					pDestImage[1] = 0;
					pDestImage[2] = 0;
//					pDestImage[0] = pixel->nRed;
//					pDestImage[1] = pixel->nGreen;
//					pDestImage[2] = pixel->nBlue;
					pDestImage[3] = 1;
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


	CENTER = com.X;

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
//		glBegin(GL_LINES);
//		//glColor4f(1,0,0,1.0f);
//		glVertex3f(-1, 1, 0);
//		glVertex3f(1, -1, 0);
//		//glColor4f(1-Colors[player%nColors][0], 1-Colors[player%nColors][1], 1-Colors[player%nColors][2], 1);
//		DrawLimb(1, XN_SKEL_HEAD, XN_SKEL_NECK);
//
//		DrawLimb(1, XN_SKEL_NECK, XN_SKEL_LEFT_SHOULDER);
//		DrawLimb(1, XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW);
//		DrawLimb(1, XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND);
//
//		DrawLimb(1, XN_SKEL_NECK, XN_SKEL_RIGHT_SHOULDER);
//		DrawLimb(1, XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW);
//		DrawLimb(1, XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND);
//
//		DrawLimb(1, XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO);
//		DrawLimb(1, XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO);
//
//		DrawLimb(1, XN_SKEL_TORSO, XN_SKEL_LEFT_HIP);
//		DrawLimb(1, XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE);
//		DrawLimb(1, XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT);
//
//		DrawLimb(1, XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP);
//		DrawLimb(1, XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE);
//		DrawLimb(1, XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT);
//		glEnd();

	//XnSkeletonJoint eJoint1 = XN_SKEL_RIGHT_HAND;

}

#include "XnVDepthMessage.h"
#include <XnVHandPointContext.h>
// Constructor. Receives the number of previous positions to store per hand,
// and a source for depth map
XnVPointDrawer::XnVPointDrawer(XnUInt32 nHistory, xn::DepthGenerator depthGenerator) :
	XnVPointControl("XnVPointDrawer"),
	m_nHistorySize(nHistory), m_DepthGenerator(depthGenerator), m_bDrawDM(false), m_bFrameID(false)
{
	m_pfPositionBuffer = new XnFloat[nHistory*3];
	pos = (float *)malloc(sizeof(float)*3);
}

// Destructor. Clear all data structures
XnVPointDrawer::~XnVPointDrawer()
{
	std::map<XnUInt32, std::list<XnPoint3D> >::iterator iter;
	for (iter = m_History.begin(); iter != m_History.end(); ++iter)
	{
		iter->second.clear();
	}
	m_History.clear();

	delete []m_pfPositionBuffer;
}

// Change whether or not to draw the depth map
void XnVPointDrawer::SetDepthMap(XnBool bDrawDM)
{
	m_bDrawDM = bDrawDM;
}
// Change whether or not to print the frame ID
void XnVPointDrawer::SetFrameID(XnBool bFrameID)
{
	m_bFrameID = bFrameID;
}

// Handle creation of a new hand
static XnBool bShouldPrint = false;
void XnVPointDrawer::OnPointCreate(const XnVHandPointContext* cxt)
{
	printf("** %d\n", cxt->nID);
	// Create entry for the hand
	m_History[cxt->nID].clear();
	bShouldPrint = true;
	OnPointUpdate(cxt);
	bShouldPrint = true;
}
// Handle new position of an existing hand
void XnVPointDrawer::OnPointUpdate(const XnVHandPointContext* cxt)
{
	// positions are kept in projective coordinates, since they are only used for drawing
	XnPoint3D ptProjective(cxt->ptPosition);

	if (bShouldPrint)printf("Point (%f,%f,%f)", ptProjective.X, ptProjective.Y, ptProjective.Z);
	m_DepthGenerator.ConvertRealWorldToProjective(1, &ptProjective, &ptProjective);
	//if (true)printf(" -> (%f,%f,%f)\n", ptProjective.X, ptProjective.Y, ptProjective.Z);

	// Add new position to the history buffer
	m_History[cxt->nID].push_front(ptProjective);
	// Keep size of history buffer
	if (m_History[cxt->nID].size() > m_nHistorySize)
		m_History[cxt->nID].pop_back();
	bShouldPrint = false;

	pos[0] = ptProjective.X;
	pos[1] = ptProjective.Y;
	pos[2] = 0;//ptProjective.Z;
}

// Handle destruction of an existing hand
void XnVPointDrawer::OnPointDestroy(XnUInt32 nID)
{
	// No need for the history buffer
	m_History.erase(nID);
}


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

	//glFlush();
}
void DrawTexture(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY)
{
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

	DrawRectangle(topLeftX, topLeftY, bottomRightX, bottomRightY);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void DrawFrameID(XnUInt32 nFrameID)
{
	glColor4f(1,0,0,1);
	glRasterPos2i(20, 50);
	XnChar strLabel[20];
	sprintf(strLabel, "%d", nFrameID);
	glPrintString(GLUT_BITMAP_HELVETICA_18, strLabel);
}

XnBool XnVPointDrawer::IsTouching(XnUInt32 id) const
{
	for (std::list<XnUInt32>::const_iterator iter = m_TouchingFOVEdge.begin(); iter != m_TouchingFOVEdge.end(); ++iter)
	{
		if (*iter == id)
			return TRUE;
	}
	return FALSE;
}

void XnVPointDrawer::Draw() const
{
	std::map<XnUInt32, std::list<XnPoint3D> >::const_iterator PointIterator;

	// Go over each existing hand
	for (PointIterator = m_History.begin();
		PointIterator != m_History.end();
		++PointIterator)
	{
		// Clear buffer
		XnUInt32 nPoints = 0;
		XnUInt32 i = 0;
		XnUInt32 Id = PointIterator->first;

		// Go over all previous positions of current hand
		std::list<XnPoint3D>::const_iterator PositionIterator;
		for (PositionIterator = PointIterator->second.begin();
			PositionIterator != PointIterator->second.end();
			++PositionIterator, ++i)
		{
			// Add position to buffer
			XnPoint3D pt(*PositionIterator);
			m_pfPositionBuffer[3*i] = pt.X;
			m_pfPositionBuffer[3*i + 1] = pt.Y;
			m_pfPositionBuffer[3*i + 2] = 0;//pt.Z();
		}

		//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		// Set color
		XnUInt32 nColor = Id % nColors;
		XnUInt32 nSingle = GetPrimaryID();
		if (Id == GetPrimaryID())
			nColor = 6;
		// Draw buffer:
//		glColor4f(1,
//				0,
//				0,
//				1.0f);
		glPointSize(2);
		glVertexPointer(3, GL_FLOAT, 0, m_pfPositionBuffer);
		glDrawArrays(GL_LINE_STRIP, 0, i);


//		if (IsTouching(Id))
//		{
//			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
//		}
		glPointSize(8);
		glDrawArrays(GL_POINTS, 0, 1);
		glFlush();
	}
}
void XnVPointDrawer::SetTouchingFOVEdge(XnUInt32 nID)
{
	m_TouchingFOVEdge.push_front(nID);
}

// Handle a new Message
void XnVPointDrawer::Update(XnVMessage* pMessage)
{
	// PointControl's Update calls all callbacks for each hand
	XnVPointControl::Update(pMessage);

	if (m_bDrawDM)
	{
		// Draw depth map
		xn::DepthMetaData depthMD;
		m_DepthGenerator.GetMetaData(depthMD);
		//DrawDepthMap(depthMD);
	}

	if (m_bFrameID)
	{
		// Print out frame ID
		xn::DepthMetaData depthMD;
		m_DepthGenerator.GetMetaData(depthMD);
		DrawFrameID(depthMD.FrameID());
	}
	// Draw hands
	//printf("DRAWING\n");
	Draw();
	m_TouchingFOVEdge.clear();
}

void PrintSessionState(SessionState eState)
{
	glColor4f(1,0,1,1);
	glRasterPos2i(20, 20);
	XnChar strLabel[200];

	switch (eState)
	{
	case 0:
		sprintf(strLabel, "Tracking hands"); break;
	case 1:
		sprintf(strLabel, "Perform click or wave gestures to track hand"); break;
	case 2:
		sprintf(strLabel, "Raise your hand for it to be identified, or perform click or wave gestures"); break;
	}

	glPrintString(GLUT_BITMAP_HELVETICA_18, strLabel);
}
