#include <cassert>
#include <cstdio>
#include <memory>
#include <vector>
#include <atomic>
#include <ctime>
#include "std/gw_pack.h"
#include "virtual_i_o/virtual_input.h"
#include "yokoi_cpu_utils.h"
#include "yokoi_input_mapping.h"
#include "clock_sync.h"
#include "native_setup.h"
#include "single_game_gate.h"
#include "juggler_gate.h"
extern "C" time_t time(time_t*out) noexcept{time_t n=1792670961;if(out)*out=n;return n;}
using Bytes=std::vector<unsigned char>;
Bytes save(SM5XX&c){FILE*f=tmpfile();assert(c.save_state(f));long n=ftell(f);rewind(f);Bytes b(n);assert(fread(b.data(),1,n,f)==size_t(n));fclose(f);return b;}
void load(SM5XX&c,const Bytes&b){FILE*f=tmpfile();fwrite(b.data(),1,b.size(),f);rewind(f);assert(c.load_state(f));fclose(f);}
struct Machine{
 std::unique_ptr<SM5XX>cpu;std::unique_ptr<Virtual_Input>input;std::atomic<int>zero{0};
 explicit Machine(const GW_rom*g){assert(yokoi_cpu_create_instance(cpu,g->rom,g->size_rom));cpu->load_rom(g->rom,g->size_rom);cpu->load_rom_melody(g->melody,g->size_melody);cpu->init();cpu->load_rom_time_addresses(g->ref);warmClockRom(cpu.get(),true);input.reset(get_input_config(cpu.get(),g->ref));assert(input);cpu->set_input_multiplexage(input->use_multiplexage);}
 void step(uint32_t keys,unsigned count){yokoi_input_apply_controller_mask(input.get(),keys,0,zero,zero);for(unsigned i=0;i<count;i++)cpu->step();}
};

bool exhausted(SM5XX&c){return c.get_segments_state(1,5,0)&&c.get_segments_state(0,6,0)&&c.get_segments_state(1,6,0);}
void mode(SM5XX&c,unsigned button,bool fish=false){
 bool a=c.get_segments_state(fish?1:0,5,fish?0:1),b=c.get_segments_state(0,5,0);
 assert(a==(button==CTL_START));assert(b==(button==CTL_SELECT));
}
int main(int argc,char**argv){assert(argc==2);std::string error;assert(gw_pack::load(argv[1],&error));unsigned titles=0;
 for(size_t i=0;i<gw_pack::game_count();i++){
  auto*g=gw_pack::game_at(i);if(g->ref!="TF_104"&&g->ref!="MB_108")continue;bool fish=g->ref=="TF_104";titles++;
  for(unsigned stride:{32u,256u})for(unsigned button:{unsigned(CTL_START),unsigned(CTL_SELECT)}){
   Machine game(g);unsigned f=game.cpu->frequency;NativeSetup transport;JugglerGate gate(!fish,true);unsigned starts=0;
   for(unsigned t=0;t<f*5;t+=stride){unsigned k=t>=f*3&&t<f*7/2?button:0;auto out=gate.update(transport.update(k,t,f),t,f,*game.cpu,k);starts+=gate.consumeStarted();game.step(out,stride);}
   mode(*game.cpu,button,fish);
   if(fish){
    // The Pause menu recreates the CPU and sends an explicit native pulse.
    Machine menu(g);NativeSetup explicitSetup;explicitSetup.explicitStart(button,f);
    for(unsigned t=0;t<f*3;t+=stride)menu.step(explicitSetup.update(0,t,f),stride);
    mode(*menu.cpu,button,true);printf("PASS TF_104 stride=%u %s: physical and Pause action select matching LCD GAME label\n",stride,button==CTL_START?"A":"B");continue;
   }
   assert(starts==1&&!exhausted(*game.cpu));Bytes active=save(*game.cpu);
   for(unsigned attempt:{unsigned(CTL_START),unsigned(CTL_SELECT)}){
    Machine reference(g),pressed(g),raw(g);load(*reference.cpu,active);load(*pressed.cpu,active);load(*raw.cpu,active);NativeSetup keys;JugglerGate protect(true,false);
    for(unsigned t=0;t<f*3;t+=stride){unsigned k=t>f/4&&(t/(f/4))%2?attempt:0;reference.step(0,stride);pressed.step(protect.update(keys.update(k,t,f),t,f,*pressed.cpu,k),stride);raw.step(k,stride);}
    assert(save(*reference.cpu)==save(*pressed.cpu));assert(save(*reference.cpu)!=save(*raw.cpu));
   }
   Machine reference(g),held(g);load(*reference.cpu,active);load(*held.cpu,active);NativeSetup heldKeys;JugglerGate protect(true,false);unsigned end=0;
   for(unsigned t=0;t<f*150;t+=stride){unsigned k=t>f/4?button:0;reference.step(0,stride);auto out=protect.update(heldKeys.update(k,t,f),t,f,*held.cpu,k);assert(!(out&(CTL_START|CTL_SELECT)));held.step(out,stride);
    if(!end&&exhausted(*reference.cpu))end=t;
    if(end&&t>end+f*2)break;
   }
   assert(end);assert(save(*reference.cpu)==save(*held.cpu));Bytes terminal=save(*held.cpu);
   // A press just BEFORE the final miss must not be queued by chord grace.
   Machine late(g);load(*late.cpu,active);NativeSetup lateKeys;JugglerGate lateGuard(true,false);
   for(unsigned t=0;t<end+f*2;t+=stride){unsigned k=t+f/32>=end?button:0;auto out=lateGuard.update(lateKeys.update(k,t,f),t,f,*late.cpu,k);assert(!(out&(CTL_START|CTL_SELECT)));late.step(out,stride);}
   for(unsigned delay=0;delay<f*2;delay+=f/8)for(unsigned replayButton:{unsigned(CTL_START),unsigned(CTL_SELECT)}){
    Machine replay(g);load(*replay.cpu,terminal);replay.step(0,delay);NativeSetup replayKeys;JugglerGate replayGuard(true,false);unsigned n=0;
    for(unsigned t=0;t<f*3;t+=stride){unsigned k=t>=f?replayButton:0;auto out=replayGuard.update(replayKeys.update(k,t,f),t,f,*replay.cpu,k);n+=replayGuard.consumeStarted();replay.step(out,stride);}
    if(n!=1||exhausted(*replay.cpu)){printf("REPLAY FAIL initial=%u stride=%u replay=%u n=%u end=%f terminal=%d AMPM=%d%d\n",button,stride,replayButton,n,end/double(f),exhausted(*replay.cpu),replay.cpu->get_segments_state(0,11,0),replay.cpu->get_segments_state(0,12,3));fflush(stdout);}assert(n==1&&!exhausted(*replay.cpu));mode(*replay.cpu,replayButton);
   }
   Machine menu(g);NativeSetup menuKeys;menuKeys.explicitStart(button,f);JugglerGate menuGuard(true,false);menuGuard.explicitStart();
   for(unsigned t=0;t<f*3;t+=stride)menu.step(menuGuard.update(menuKeys.update(0,t,f),t,f,*menu.cpu,0),stride);
   mode(*menu.cpu,button);assert(!exhausted(*menu.cpu));
   printf("PASS MB_108 stride=%u initial=%s: both resets reproduced and blocked, held/late keys, native end %.2fs, A/B replay after load, explicit Pause action\n",stride,button==CTL_START?"A":"B",5+end/double(f));fflush(stdout);
  }
 }assert(titles==2);
}
