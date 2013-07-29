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



//~#include <stdio.h>
//~#include <stdlib.h>
//~#include <string.h>
//#include <assert.h>
//~#include <iomanip>
//#include <cmath>
//~#include <vector>
//~#include <stdint.h>

#include <OpenNI.h>

#include "Base/GemBase.h"
#include "Gem/Exception.h"
//~#include "Gem/Properties.h"
#include "Gem/State.h"
//~#include "Gem/Image.h"
#include "Base/GemPixObj.h"
#include "RTE/MessageCallbacks.h"

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
  
  FrameListener *m_rgbListener, *m_depthListener;
  VideoFrameRef m_rgbFrame, m_depthFrame;
  
  //~ pix_block where the pixels are
  pixBlock m_rgbPix, m_depthPix;
  
  VideoStream m_rgbStream, m_depthStream;  
  PixelFormat m_rgbPixFormat, m_depthPixFormat;
  
protected:

  virtual ~pix_openni();

  virtual void	startRendering();
  virtual void  stopRendering();
  virtual void 	render(GemState *state);
  void  renderDepth(t_symbol *s,int argc, t_atom*argv);
  
  void  enumerateMess();
  void  openMess(t_symbol *s,int argc, t_atom*argv);
  void  closeMess();
  void  rgbMess(t_float f);
  void  irMess(t_float f);
  void  depthMess(t_float f);
  
private:
  t_outlet  *m_depthoutlet;
  t_outlet  *m_dataout;
  t_inlet   *m_depthinlet;
}; 

class FrameListener : public VideoStream::NewFrameListener
{
public:
  FrameListener();
  void onNewFrame(VideoStream& stream);
  void unSet();
  unsigned int m_newFrame;
}; 
#endif	// for header file
