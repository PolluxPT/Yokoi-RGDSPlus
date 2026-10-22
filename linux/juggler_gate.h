#pragma once
#include <cstdint>
#include "SM5XX/SM5XX.h"
#include "yokoi_input_mapping.h"

// Mario The Juggler keeps animating after Game Over, without CEND. Its three
// native MISS icons, or the Watch AM/PM indicator, establish a finished run.
// This guard is enabled only for the MB_108 virtual input configuration.
class JugglerGate {
 bool enabled,ready,observedPlay,started=false;
 uint32_t previousPhysical=0,previousKeys=0,allowed=0,pulse=0;
 uint64_t pulseUntil=0;
public:
 JugglerGate(bool use,bool watch):enabled(use),ready(watch),observedPlay(!watch){}
 void explicitStart(){ready=true;allowed=CTL_START|CTL_SELECT;previousKeys=0;}
 bool consumeStarted(){bool value=started;started=false;return value;}
 uint32_t update(uint32_t keys,uint64_t cycle,uint32_t frequency,SM5XX&cpu,uint32_t physical){
  if(!enabled)return keys;
  constexpr uint32_t setup=CTL_START|CTL_SELECT;
  bool terminal=cpu.get_segments_state(0,11,0)||cpu.get_segments_state(0,12,3)
   ||(cpu.get_segments_state(1,5,0)&&cpu.get_segments_state(0,6,0)&&cpu.get_segments_state(1,6,0));
  // Wait for the previous terminal display to clear after a request. Otherwise
  // stale MISS icons could immediately reenable GAME during the new run.
  if(!ready){
   if(!terminal)observedPlay=true;
   if(terminal&&observedPlay&&cycle>=pulseUntil)ready=true;
  }
  uint32_t freshPhysical=(physical&setup)&~previousPhysical;
  allowed=(allowed&~freshPhysical)|(ready?freshPhysical:0);
  previousPhysical=physical&setup;
  uint32_t freshKeys=(keys&setup)&~previousKeys;previousKeys=keys&setup;
  if(ready&&(freshKeys&allowed)){
   pulse=(freshKeys&allowed&CTL_START)?CTL_START:CTL_SELECT;
   pulseUntil=cycle+frequency/4;ready=false;observedPlay=false;started=true;
  }
  // Held keys and presses made before Game Over never queue another run.
  return (keys&~setup)|(cycle<pulseUntil?pulse:0);
 }
};
