#include <cassert>
#include <cstdio>
#include <memory>
#include <atomic>
#include "std/gw_pack.h"
#include "virtual_i_o/virtual_input.h"
#include "yokoi_cpu_utils.h"
#include "yokoi_input_mapping.h"
#include "../native_setup.h"
int main(int argc,char**argv){
 assert(argc==2);std::string error;assert(gw_pack::load(argv[1],&error));std::atomic<int>zero{0};int titles=0;
 for(size_t i=0;i<gw_pack::game_count();i++){
  auto*g=gw_pack::game_at(i);if(g->ref!="DR_106"&&g->ref!="DR_802"&&g->ref!="BF_107"&&g->ref!="BF_803")continue;
  std::unique_ptr<SM5XX>cpu;assert(yokoi_cpu_create_instance(cpu,g->rom,g->size_rom));cpu->load_rom(g->rom,g->size_rom);cpu->init();std::unique_ptr<Virtual_Input>input(get_input_config(cpu.get(),g->ref));assert(input);
  NativeSetup keys;keys.update(0,0,cpu->frequency);bool gameSeen=false;
  for(unsigned t=32;t<cpu->frequency;t+=32){auto mask=keys.update(CTL_SELECT,t,cpu->frequency);yokoi_input_apply_controller_mask(input.get(),mask,0,zero,zero);assert(!(cpu->debug_k_input(0)&2));}
  keys.update(0,cpu->frequency,cpu->frequency);
  for(unsigned t=cpu->frequency+32;t<cpu->frequency*2;t+=32){auto mask=keys.update(CTL_START,t,cpu->frequency);yokoi_input_apply_controller_mask(input.get(),mask,0,zero,zero);gameSeen|=bool(cpu->debug_k_input(0)&2);}
  assert(gameSeen);printf("PASS %s: SELECT does not press GAME; START does\n",g->ref.c_str());titles++;
 }assert(titles==4);gw_pack::unload();
}
