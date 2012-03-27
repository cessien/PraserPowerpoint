/*******************************************************************************
*                                                                              *
*   PrimeSense NITE 1.3 - Players Sample                                       *
*   Copyright (C) 2010 PrimeSense Ltd.                                         *
*                                                                              *
*******************************************************************************/

#ifndef XNV_POINT_DRAWER_H_
#define XNV_POINT_DRAWER_H_

#include <XnCppWrapper.h>
#include <GL/gl.h>
#include <GL/glut.h>

void DrawDepthMap(const xn::DepthMetaData& dmd, const xn::SceneMetaData& smd, XnUserID player, xn::ImageMetaData &imd);
//void DrawDepthMap(const xn::DepthMetaData& dmd, const xn::SceneMetaData& smd, XnUserID player);
unsigned char* raw_texture_load(const char *filename, int width, int height);
#endif
