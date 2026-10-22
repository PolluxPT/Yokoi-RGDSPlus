#pragma once
#include <cstdint>
#include "yokoi_input_mapping.h"

// Forward native setup buttons, leaving game state transitions to the ROM.
// Retain library-release gating and the START+SELECT Pause chord only.
class NativeSetup {
 uint32_t previous=0,pending=0,forced=0;
 uint64_t pendingAt=0,minimumUntil=0,forcedUntil=0;
 bool waitingRelease=true,delivered=false,started=false;
public:
 void explicitStart(uint32_t bit,uint32_t frequency){forced=bit;forcedUntil=frequency/4;}
 bool consumeStarted(){bool value=started;started=false;return value;}
 uint32_t update(uint32_t controls,uint64_t cycle,uint32_t frequency){
  constexpr uint32_t setup=CTL_START|CTL_SELECT;uint32_t buttons=controls&setup,out=0;
  if(waitingRelease){if(!buttons)waitingRelease=false;previous=buttons;buttons=0;}
  uint32_t fresh=buttons&~previous;previous=buttons;
  if(buttons==setup){pending=0;forced=0;}
  else{
   if(fresh){pending=(fresh&CTL_START)?CTL_START:CTL_SELECT;pendingAt=cycle;delivered=false;}
   if(pending&&cycle-pendingAt>=frequency*60/1000){
    if(!delivered){delivered=true;minimumUntil=cycle+frequency/16;started=true;}
    if((buttons&pending)||cycle<minimumUntil)out=pending;else pending=0;
   }
   if(cycle<forcedUntil)out|=forced;else forced=0;
  }
  return (controls&~setup)|out;
 }
};
