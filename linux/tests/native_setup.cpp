#include "../native_setup.h"
#include <cassert>
#include <cstdio>
static constexpr uint32_t hz=32768,mask=CTL_START|CTL_SELECT;
int main(){
 NativeSetup gate;
 for(unsigned t=0;t<hz;t+=32)assert(!(gate.update(CTL_START,t,hz)&mask));
 gate.update(0,hz,hz);
 bool a=false,b=false;
 for(unsigned t=hz;t<hz*2;t+=32)a|=bool(gate.update(CTL_START|CTL_DPAD_LEFT,t,hz)&CTL_START);
 assert(a);gate.update(0,hz*2,hz);
 for(unsigned t=hz*2;t<hz*3;t+=32)b|=bool(gate.update(CTL_SELECT,t,hz)&CTL_SELECT);
 assert(b);gate.update(0,hz*3,hz);
 // A Pause chord after release must never reach the ROM as setup.
 for(unsigned t=hz*3;t<hz*4;t+=32)assert(!(gate.update(mask,t,hz)&mask));
 NativeSetup tap;tap.update(0,0,hz);tap.update(CTL_START,32,hz);tap.update(0,64,hz);
 bool shortTap=false;for(unsigned t=96;t<hz/4;t+=32)shortTap|=bool(tap.update(0,t,hz)&CTL_START);assert(shortTap);
 NativeSetup climber;climber.update(0,0,hz);bool selectOnly=false;
 for(unsigned t=32;t<hz/4;t+=32){auto keys=climber.update(CTL_SELECT,t,hz);assert(!(keys&CTL_START));selectOnly|=bool(keys&CTL_SELECT);}assert(selectOnly);
 NativeSetup explicitGame;explicitGame.explicitStart(CTL_SELECT,hz);assert(explicitGame.update(0,0,hz)&CTL_SELECT);assert(!(explicitGame.update(0,hz/2,hz)&mask));
 puts("PASS native setup: opening hold, repeated A/B without Game Over profiles, Pause chord, short tap, no SELECT-to-START alias and explicit Pause start");
}
