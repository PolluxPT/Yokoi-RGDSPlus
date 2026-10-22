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
extern "C" time_t time(time_t*out) noexcept{time_t n=1792670961;if(out)*out=n;return n;}
using Bytes=std::vector<unsigned char>;
Bytes save(SM5XX&c){FILE*f=tmpfile();assert(c.save_state(f));long n=ftell(f);rewind(f);Bytes b(n);assert(fread(b.data(),1,n,f)==size_t(n));fclose(f);return b;}
void load(SM5XX&c,const Bytes&b){FILE*f=tmpfile();fwrite(b.data(),1,b.size(),f);rewind(f);assert(c.load_state(f));fclose(f);}
struct Machine{
 std::unique_ptr<SM5XX>cpu;std::unique_ptr<Virtual_Input>input;std::atomic<int>zero{0};
 explicit Machine(const GW_rom*g){assert(yokoi_cpu_create_instance(cpu,g->rom,g->size_rom));cpu->load_rom(g->rom,g->size_rom);cpu->load_rom_melody(g->melody,g->size_melody);cpu->init();cpu->load_rom_time_addresses(g->ref);warmClockRom(cpu.get(),true);input.reset(get_input_config(cpu.get(),g->ref));assert(input);cpu->set_input_multiplexage(input->use_multiplexage);}
 void step(uint32_t keys,unsigned count){yokoi_input_apply_controller_mask(input.get(),keys,0,zero,zero);for(unsigned i=0;i<count;i++)cpu->step();}
};
int main(int argc,char**argv){assert(argc==2);std::string error;assert(gw_pack::load(argv[1],&error));unsigned titles=0,nativeTitles=0;
 for(size_t i=0;i<gw_pack::game_count();i++){
  auto*g=gw_pack::game_at(i);Machine initial(g);if(!initial.input->single_game_button){
   if(g->ref=="MB_108"||g->ref=="TF_104")continue;
   Machine native(g),filtered(g);load(*filtered.cpu,save(*native.cpu));NativeSetup a,b;SingleGameGate passthrough(false,true,*filtered.cpu);auto f=native.cpu->frequency;
   for(unsigned t=0;t<f*6;t+=256){unsigned k=t<f?0:((t/(f/4))%2?CTL_START:CTL_SELECT)|CTL_A;native.step(a.update(k,t,f),256);filtered.step(passthrough.update(b.update(k,t,f),t,f,*filtered.cpu,k),256);}
   assert(save(*native.cpu)==save(*filtered.cpu));nativeTitles++;continue;
  }titles++;
  for(unsigned stride:{32u,256u}){
   auto f=initial.cpu->frequency;
   Machine game(g);NativeSetup transport;SingleGameGate gate(true,true,*game.cpu);unsigned starts=0;
   // Open in Watch, ignore SELECT, start with START and hold it through the
   // beginning of play. A single short native pulse must be delivered.
   for(unsigned t=0;t<f*5;t+=stride){unsigned k=t>=f&&t<f*2?CTL_SELECT:t>=f*3?CTL_START:0;auto out=gate.update(transport.update(k,t,f),t,f,*game.cpu,k);starts+=gate.consumeStarted();game.step(out,stride);}
   assert(starts==1);Bytes active=save(*game.cpu);
   Machine expected(g),pressed(g),raw(g);load(*expected.cpu,active);load(*pressed.cpu,active);load(*raw.cpu,active);
   NativeSetup pressTransport;SingleGameGate activeGate(true,false,*pressed.cpu);
   // Compare complete CPU snapshots, not just the guard's bookkeeping. Every
   // attempted reset during active play must produce exactly the same run.
   for(unsigned t=0;t<f*3;t+=stride){unsigned k=t>f/4&&(t/(f/4))%2?CTL_START:0;expected.step(0,stride);pressed.step(activeGate.update(pressTransport.update(k,t,f),t,f,*pressed.cpu,k),stride);raw.step(k,stride);}
   assert(save(*expected.cpu)==save(*pressed.cpu));
   if(g->ref.rfind("BF_",0)==0||g->ref.rfind("DR_",0)==0)assert(save(*expected.cpu)!=save(*raw.cpu)); // reproduces v1.4
   // Reload/resume an active snapshot: no saved frontend state or idle counter
   // is needed. Keep START held all the way through native Game Over.
   Machine endReference(g),endPressed(g);load(*endReference.cpu,active);load(*endPressed.cpu,active);
   NativeSetup endTransport;SingleGameGate endGate(true,false,*endPressed.cpu);
   auto seq=endReference.cpu->idle_sequence_number();bool ended=false;unsigned endAt=0;
   for(unsigned t=0;t<f*100;t+=stride){
    unsigned movement=0;if(g->ref.rfind("DR_",0)==0){unsigned p=t/(f/4);movement=(p%8<4?CTL_DPAD_RIGHT:CTL_DPAD_LEFT)|(p%2?CTL_A:0);}
    endReference.step(movement,stride);
    auto out=endGate.update(endTransport.update(movement|(t>f/4?CTL_START:0),t,f),t,f,*endPressed.cpu,movement|(t>f/4?CTL_START:0));
    assert(!(out&CTL_START));assert(!endGate.consumeStarted());endPressed.step(out,stride);
    if(!ended&&endReference.cpu->idle_sequence_number()!=seq){ended=true;endAt=t;}
    if(ended&&t>endAt+f*2)break;
   }
   assert(ended);assert(save(*endReference.cpu)==save(*endPressed.cpu));
   Machine late(g);load(*late.cpu,active);NativeSetup lateTransport;SingleGameGate lateGate(true,false,*late.cpu);
   for(unsigned t=0;t<endAt+f*2;t+=stride){
    unsigned movement=0;if(g->ref.rfind("DR_",0)==0){unsigned p=t/(f/4);movement=(p%8<4?CTL_DPAD_RIGHT:CTL_DPAD_LEFT)|(p%2?CTL_A:0);}
    unsigned k=movement|(t+f/32>=endAt?CTL_START:0);
    auto out=lateGate.update(lateTransport.update(k,t,f),t,f,*late.cpu,k);assert(!(out&CTL_START));assert(!lateGate.consumeStarted());late.step(out,stride);
   }
   Bytes terminal=save(*endPressed.cpu);
   // Resume/load at Game Over and require a released then newly pressed START.
   Machine replay(g),noReplay(g);load(*replay.cpu,terminal);load(*noReplay.cpu,terminal);
   NativeSetup replayTransport;SingleGameGate replayGate(true,false,*replay.cpu);unsigned replayStarts=0;
   for(unsigned t=0;t<f*3;t+=stride){unsigned k=t>=f&&t<f*3/2?CTL_START:0;auto out=replayGate.update(replayTransport.update(k,t,f),t,f,*replay.cpu,k);replayStarts+=replayGate.consumeStarted();replay.step(out,stride);noReplay.step(0,stride);}
   assert(replayStarts==1);assert(save(*replay.cpu)!=save(*noReplay.cpu));
   // Explicit Pause > GAME is an intentional new game even during a run.
   SingleGameGate explicitGate(true,false,*replay.cpu);explicitGate.explicitStart();assert(explicitGate.update(CTL_START,0,f,*replay.cpu,0)&CTL_START);
   printf("PASS %s stride=%u: Watch/start, active reset blocked, held-through-end, reload and replay (native end %.2fs)\n",g->ref.c_str(),stride,5+endAt/double(f));fflush(stdout);
  }
 }
 assert(titles==6);assert(nativeTitles==48);printf("PASS all %u remaining ROMs: complete CPU state identical to native v1.4 input\n",nativeTitles);
 // GAME A/B mappings retain their native behaviour, including Donkey Kong.
 auto*g=gw_pack::game_at(0);Machine generic(g);SingleGameGate passthrough(false,false,*generic.cpu);
 assert(passthrough.update(CTL_START|CTL_SELECT|CTL_A,0,generic.cpu->frequency,*generic.cpu,0)==(CTL_START|CTL_SELECT|CTL_A));
 puts("PASS native GAME A/B passthrough unchanged");
}
