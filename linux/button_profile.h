#pragma once
#include <SDL.h>
#include <cstdio>
#include <string>
#include "yokoi_input_mapping.h"

struct ButtonProfile {
 int buttons[8]{-1,-1,-1,-1,-1,-1,-1,-1}; // START, SELECT, TIME/L1, SWAP/R1
 bool ready=false;
 int special=-1; char specialType='B';
 bool loadSpecial(SDL_GameController*c){FILE*f=fopen("special-rgds.cfg","r");if(!f)return false;char g[64]{};bool ok=fscanf(f,"%63s %c %d",g,&specialType,&special)==3;fclose(f);return ok&&guid(c)==g&&special>=0&&(specialType=='B'||specialType=='K'||specialType=='E'||specialType=='N');}
 bool saveSpecial(SDL_GameController*c){FILE*f=fopen("special-rgds.cfg.new","w");if(!f)return false;bool ok=fprintf(f,"%s %c %d\n",guid(c).c_str(),specialType,special)>0;if(fclose(f))ok=false;if(ok)ok=rename("special-rgds.cfg.new","special-rgds.cfg")==0;if(ok)printf("RGDS special button: type=%c code=%d\n",specialType,special);return ok;}
 static std::string guid(SDL_GameController*c){char s[64]{};if(c)SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(SDL_GameControllerGetJoystick(c)),s,sizeof s);return s;}
 bool valid(int count)const{for(int i=0;i<8;i++){if(buttons[i]<0||buttons[i]>=count)return false;for(int j=0;j<i;j++)if(buttons[j]==buttons[i])return false;}return true;}
 bool load(SDL_GameController*c,const char*path="buttons-rgds-v2.cfg"){
  if(!c)return false;FILE*f=fopen(path,"r");if(!f)return false;char g[64]{};
  bool ok=fscanf(f,"%63s %d %d %d %d %d %d %d %d",g,&buttons[0],&buttons[1],&buttons[2],&buttons[3],&buttons[4],&buttons[5],&buttons[6],&buttons[7])==9;fclose(f);
  ready=ok&&guid(c)==g&&valid(SDL_JoystickNumButtons(SDL_GameControllerGetJoystick(c)));
  if(ready)printf("RGDS calibrated buttons: START=%d SELECT=%d L1=%d R1=%d A=%d B=%d X=%d Y=%d\n",buttons[0],buttons[1],buttons[2],buttons[3],buttons[4],buttons[5],buttons[6],buttons[7]);return ready;
 }
 bool save(SDL_GameController*c,const char*path="buttons-rgds-v2.cfg"){
  if(!c||!valid(SDL_JoystickNumButtons(SDL_GameControllerGetJoystick(c))))return false;
  std::string temp=std::string(path)+".new";FILE*f=fopen(temp.c_str(),"w");if(!f)return false;
  bool ok=fprintf(f,"%s %d %d %d %d %d %d %d %d\n",guid(c).c_str(),buttons[0],buttons[1],buttons[2],buttons[3],buttons[4],buttons[5],buttons[6],buttons[7])>0;
  if(fclose(f)!=0)ok=false;if(ok)ok=rename(temp.c_str(),path)==0;if(!ok)remove(temp.c_str());ready=ok;return ok;
 }
 bool reserved(int button)const{if(!ready)return false;for(int b:buttons)if(b==button)return true;return false;}
 int loadLegacy(SDL_GameController*c){
  FILE*f=fopen("buttons-rgds-v1.cfg","r");if(!f||!c){if(f)fclose(f);return 0;}char g[64]{};
  bool ok=fscanf(f,"%63s %d %d %d %d",g,&buttons[0],&buttons[1],&buttons[2],&buttons[3])==5;fclose(f);
  if(!ok||guid(c)!=g)return 0;int count=SDL_JoystickNumButtons(SDL_GameControllerGetJoystick(c));for(int i=0;i<4;i++){if(buttons[i]<0||buttons[i]>=count)return 0;for(int j=0;j<i;j++)if(buttons[i]==buttons[j])return 0;}return 4;
 }
 uint32_t press(int button)const{
  if(!ready)return 0;
  if(button==buttons[0])return CTL_START;if(button==buttons[1])return CTL_SELECT;if(button==buttons[2])return CTL_L1;for(int i=4;i<8;i++)if(button==buttons[i])return CTL_A<<(i-4);return 0;
 }
 uint32_t held(SDL_GameController*c)const{
  if(!ready||!c)return 0;auto*j=SDL_GameControllerGetJoystick(c);uint32_t result=0;
  for(int i=0;i<8;i++)if(i!=3&&SDL_JoystickGetButton(j,buttons[i]))result|=press(buttons[i]);return result;
 }
 bool swapPress(int button)const{return ready&&button==buttons[3];}
};
