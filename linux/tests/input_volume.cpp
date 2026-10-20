#include "rgds_input.h"
#include "rgds_volume.h"
#include "button_profile.h"
#include <cassert>
#include <cmath>
#include <fstream>
#include <unistd.h>

int main(){
 // A complete tap between two SDL/render frames must still reach the emulated CPU.
 RgdsInput raw;raw.key(BTN_START,1,100);raw.key(BTN_START,0,101);
 assert(raw.setupMask()==0);assert(raw.takePresses()==CTL_START);assert(raw.takePresses()==0);
 raw.key(BTN_SELECT,1,200);assert(raw.setupMask()==CTL_SELECT);
 raw.key(BTN_START,1,201);assert(raw.setupMask()==(CTL_START|CTL_SELECT));
 raw.key(BTN_SELECT,0,210);raw.key(BTN_START,0,211);assert(raw.setupMask()==0);
 // L2/R2 kernel codes must not start a game or operate Select.
 raw.takePresses();raw.key(BTN_TL2,1,220);raw.key(BTN_TR2,1,220);
 assert(raw.setupMask()==0&&raw.takePresses()==0);
 // Kernel autorepeat must not double the application's controlled repeat rate.
 raw.key(KEY_VOLUMEUP,1,300);assert(raw.volumeDelta(300)==1);
 raw.key(KEY_VOLUMEUP,2,350);assert(raw.volumeDelta(350)==0);
 assert(raw.volumeDelta(749)==0);assert(raw.volumeDelta(750)==1);
 raw.key(KEY_VOLUMEUP,0,760);assert(raw.volumeDelta(900)==0);
 raw.key(KEY_VOLUMEDOWN,1,1000);raw.key(KEY_VOLUMEDOWN,0,1001);assert(raw.volumeDelta(1002)==-1);
 PressLatch latch;latch.press(CTL_A,100);assert(latch.apply(0,101)==CTL_A);
 assert(latch.apply(0,115)==CTL_A);assert(latch.apply(0,116)==0);
 assert(latch.apply(CTL_A,200)==CTL_A);
 assert(RgdsVolume::curve(0)==0);assert(std::abs(RgdsVolume::curve(1)-.0035f)<1e-6);
 assert(std::abs(RgdsVolume::curve(5)-.0875f)<1e-6);
 assert(std::abs(RgdsVolume::curve(10)-.35f)<1e-6);
 float previous=0,previousStep=0;for(int i=1;i<=10;i++){float gain=RgdsVolume::curve(i),step=gain-previous;assert(gain>previous&&step>=previousStep-1e-6);previous=gain;previousStep=step;}
 char dir[]="/tmp/yokoi-volume-test-XXXXXX";assert(mkdtemp(dir));
 std::string stock=std::string(dir)+"/stock",saved=std::string(dir)+"/saved";
 {std::ofstream(stock)<<5;}RgdsVolume vol(stock,saved);vol.init();assert(vol.level()==5);
 vol.change(1,100);assert(vol.level()==6);int persisted=-1;{std::ifstream(stock)>>persisted;}assert(persisted==6);
 vol.change(-100,200);assert(vol.level()==0&&vol.gain()==0);
 vol.change(100,300);assert(vol.level()==10);
 {std::ofstream(stock)<<2;}assert(vol.poll(1000)&&vol.level()==2);
 RgdsVolume fallback(std::string(dir)+"/missing/stock",saved);fallback.init();assert(fallback.level()==10);
 fallback.change(-1,100);fallback.poll(1000);assert(fallback.level()==9);
 unlink(stock.c_str());unlink(saved.c_str());rmdir(dir);
 // Reproduce the old SDL mapping where buttons 6/7 are incorrectly Back/Start.
 assert(SDL_Init(SDL_INIT_GAMECONTROLLER)==0);
 int index=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,0,10,0);assert(index>=0);
 char guid[64];SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(index),guid,sizeof guid);
 std::string mapping=std::string(guid)+",ANBERNIC-test,a:b0,b:b1,x:b2,y:b3,back:b6,start:b7,leftstick:b8,rightstick:b9,";
 assert(SDL_GameControllerAddMapping(mapping.c_str())>=0);
 auto ctl=SDL_GameControllerOpen(index);assert(ctl);auto joy=SDL_GameControllerGetJoystick(ctl);
 ButtonProfile profile;profile.buttons[0]=4;profile.buttons[1]=5;profile.buttons[2]=0;profile.buttons[3]=1;profile.buttons[4]=2;profile.buttons[5]=3;profile.buttons[6]=6;profile.buttons[7]=7;profile.ready=true;
 assert(profile.valid(10));assert(profile.press(4)==CTL_START);assert(profile.press(5)==CTL_SELECT);
 assert(!profile.swapPress(5)&&profile.swapPress(1));assert(profile.press(6)==CTL_X&&profile.press(7)==CTL_Y);
 SDL_JoystickSetVirtualButton(joy,6,1);SDL_JoystickUpdate();
 assert(SDL_GameControllerGetButton(ctl,SDL_CONTROLLER_BUTTON_BACK));assert((profile.held(ctl)&(CTL_START|CTL_SELECT))==0);
 SDL_JoystickSetVirtualButton(joy,4,1);SDL_JoystickUpdate();assert((profile.held(ctl)&(CTL_START|CTL_SELECT))==CTL_START);
 SDL_JoystickSetVirtualButton(joy,5,1);SDL_JoystickUpdate();assert((profile.held(ctl)&(CTL_START|CTL_SELECT))==(CTL_START|CTL_SELECT));
 const char*config="/tmp/yokoi-profile-test.cfg";assert(profile.save(ctl,config));ButtonProfile restored;assert(restored.load(ctl,config));assert(restored.buttons[0]==4&&restored.buttons[1]==5);unlink(config);
 restored.buttons[3]=restored.buttons[0];assert(!restored.valid(10));
 SDL_GameControllerClose(ctl);SDL_JoystickDetachVirtual(index);SDL_Quit();
 puts("PASS: physical setup mapping, short taps, volume keys/repeat, quadratic curve, persistence, calibrated arbitrary-index SDL regression");
}
