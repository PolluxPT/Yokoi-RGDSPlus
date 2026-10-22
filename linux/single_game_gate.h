#pragma once
#include <cstdint>
#include "SM5XX/SM5XX.h"
#include "yokoi_input_mapping.h"

// The single-GAME machines accept GAME even while playing (it resets the run).
// Their ROMs execute CEND when returning to Watch after the run. Observe every
// CEND, including ones immediately woken by held gameplay inputs. Never queue
// a press made during play: starting again requires a new physical press.
class SingleGameGate {
 bool enabled,ready,previousStart=false,previousPhysical=false,pressAllowed=false,started=false;
 uint64_t idleSequence,pulseUntil=0;
public:
 SingleGameGate(bool single,bool watch,SM5XX&cpu)
  :enabled(single),ready(watch),idleSequence(cpu.idle_sequence_number()){}
 void explicitStart(){ready=true;previousStart=false;pressAllowed=true;}
 bool consumeStarted(){bool value=started;started=false;return value;}
 uint32_t update(uint32_t keys,uint64_t cycle,uint32_t frequency,SM5XX&cpu,uint32_t physical){
  if(!enabled)return keys;
  uint64_t sequence=cpu.idle_sequence_number();
  if(sequence!=idleSequence&&cycle>=pulseUntil)ready=true;
  idleSequence=sequence;
  bool physicalStart=physical&CTL_START;
  if(physicalStart&&!previousPhysical)pressAllowed=ready;
  previousPhysical=physicalStart;
  bool pressed=keys&CTL_START;
  if(ready&&pressAllowed&&pressed&&!previousStart){
   ready=false;started=true;pulseUntil=cycle+frequency/16;
  }
  previousStart=pressed;
  // A bounded GAME pulse also prevents a held START repeatedly resetting a
  // newly started run. SELECT is intentionally unmapped on these machines.
  return (keys&~(CTL_START|CTL_SELECT))|(cycle<pulseUntil?CTL_START:0);
 }
};
