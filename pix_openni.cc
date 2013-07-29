////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::für::umläute. IEM
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

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


#include "pix_openni.h"

FrameListener :: FrameListener() : m_newFrame(0){}

void FrameListener :: onNewFrame(VideoStream& stream)
{
  m_newFrame++;
}

void FrameListener :: unSet()
{
  m_newFrame--;
}

CPPEXTERN_NEW_WITH_GIMME(pix_openni);

// Constructor
pix_openni :: pix_openni(int argc, t_atom *argv) : m_deviceURI(ANY_DEVICE), \
                                                   m_rgb(0), \
                                                   m_ir(0), \
                                                   m_depth(0), \
                                                   m_rgbListener(NULL), \
                                                   m_depthListener(NULL)
{
   
   	// second inlet/outlet for depthmap
	m_depthoutlet = outlet_new(this->x_obj, 0);
	m_depthinlet  = inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("gem_state"), gensym("depth_state"));

	m_dataout = outlet_new(this->x_obj, 0);
  
  m_rgbListener = new FrameListener;
  m_depthListener = new FrameListener;
   
   // initialize OpenNI
   Status rc = STATUS_OK;
   
   rc = OpenNI::initialize();
   if ( rc != STATUS_OK ){
      error("can't initialized OpenNI : %s", OpenNI::getExtendedError());
#ifdef __UNIX__
      error("this often happens when drivers are not reachable (in /usr/lib or near this external)");
#endif
      throw(GemException("OpenNI Init() failed\n"));
   }
}

// Destructor
pix_openni :: ~pix_openni(){
   OpenNI::shutdown();
}

void pix_openni :: obj_setupCallback(t_class *classPtr)
{
  CPPEXTERN_MSG0(classPtr, "enumerate", enumerateMess);
  CPPEXTERN_MSG (classPtr, "open", openMess);
  CPPEXTERN_MSG0(classPtr, "close", closeMess);
  CPPEXTERN_MSG1(classPtr, "rgb", rgbMess, t_float);
  CPPEXTERN_MSG1(classPtr, "depth", depthMess, t_float);
  CPPEXTERN_MSG1(classPtr, "ir", irMess, t_float);
  CPPEXTERN_MSG (classPtr, "depth_state", renderDepth);
}

void pix_openni :: enumerateMess(){
   
   Array<openni::DeviceInfo> deviceList;
   OpenNI::enumerateDevices(&deviceList);
   
   t_atom a_devices;
   SETFLOAT(&a_devices, deviceList.getSize());
   outlet_anything(m_dataout, gensym("devices"), 1, &a_devices);
   
   for(int i=0; i<deviceList.getSize(); i++){
      t_atom a_deviceInfo[5];
      SETSYMBOL(a_deviceInfo, gensym(deviceList[i].getUri()));
      char hexID[7];
      sprintf(hexID, "0x%04x", deviceList[i].getUsbVendorId());
      SETSYMBOL(a_deviceInfo+1, gensym(hexID));
      sprintf(hexID, "0x%04x", deviceList[i].getUsbProductId());
      SETSYMBOL(a_deviceInfo+2, gensym(hexID));
      SETSYMBOL(a_deviceInfo+3, gensym(deviceList[i].getVendor()));
      SETSYMBOL(a_deviceInfo+4, gensym(deviceList[i].getName()));
      outlet_anything(m_dataout, gensym("device"), 5, a_deviceInfo);
   }
}

// enable/disable rgb grabbing
void pix_openni :: rgbMess(t_float f){
  if ( !m_device.isValid() ){
    error("please open a device before...");
    return;
  }

  if ( f > 0 ){
    m_rgb = 1;
    if ( !m_device.hasSensor(SENSOR_COLOR) ){
      m_rgb = 0;
      error("device %s has no rgb sensor", m_deviceURI);
    }
  } else {
    m_rgb = 0;
  }
  t_atom a_rgb;
  SETFLOAT(&a_rgb, m_rgb);
  outlet_anything(m_dataout, gensym("rgb"), 1, &a_rgb);
}

// enable/diasable ir grabbing
void pix_openni :: irMess(t_float f){
  if ( !m_device.isValid() ){
    error("please open a device before...");
    return;
  }
  if ( f > 0 ){
    m_rgb = 1;
    if ( !m_device.hasSensor(SENSOR_IR) ){
      m_rgb = 0;
      error("device %s has no rgb sensor", m_deviceURI);
    }
  } else {
    m_rgb = 0;
  }
  t_atom a_ir;
  SETFLOAT(&a_ir, m_ir);
  outlet_anything(m_dataout, gensym("ir"), 1, &a_ir);
}

// enable/disable depth grabbing
void pix_openni :: depthMess(t_float f){
  if ( !m_device.isValid() ){
    error("please open a device before...");
    return;
  }
  if ( f > 0 ){
    m_rgb = 1;
    if ( !m_device.hasSensor(SENSOR_DEPTH) ){
      m_rgb = 0;
      error("device %s has no rgb sensor", m_deviceURI);
    }
  } else {
    m_rgb = 0;
  }
  t_atom a_depth;
  SETFLOAT(&a_depth, m_depth);
  outlet_anything(m_dataout, gensym("depth"), 1, &a_depth);
}

// Open a device/file by its URI or (for device only) by its vendor:device unique ID
// one argument : symbol (URI)
// two argument : vendor id, model id
void pix_openni :: openMess(t_symbol *s,int argc, t_atom*argv){   
  if (argc>0){
    if (argv->a_type == A_SYMBOL){
      m_deviceURI=atom_getsymbol(argv)->s_name;
    }
  } else {
    m_deviceURI=ANY_DEVICE;
  }
  
  Status rc = STATUS_OK;
  rc = m_device.open(m_deviceURI);
  t_atom a_status;
  SETFLOAT(&a_status, rc);
  outlet_anything(m_dataout, gensym("open"), 1, &a_status);
  if ( rc != STATUS_OK ){
    error("can't open device %s : %s", m_deviceURI, OpenNI::getExtendedError());
    return;
  }
}

// Close the device/file
void pix_openni :: closeMess(){
  m_device.close();
}

void pix_openni :: render(GemState *state){
  if ( m_rgbListener->m_newFrame ){
    m_depthListener->unSet();
    
    Status rc = STATUS_OK;
    
    rc = m_rgbStream.readFrame(&m_rgbFrame);
    if ( rc != STATUS_OK ){
      error("error when reading rgb frame : %s\n",OpenNI::getExtendedError());
      return;
    }
    
    VideoMode vmode = m_rgbFrame.getVideoMode();
    
    if ( m_rgbPix.image.xsize != vmode.getResolutionX() || \
         m_rgbPix.image.ysize != vmode.getResolutionY() || \
         m_rgbPixFormat != vmode.getPixelFormat() )
    {
      m_rgbPix.image.xsize = vmode.getResolutionX();
      m_rgbPix.image.ysize = vmode.getResolutionY();
      m_rgbPixFormat = vmode.getPixelFormat();
      
      if ( m_rgbPixFormat == PIXEL_FORMAT_GRAY8){
        m_rgbPix.image.csize = 1;
      } else {
        m_rgbPix.image.csize = 4;
      }
      
      m_rgbPix.image.reallocate();
      state->set(GemState::_PIX, &m_rgbPix);
    }
    
    m_rgbPix.newimage=true;
    
    unsigned char *data = (unsigned char *) m_rgbFrame.getData();
    unsigned char *pixels=m_rgbPix.image.data;
    int size = m_rgbPix.image.xsize * m_rgbPix.image.ysize;
    
    int step;
    switch ( vmode.getPixelFormat() ){
      case PIXEL_FORMAT_RGB888:
        step = 3;
        break;
      case PIXEL_FORMAT_GRAY8:
        step = 1;
        break;
      case PIXEL_FORMAT_GRAY16:
        step = 2;
        break;
      default:
        step = 1;
      }
      
      // TODO a loop to write data to m_rgbPix
    
    //~m_rgbPix.image.x_size;

    // do something with the new frame
  } else {
    
    m_rgbPix.newimage=false;
    // do nothing
  }
}

void pix_openni :: renderDepth(t_symbol *s,int argc, t_atom*argv){
  if ( m_depthListener->m_newFrame ){
    m_depthListener->unSet();
    // do something with the new frame
  } else {
    // do nothing
  }
}

void pix_openni :: startRendering(){
  if (m_rgb){
    Status rc = STATUS_OK;
    rc = m_rgbStream.create(m_device, SENSOR_COLOR);
    //~t_atom a_status;
    
    if ( rc != STATUS_OK ){
      error("can't create rgb stream : %s", OpenNI::getExtendedError());
      //~SETFLOAT(&a_status, rc);
      //~outlet_anything(m_dataout, gensym("rgb"), 1, &a_status);
      return;
    }
    
    rc = m_rgbStream.start();
    if ( rc != STATUS_OK ){
      error("can't start rgb stream : %s", OpenNI::getExtendedError());
      //~SETFLOAT(&a_status, rc);
      //~outlet_anything(m_dataout, gensym("rgb"), 1, &a_status);
      return;
    }
    m_rgbStream.addNewFrameListener(m_rgbListener);
  }
}

void pix_openni :: stopRendering(){
}
