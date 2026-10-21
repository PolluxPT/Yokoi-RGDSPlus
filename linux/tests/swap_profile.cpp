#include "../swap_profile.h"
#include <cassert>
#include <unistd.h>

int main(){
 assert(SDL_Init(SDL_INIT_GAMECONTROLLER)==0);
 int index=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,4,16,1);assert(index>=0);
 auto*c=SDL_GameControllerOpen(index);assert(c);auto id=SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(c));
 SwapProfile p;p.ready=true;p.bindings[0]={'B',8};p.bindings[1]={'B',9};
 SDL_Event e{};e.type=SDL_JOYBUTTONDOWN;e.jbutton.which=id;
 for(int n:{8,9}){e.jbutton.button=n;assert(p.event(e,id));assert(!p.event(e,id));e.type=SDL_JOYBUTTONUP;assert(!p.event(e,id));e.type=SDL_JOYBUTTONDOWN;assert(p.event(e,id));}
 e.jbutton.button=12;assert(!p.event(e,id)); // special R must not swap
 e.jbutton.button=8;e.jbutton.which=id+1;assert(!p.event(e,id));
 for(int rest:{-32768,0,32767})for(int direction:{-1,1}){
  if((rest==-32768&&direction==-1)||(rest==32767&&direction==1))continue;
  SwapBinding b{'A',2,rest,direction};assert(!b.axis(2,rest));
  assert(b.axis(2,rest+direction*16000));assert(!b.axis(2,rest+direction*15000));
  assert(!b.axis(2,rest+direction*8000));assert(!b.axis(2,rest+direction*16000));
  assert(!b.axis(2,rest+direction*1000));assert(b.axis(2,rest+direction*16000));
 }
 p.bindings[0]={'A',0,-32768,1};p.bindings[1]={'A',3,32767,-1};
 e.type=SDL_CONTROLLERAXISMOTION;e.caxis.which=id;e.caxis.axis=SDL_CONTROLLER_AXIS_TRIGGERLEFT;e.caxis.value=32767;assert(!p.event(e,id));
 char path[]="/tmp/yokoi-swap-XXXXXX";int fd=mkstemp(path);assert(fd>=0);close(fd);assert(p.save(c,path));SwapProfile restored;assert(restored.load(c,path));assert(restored.bindings[1].rest==32767&&restored.bindings[1].direction==-1);
 FILE*f=fopen(path,"w");fputs("wrong-guid\nA 0 -32768 1\nA 3 0 1\n",f);fclose(f);assert(!restored.load(c,path));assert(!restored.ready);unlink(path);
 p.bindings[0]={'E',354};assert(p.key(354));assert(!p.key(312));p.bindings[1]={'K',SDL_SCANCODE_F14};
 e={};e.type=SDL_KEYDOWN;e.key.keysym.scancode=SDL_SCANCODE_F14;assert(p.event(e,id));e.key.repeat=1;assert(!p.event(e,id));
 SDL_GameControllerClose(c);SDL_Quit();puts("PASS: digital edges, axis baselines/directions/hysteresis, special isolation, device filtering, duplicate SDL controller events, persistence, evdev and keyboard");
}
