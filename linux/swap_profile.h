#pragma once
#include <SDL.h>
#include <array>
#include <cstdio>
#include <string>
#include "button_profile.h"

// Physical bindings, independent of SDL's controller database.
struct SwapBinding {
 char type='N'; int code=0,rest=0,direction=1; bool held=false;
 bool valid()const{return (type=='N'||type=='B'||type=='A'||type=='K'||type=='E')&&code>=0&&code<=65535&&rest>=-32768&&rest<=32767&&(direction==1||direction==-1);}
 bool axis(int index,int value){
  if(type!='A'||code!=index)return false;
  int distance=(value-rest)*direction;bool before=held;
  if(distance>=12000)held=true;else if(distance<6000)held=false;
  return held&&!before;
 }
 bool button(int index,bool down){if(type!='B'||code!=index)return false;bool fire=down&&!held;held=down;return fire;}
};
struct SwapProfile {
 std::array<SwapBinding,2> bindings{}; bool ready=false; int lastFired=-1;
 bool save(SDL_GameController*c,const char*path="swap-rgds-v1.cfg")const{
  std::string tmp=std::string(path)+".new";FILE*f=fopen(tmp.c_str(),"w");if(!f)return false;
  bool ok=fprintf(f,"%s\n",ButtonProfile::guid(c).c_str())>0;
  for(const auto&b:bindings)ok=(fprintf(f,"%c %d %d %d\n",b.type,b.code,b.rest,b.direction)>0)&&ok;
  if(fclose(f))ok=false;if(ok)ok=rename(tmp.c_str(),path)==0;if(!ok)remove(tmp.c_str());return ok;
 }
 bool load(SDL_GameController*c,const char*path="swap-rgds-v1.cfg"){
  ready=false;FILE*f=fopen(path,"r");if(!f)return false;char guid[64]{};
  bool ok=fscanf(f,"%63s",guid)==1&&ButtonProfile::guid(c)==guid;
  for(auto&b:bindings){b.held=false;ok=(fscanf(f," %c %d %d %d",&b.type,&b.code,&b.rest,&b.direction)==4&&b.valid())&&ok;}
  fclose(f);if(!ok)return false;
  auto*j=SDL_GameControllerGetJoystick(c);
  for(const auto&b:bindings)if((b.type=='B'&&b.code>=SDL_JoystickNumButtons(j))||(b.type=='A'&&b.code>=SDL_JoystickNumAxes(j))||(b.type=='K'&&b.code>=SDL_NUM_SCANCODES))return false;
  ready=true;for(int n=0;n<2;n++){const auto&b=bindings[n];printf("RGDS %s: type=%c code=%d rest=%d direction=%d\n",n?"R2":"L2",b.type,b.code,b.rest,b.direction);}return true;
 }
 bool event(const SDL_Event&e,SDL_JoystickID id){
  if(!ready)return false;bool fire=false;
  for(size_t n=0;n<bindings.size();n++){auto&b=bindings[n];bool triggered=false;
   if(e.type==SDL_JOYAXISMOTION&&e.jaxis.which==id)triggered=b.axis(e.jaxis.axis,e.jaxis.value);
   if((e.type==SDL_JOYBUTTONDOWN||e.type==SDL_JOYBUTTONUP)&&e.jbutton.which==id)triggered=b.button(e.jbutton.button,e.type==SDL_JOYBUTTONDOWN);
   if(e.type==SDL_KEYDOWN&&!e.key.repeat&&b.type=='K'&&b.code==e.key.keysym.scancode)triggered=true;
   if(triggered){fire=true;lastFired=int(n);}
  }return fire;
 }
 bool key(int code){if(!ready)return false;for(size_t n=0;n<bindings.size();n++)if(bindings[n].type=='E'&&bindings[n].code==code){lastFired=int(n);return true;}return false;}
};
