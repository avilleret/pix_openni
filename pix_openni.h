/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Adaptive threshold object

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::für::umläute. IEM. zmoelnig@iem.kug.ac.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/

#ifndef INCLUDE_pix_openni_H_
#define INCLUDE_pix_openni_H_



#include <sstream>
#include <iomanip>

#include <OpenNI.h>
#include <NiTE.h>

#include "Base/GemBase.h"
#include "Gem/Exception.h"
#include "Gem/State.h"
#include "Base/GemPixObj.h"
#include "RTE/MessageCallbacks.h"

#define MAX_USERS 32

#define USER_MESSAGE(msg) \
	{printf("[%08llu] User #%d:\t%s\n",ts, user.getId(),msg);}

using namespace openni;
/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_openni
    

KEYWORDS
    pix
    
DESCRIPTION
* New OpenNI 2.x implementation for pure-data
* based on Matthias Kronlachner's pix_openni
   
-----------------------------------------------------------------*/
class FrameListener;
class DepthChannel;

#ifdef _WIN32
class GEM_EXPORT pix_openni : public GemBase
#else
class GEM_EXTERN pix_openni : public GemBase
#endif
{
  CPPEXTERN_HEADER(pix_openni, GemBase);

public:

 	pix_openni(int argc, t_atom *argv);
  
  Device m_device;
  const char *m_deviceURI;
  
  bool m_rgb, m_ir, m_depth;
  t_float m_confidenceThreshold;
  
  FrameListener *m_frameListener;
  VideoFrameRef m_videoFrameRef;
  
  //~ pix_block where the pixels are
  pixBlock m_pixBlock;
  
  VideoStream m_videoStream;  
  PixelFormat m_pixelFormat;
  
  DepthChannel *m_depthChannel;
  
  nite::UserTracker m_userTracker;
  
  bool m_visibleUsers[MAX_USERS];
  nite::SkeletonState m_skeletonStates[MAX_USERS];
  nite::UserTrackerFrameRef m_userTrackerFrame;

  
protected:

  virtual ~pix_openni();

  virtual void  startRendering();
  virtual void  stopRendering();
  virtual void  render(GemState *state);
  
  void  enumerateMess();
  void  openMess(t_symbol *s,int argc, t_atom*argv);
  void  openBySerialMess(t_symbol *s,int argc, t_atom*argv);
  void  closeMess();
  void  rgbMess(t_float f);
  void  irMess(t_float f);
  void  depthMess(t_float f);
  void  getVideoMode();
  void  setVideoMode(t_symbol *s_videomode);
  
private:

  static void gem_depthMessCallback(void *data, t_symbol *s, int argc, t_atom *argv);
  void updateUserState(const nite::UserData& user, unsigned long long ts);
  
  t_outlet  *m_depthoutlet;
  t_outlet  *m_dataout;
}; 

class GEM_EXTERN DepthChannel : public GemBase
{
  CPPEXTERN_HEADER(DepthChannel, GemBase);
  
public:
  DepthChannel();
  ~DepthChannel();
  
  virtual void	startRendering();
  virtual void  stopRendering();
  virtual void 	render(GemState *state);
  
  Device *m_devicePt;
  bool *m_depthPt;

private:
  
  FrameListener *m_frameListener;
  VideoFrameRef m_videoFrameRef;
  
  //~ pix_block where the pixels are
  pixBlock m_pixBlock;
  
  VideoStream m_videoStream;  
  PixelFormat m_pixelFormat;
  
  t_inlet   *m_depthinlet;
  
};
#endif	// for header file
