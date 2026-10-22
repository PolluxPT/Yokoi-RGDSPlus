#pragma once
#include "yokoi_cpu_utils.h"
#include <ctime>
// Call only on the emulation thread, or while the worker is stopped.
inline void syncHostClock(SM5XX*cpu){cpu->time_set(false);yokoi_cpu_set_time_if_needed(cpu);}
inline void warmClockRom(SM5XX*cpu,bool supported){
 if(supported)syncHostClock(cpu);
 int grace=500;
 for(int i=0;i<2000;i++)if(cpu->step()&&supported&&grace>0){--grace;syncHostClock(cpu);}
 if(supported)syncHostClock(cpu);
 cpu->segments_state_are_update=true;
}
