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


#include "pix_openni2.h"

class FrameListener : public VideoStream::NewFrameListener
{
public:
  FrameListener() : m_newFrame(0){};
  void onNewFrame(VideoStream& stream)
  {
    m_newFrame++;
  }
  void unSet(){
    m_newFrame--;
  }
  unsigned int m_newFrame;
};

CPPEXTERN_NEW(DepthChannel);

DepthChannel :: DepthChannel(){
  m_depthinlet  = inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("gem_state"), gensym("gem_state"));
  m_frameListener = new FrameListener();
}

DepthChannel :: ~DepthChannel(){
  delete(m_frameListener);
}

CPPEXTERN_NEW_WITH_GIMME(pix_openni2);

// Constructor
pix_openni2 :: pix_openni2(int argc, t_atom *argv) : m_deviceURI(ANY_DEVICE), \
                                                   m_rgb(0), \
                                                   m_ir(0), \
                                                   m_connected(false), \
                                                   m_depth(0)
{
   
	//~m_depthinlet  = inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("gem_state"), gensym("depth_state"));
	//~m_depthoutlet = outlet_new(this->x_obj, 0);
  m_depthChannel = new DepthChannel; // create a new intance of DepthChannel wichi inherit from GemBase and thus create a new outlet...
  m_depthChannel->m_devicePt = &m_device;
  m_depthChannel->m_depthPt = &m_depth;

  m_dataout = outlet_new(this->x_obj, 0);
  
  m_frameListener = new FrameListener;
  
  OpenNI::addDeviceConnectedListener(this);
  OpenNI::addDeviceDisconnectedListener(this);
  OpenNI::addDeviceStateChangedListener(this);
}

// Destructor
pix_openni2 :: ~pix_openni2(){
  closeMess();
  delete(m_frameListener);
  delete(m_depthChannel);
  //~ OpenNI::shutdown(); // this should be called only on Pd quit
}

void pix_openni2 :: obj_setupCallback(t_class *classPtr)
{
  CPPEXTERN_MSG0(classPtr, "enumerate", enumerateMess);
  CPPEXTERN_MSG (classPtr, "open", openMess);
  CPPEXTERN_MSG (classPtr, "openBySerial", openBySerialMess);
  CPPEXTERN_MSG0(classPtr, "close", closeMess);
  CPPEXTERN_MSG1(classPtr, "rgb", rgbMess, t_float);
  CPPEXTERN_MSG1(classPtr, "depth", depthMess, t_float);
  CPPEXTERN_MSG1(classPtr, "ir", irMess, t_float);
  CPPEXTERN_MSG0(classPtr, "getVideoMode", getVideoMode);
  CPPEXTERN_MSG1(classPtr, "setVideoMode", setVideoMode, t_symbol*);
  
  printf("pix_openni2 by Antoine Villeret based on Matthias Kronlachner work, build on %s at %s\n", __DATE__, __TIME__);
  
     // initialize OpenNI
   Status rc = STATUS_OK;
   rc = OpenNI::initialize();
   if ( rc != STATUS_OK ){
      printf("can't initialized OpenNI : %s\n", OpenNI::getExtendedError());
#ifdef __linux__
      printf("this often happens when drivers are not reachable (in /usr/lib or near this external)\n");
#endif
      throw(GemException("OpenNI Init() failed\n"));
   }
   
   
}

void DepthChannel :: obj_setupCallback(t_class *classPtr)
{
  //~CPPEXTERN_MSG (classPtr, "depth_state", renderDepth);
}

void pix_openni2 :: enumerateMess(){
   
  Array<openni::DeviceInfo> deviceList;
  OpenNI::enumerateDevices(&deviceList);
  
  t_atom a_devices;
  SETFLOAT(&a_devices, deviceList.getSize());
  outlet_anything(m_dataout, gensym("devices"), 1, &a_devices);
  
  for(int i=0; i<deviceList.getSize(); i++){
    t_atom a_deviceInfo[6];
    SETSYMBOL(a_deviceInfo, gensym(deviceList[i].getUri()));
    char hexID[7];
    sprintf(hexID, "0x%04x", deviceList[i].getUsbVendorId());
    SETSYMBOL(a_deviceInfo+1, gensym(hexID));
    sprintf(hexID, "0x%04x", deviceList[i].getUsbProductId());
    SETSYMBOL(a_deviceInfo+2, gensym(hexID));
    SETSYMBOL(a_deviceInfo+3, gensym(deviceList[i].getVendor()));
    SETSYMBOL(a_deviceInfo+4, gensym(deviceList[i].getName()));
    
    Device device;
    char serial[512];
    int datasize=sizeof(serial);

    if ( device.open(deviceList[i].getUri()) == STATUS_OK ){
      if ( device.getProperty(ONI_DEVICE_PROPERTY_SERIAL_NUMBER, &serial, &datasize) == STATUS_OK ){
        SETSYMBOL(a_deviceInfo+5, gensym(serial));
      } else {
        SETSYMBOL(a_deviceInfo+5, gensym(deviceList[i].getUri()));
      }
      device.close();
    } else {
      SETSYMBOL(a_deviceInfo+5, gensym(deviceList[i].getUri()));
    }

    outlet_anything(m_dataout, gensym("device"), 6, a_deviceInfo);
    printf("device : %s %04x%04x %s %s %s\n",deviceList[i].getUri(),deviceList[i].getUsbVendorId(),deviceList[i].getUsbProductId(),deviceList[i].getVendor(),deviceList[i].getName(), serial);
  }
}

// enable/disable rgb grabbing
void pix_openni2 :: rgbMess(t_float f){
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
void pix_openni2 :: irMess(t_float f){
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
void pix_openni2 :: depthMess(t_float f){
  if ( !m_device.isValid() ){
    error("please open a device before...");
    return;
  }
  if ( f > 0 ){
    m_depth = 1;
    if ( !m_device.hasSensor(SENSOR_DEPTH) ){
      m_depth = 0;
      error("device %s has no rgb sensor", m_deviceURI);
    }
  } else {
    m_depth = 0;
  }
  t_atom a_depth;
  SETFLOAT(&a_depth, m_depth);
  outlet_anything(m_dataout, gensym("depth"), 1, &a_depth);
}

// Open a device/file by its URI
// one argument : symbol (URI)
void pix_openni2 :: openMess(t_symbol *s,int argc, t_atom*argv){   
  int deviceId = -1;

  if (argc>0){
    if (argv->a_type == A_SYMBOL){
      m_deviceURI=atom_getsymbol(argv)->s_name;
    } else if (argv->a_type == A_FLOAT) {
	  int arg = atom_getint(argv);
	  deviceId = arg>-1 ? arg : -1;
	}
  }
  
  Array<openni::DeviceInfo> deviceList;
  OpenNI::enumerateDevices(&deviceList);
	
  if ( deviceId > -1 ){
	if ( deviceId > deviceList.getSize()-1 ){
		error("id %d is out of device list (%d)", deviceId,deviceList.getSize());
		return;
	} else {
		m_deviceURI=deviceList[deviceId].getUri();
	}
  } else if ( m_deviceURI != ANY_DEVICE ){
	for (int i=0; i< deviceList.getSize();i++){
	  if ( std::string(m_deviceURI) == std::string(deviceList[i].getUri()) ){
		  m_deviceURI=deviceList[i].getUri();
		  continue;
	  }
	}
  }
  
  closeMess();
  
  Status rc = STATUS_OK;
  post("open device with URI %s", m_deviceURI);
  
  rc = m_device.open(m_deviceURI);
  t_atom a_status;
  SETFLOAT(&a_status, rc);
  outlet_anything(m_dataout, gensym("open"), 1, &a_status);
  if ( rc != STATUS_OK ){
    error("can't open device %s : %s", m_deviceURI, OpenNI::getExtendedError());
    return;
  } else {
	  m_connected = true;
  }
  
  char serial[512]; 
  int datasize=sizeof(serial);
  rc=m_device.getProperty(ONI_DEVICE_PROPERTY_SERIAL_NUMBER, &serial, &datasize);
  if ( rc != STATUS_OK ){
    printf("error when getting serial number : %s\n", OpenNI::getExtendedError());
    return;
  } else {
    printf("device %s has serial %s\n", m_deviceURI, serial);
  }
}

// OpenBySerial : open a device thanks to its serial
void pix_openni2 :: openBySerialMess(t_symbol *s,int argc, t_atom*argv){
  if ( argc < 1 ){
    error("openBySerial message needs one symbol argument");
    return;
  }
  
  if ( argv[0].a_type != A_SYMBOL ){
    error("openBySerial message needs one symbol argument");
    return;
  }
  
  char *a_serial;

  a_serial=argv[0].a_w.w_symbol->s_name;
  
  if ( a_serial[0] == '@' ) a_serial++;
  
  Array<openni::DeviceInfo> deviceList;
  OpenNI::enumerateDevices(&deviceList);
  int i;
  for(i=0; i<deviceList.getSize(); i++){
    Device device;
    Status rc = STATUS_OK;
    rc=device.open(deviceList[i].getUri());
    if ( rc == STATUS_OK ){
      char serial[512]; 
      int datasize=sizeof(serial);
      rc=device.getProperty(ONI_DEVICE_PROPERTY_SERIAL_NUMBER, &serial, &datasize);
      device.close();
      if( rc == STATUS_OK && std::string(a_serial) == std::string(serial) ){
        m_deviceURI=deviceList[i].getUri();
        if (m_device.isValid()) closeMess();
        rc=m_device.open(m_deviceURI);
        if( rc != STATUS_OK ){
          m_deviceURI=ANY_DEVICE;
        } else {
          verbose(4, "device with serial %s is open", a_serial);
          m_connected=true;
          t_atom a_status;
          SETFLOAT(&a_status,rc);
          outlet_anything(m_dataout, gensym("open"), 1, &a_status);
          break;
        }
      }
    }
  }
  if (i==deviceList.getSize()) error("can't find device with serial %s", a_serial);
}

// Close the device/file
void pix_openni2 :: closeMess(){
  m_connected=false;
  m_videoStream.removeNewFrameListener(m_frameListener);
  m_videoStream.stop();
  m_videoStream.destroy();
  m_device.close();
}

void pix_openni2 :: getVideoMode(){
    printf("getVideoMode\n");

  if ( !m_device.isValid() ){
    error("please open a device before...");
    return;
  }
  
  const SensorInfo *sensorInfo;
  sensorInfo = m_device.getSensorInfo(SENSOR_DEPTH);
  
  if ( sensorInfo == NULL ){
    error("current device doesn't have a depth sensor");
  } else {
    const Array<openni::VideoMode> *videoModeList;
    videoModeList = &sensorInfo->getSupportedVideoModes();
    printf("found %d videomode\n", videoModeList->getSize());
    for (int i=0;i<videoModeList->getSize();i++){
      VideoMode videoMode = (*videoModeList)[i];
      std::ostringstream convert;
      
      convert << videoMode.getResolutionX();
      convert << "x";
      convert << videoMode.getResolutionY();
      convert << "@";
      convert << videoMode.getFps();
      convert << "_";
      convert << videoMode.getPixelFormat();
      
      t_atom a_videoMode;
      SETSYMBOL(&a_videoMode, gensym(convert.str().c_str()));
      outlet_anything(m_dataout, gensym("video_mode"), 1, &a_videoMode);
    }
  }
}

void pix_openni2 :: setVideoMode(t_symbol *s_videoMode){
  std::string str_videoMode = std::string(s_videoMode->s_name);
}

void pix_openni2 :: render(GemState *state){
  Status rc = STATUS_OK;
  
  if ( !m_connected ){
	  error("no device opened");
	  return;
  }
  if ( !m_device.isValid() ){
	post("open device with URI : %s", m_deviceURI);
	Status rc = STATUS_OK;
    rc = m_device.open(m_deviceURI);
    if ( rc != STATUS_OK ){
      error("can't create rgb stream : %s", OpenNI::getExtendedError());
    }
  } 
  
  if ( !m_videoStream.isValid() ){
    Status rc = STATUS_OK;
    rc = m_videoStream.create(m_device, SENSOR_DEPTH); // force only depth output for now
    //~t_atom a_status;
    
    if ( rc != STATUS_OK ){
      error("can't create rgb stream : %s", OpenNI::getExtendedError());
      //~SETFLOAT(&a_status, rc);
      //~outlet_anything(m_dataout, gensym("rgb"), 1, &a_status);
      return;
    }
    
    rc = m_videoStream.start();
    if ( rc != STATUS_OK ){
      error("can't start rgb stream : %s", OpenNI::getExtendedError());
      //~SETFLOAT(&a_status, rc);
      //~outlet_anything(m_dataout, gensym("rgb"), 1, &a_status);
      return;
    }
    m_videoStream.addNewFrameListener(m_frameListener);
  }
  
  if ( m_frameListener->m_newFrame && m_device.isValid() && m_videoStream.isValid() ){

    m_frameListener->unSet();
    
    Status rc = STATUS_OK;
    
    rc = m_videoStream.readFrame(&m_videoFrameRef);
    if ( rc != STATUS_OK ){
      error("error when reading rgb frame : %s\n",OpenNI::getExtendedError());
      return;
    }
    
    VideoMode vmode = m_videoFrameRef.getVideoMode();
    
    if ( m_pixBlock.image.xsize != vmode.getResolutionX() || \
         m_pixBlock.image.ysize != vmode.getResolutionY() || \
         m_pixelFormat != vmode.getPixelFormat() )
    {
      m_pixBlock.image.xsize = vmode.getResolutionX();
      m_pixBlock.image.ysize = vmode.getResolutionY();
      m_pixelFormat = vmode.getPixelFormat();
      
      if ( m_pixelFormat == PIXEL_FORMAT_GRAY8){
        m_pixBlock.image.csize = 1;
      } else {
        m_pixBlock.image.csize = 4;
      }
      
      m_pixBlock.image.reallocate();
    }
    
    m_pixBlock.newimage=true;
    
    unsigned char *data = (unsigned char *) m_videoFrameRef.getData();
    
    unsigned char *pixels=m_pixBlock.image.data;
    int size = m_pixBlock.image.xsize * m_pixBlock.image.ysize;
    
    int step;
    switch ( vmode.getPixelFormat() ){
      case PIXEL_FORMAT_RGB888:
        m_pixBlock.image.fromRGB(data);
        m_pixBlock.image.notowned=0;
        break;
      case PIXEL_FORMAT_GRAY8:
        m_pixBlock.image.data=data;
        m_pixBlock.image.notowned=1;
        break;
      default:
        while ( size-- ){
          pixels[chGreen]=*data++;
          pixels[chRed]=*data++;
          pixels[chBlue]=0;
          pixels[chAlpha]=255;
          pixels+=4;
        }
        m_pixBlock.image.notowned=0;
        break;
    }
    state->set(GemState::_PIX, &m_pixBlock);
  } else {
    m_pixBlock.newimage=false;
    state->set(GemState::_PIX, &m_pixBlock);
  }
}


void DepthChannel :: render(GemState *state){
  printf("render depth channel\n");
  if ( m_frameListener->m_newFrame ){
    printf("new depth frame \n");
    m_frameListener->unSet();
    
    Status rc = STATUS_OK;
    
    rc = m_videoStream.readFrame(&m_videoFrameRef);
    if ( rc != STATUS_OK ){
      error("error when reading rgb frame : %s\n",OpenNI::getExtendedError());
      return;
    }
    
    VideoMode vmode = m_videoFrameRef.getVideoMode();
    
    if ( m_pixBlock.image.xsize != vmode.getResolutionX() || \
         m_pixBlock.image.ysize != vmode.getResolutionY() || \
         m_pixelFormat != vmode.getPixelFormat() )
    {
      m_pixBlock.image.xsize = vmode.getResolutionX();
      m_pixBlock.image.ysize = vmode.getResolutionY();
      m_pixelFormat = vmode.getPixelFormat();
      
      if ( m_pixelFormat == PIXEL_FORMAT_GRAY8){
        m_pixBlock.image.csize = 1;
      } else {
        m_pixBlock.image.csize = 4;
      }
      
      m_pixBlock.image.reallocate();
      state->set(GemState::_PIX, &m_pixBlock);
    }
    
    m_pixBlock.newimage=true;
    
    unsigned char *data = (unsigned char *) m_videoFrameRef.getData();
    
    unsigned char *pixels=m_pixBlock.image.data;
    int size = m_pixBlock.image.xsize * m_pixBlock.image.ysize;
    
    int step;
    switch ( vmode.getPixelFormat() ){
      case PIXEL_FORMAT_RGB888:
        m_pixBlock.image.fromRGB(data);
        m_pixBlock.image.notowned=0;
        break;
      case PIXEL_FORMAT_GRAY8:
        m_pixBlock.image.data=data;
        m_pixBlock.image.notowned=1;
        break;
      case PIXEL_FORMAT_GRAY16:
        while ( size-- ){
          pixels[chRed]=*data++;
          pixels[chGreen]=*data++;
          pixels[chBlue]=0;
          pixels[chAlpha]=255;
          pixels+=4;
        }
        m_pixBlock.image.notowned=0;
        break;
    }
  } else {
    m_pixBlock.newimage=false;
  }
}

void pix_openni2 :: startRendering(){
  //~if (m_rgb){

}

void DepthChannel :: startRendering(){
  if (*m_depthPt){
    Status rc = STATUS_OK;
    rc = m_videoStream.create(*m_devicePt, SENSOR_DEPTH);
    //~t_atom a_status;
    
    if ( rc != STATUS_OK ){
      error("can't create depth stream : %s", OpenNI::getExtendedError());
      //~SETFLOAT(&a_status, rc);
      //~outlet_anything(m_dataout, gensym("rgb"), 1, &a_status);
      return;
    }
    
    rc = m_videoStream.start();
    if ( rc != STATUS_OK ){
      error("can't start depth stream : %s", OpenNI::getExtendedError());
      //~SETFLOAT(&a_status, rc);
      //~outlet_anything(m_dataout, gensym("rgb"), 1, &a_status);
      return;
    }
    m_videoStream.addNewFrameListener(m_frameListener);
  }
}

void pix_openni2 :: stopRendering(){
}

void DepthChannel :: stopRendering(){
}

void pix_openni2 :: onDeviceStateChanged(const DeviceInfo* pInfo, DeviceState state) 
{
  printf("Device \"%s\" error state changed to %d\n", pInfo->getUri(), state);
}

void pix_openni2 :: onDeviceConnected(const DeviceInfo* pInfo)
{
  printf("Device \"%s\" connected\n", pInfo->getUri());
}

void pix_openni2 :: onDeviceDisconnected(const DeviceInfo* pInfo)
{
  const char * uri = pInfo->getUri();
  printf("Device \"%s\" disconnected\n", uri);
  printf("m_deviceURI : %s\n", m_deviceURI);
  printf("deviceANY : %s\n", ANY_DEVICE);
  if ( strcmp(uri,m_deviceURI)==0 ){
	printf("call close device\n");
	m_connected = false;
  }
}
