/*******************************************************************************
*                                                                              *
*   PrimeSense NITE 1.3 - Players Sample                                       *
*   Copyright (C) 2010 PrimeSense Ltd.                                         *
*                                                                              *
*******************************************************************************/
#include <XnCppWrapper.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <map>
#include <list>
#include <XnCppWrapper.h>
#include <XnVPointControl.h>


void glPrintString(void *font, char *str);
void DrawLimb(XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2);
void DrawDepthMap(const xn::DepthMetaData& dmd, const xn::SceneMetaData& smd, XnUserID player, xn::ImageMetaData& imd);

//	void XnVPointDrawer::SetDepthMap(XnBool bDrawDM){};
//	void XnVPointDrawer::SetFrameID(XnBool bFrameID){};
//	void XnVPointDrawer::OnPointCreate(const XnVHandPointContext* cxt){};
//	void XnVPointDrawer::OnPointUpdate(const XnVHandPointContext* cxt){};
//	void XnVPointDrawer::OnPointDestroy(XnUInt32 nID){};
unsigned int getClosestPowerOfTwo(unsigned int n);
GLuint initTexture(void** buf, int& width, int& height);
void DrawRectangle(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY);
void DrawTexture(float topLeftX, float topLeftY, float bottomRightX, float bottomRightY);
void DrawFrameID(XnUInt32 nFrameID);
//	XnBool XnVPointDrawer::IsTouching(XnUInt32 id) const{};
//	void XnVPointDrawer::Draw() const{};
//	void XnVPointDrawer::SetTouchingFOVEdge(XnUInt32 nID){};
//	void XnVPointDrawer::Update(XnVMessage* pMessage){};



void DrawDepthMap(const xn::DepthMetaData& dmd, const xn::SceneMetaData& smd, XnUserID player, xn::ImageMetaData &imd);
//void DrawDepthMap(const xn::DepthMetaData& dmd, const xn::SceneMetaData& smd, XnUserID player);
unsigned char* raw_texture_load(const char *filename, int width, int height);
typedef enum
{
	IN_SESSION,
	NOT_IN_SESSION,
	QUICK_REFOCUS
} SessionState;
void PrintSessionState(SessionState eState);
/**
 * This is a point control, which stores the history of every point
 * It can draw all the points as well as the depth map.
 */
class XnVPointDrawer : public XnVPointControl
{
public:
	XnVPointDrawer(XnUInt32 nHistorySize, xn::DepthGenerator depthGenerator);
	virtual ~XnVPointDrawer();

	/**
	 * Handle a new message.
	 * Calls other callbacks for each point, then draw the depth map (if needed) and the points
	 */
	void Update(XnVMessage* pMessage);

	/**
	 * Handle creation of a new point
	 */
	void OnPointCreate(const XnVHandPointContext* cxt);
	/**
	 * Handle new position of an existing point
	 */
	void OnPointUpdate(const XnVHandPointContext* cxt);
	/**
	 * Handle destruction of an existing point
	 */
	void OnPointDestroy(XnUInt32 nID);

	/**
	 * Draw the points, each with its own color.
	 */
	void Draw() const;

	/**
	 * Change mode - should draw the depth map?
	 */
	void SetDepthMap(XnBool bDrawDM);
	/**
	 * Change mode - print out the frame id
	 */
	void SetFrameID(XnBool bFrameID);

	void SetTouchingFOVEdge(XnUInt32 nID);


protected:
	XnBool IsTouching(XnUInt32 nID) const;
	// Number of previous position to store for each hand
	XnUInt32 m_nHistorySize;
	// previous positions per hand
	std::map<XnUInt32, std::list<XnPoint3D> > m_History;
	std::list<XnUInt32> m_TouchingFOVEdge;
	// Source of the depth map
	xn::DepthGenerator m_DepthGenerator;
	XnFloat* m_pfPositionBuffer;

	XnBool m_bDrawDM;
	XnBool m_bFrameID;
};
