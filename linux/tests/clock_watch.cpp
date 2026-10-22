#define main yokoi_program
#include "../main.cpp"
#undef main
#include "SM5XX/SM510/SM510.h"
#include <cassert>
#include <map>
struct ProbeCPU:SM510{std::map<int,int> digits;void set_ram_value(uint8_t c,uint8_t l,uint8_t v)override{digits[c*100+l]=v;}};
struct ProbeInput:Virtual_Input{int startsA=0,startsB=0;ProbeInput(SM5XX*c):Virtual_Input(c){}void set_input(uint8_t part,uint8_t button,bool on,uint8_t=1)override{if(part==PART_SETUP&&on){if(button==BUTTON_GAMEA)startsA++;if(button==BUTTON_GAMEB)startsB++;}}};
int main(){
 setenv("TZ","UTC-3",1);tzset();ProbeCPU cpu;std::vector<uint8_t>rom(4096);cpu.load_rom(rom.data(),rom.size());cpu.init();cpu.load_rom_time_addresses("TL_28");warmClockRom(&cpu,true);
 auto check=[&](){time_t n=time(nullptr);tm t{};localtime_r(&n,&t);int hour=t.tm_hour%12;if(!hour)hour=12;assert(cpu.digits[105]==hour%10);assert(cpu.digits[104]==((hour/10)|(t.tm_hour>=12?8:0)));assert(cpu.digits[106]==t.tm_min/10&&cpu.digits[107]==t.tm_min%10);};check();
 // Incorrect/stale RAM must be resynchronised even after the one-time flag was set.
 cpu.set_ram_value(1,5,99);assert(cpu.is_time_set());syncHostClock(&cpu);check();
 ProbeInput input(&cpu);EmuWorker worker;std::vector<Segment>segs;worker.start(&cpu,&input,segs,0,nullptr,-1,true);SDL_Delay(2700);worker.stop();assert(input.startsA==0&&input.startsB==0);check();
 worker.start(&cpu,&input,segs,0,nullptr,0,false);SDL_Delay(350);worker.stop();assert(input.startsA>0&&input.startsB==0);int a=input.startsA;
 worker.start(&cpu,&input,segs,0,nullptr,1,false);SDL_Delay(350);worker.stop();assert(input.startsA==a&&input.startsB>0);
 puts("PASS: Linux local timezone, clock digits/PM after ROM warmup, re-sync stale RAM, watch runs beyond old auto-start delay without A/B, explicit A/B pulses only on request");
}
