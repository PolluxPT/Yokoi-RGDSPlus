#include <cassert>
#include <filesystem>
#include <fstream>
#include "std/gw_pack.h"
#include "yokoi_cpu_utils.h"
#include "state_store.h"
#include "emu_worker.h"
#include "touch_input.h"
struct Recording:Virtual_Input{bool rightLeft=false,gameA=false,gameB=false;Recording(SM5XX*c):Virtual_Input(c){left_configuration=CONF_2_BUTTON_UPDOWN;right_configuration=CONF_2_BUTTON_LEFTRIGHT;}
 void set_input(uint8_t part,uint8_t button,bool state,uint8_t=1)override{if(part==PART_RIGHT&&button==BUTTON_LEFT)rightLeft=state;if(part==PART_SETUP&&button==BUTTON_GAMEA)gameA=state;if(part==PART_SETUP&&button==BUTTON_GAMEB)gameB=state;}
};
int main(int argc,char**argv){assert(argc>1);std::string pack=std::filesystem::absolute(argv[1]);std::string error;assert(gw_pack::load(pack,&error));
 char folder[]="/tmp/yokoi-states-XXXXXX";assert(mkdtemp(folder));assert(chdir(folder)==0);
 for(int index:{0,10,34,35,49,55}){
  auto*g=gw_pack::game_at(index);std::unique_ptr<SM5XX>cpu;assert(yokoi_cpu_create_instance(cpu,g->rom,g->size_rom));cpu->load_rom(g->rom,g->size_rom);cpu->load_rom_melody(g->melody,g->size_melody);cpu->init();cpu->load_rom_time_addresses(g->ref);cpu->time_set(true);
  std::unique_ptr<Virtual_Input>input(get_input_config(cpu.get(),g->ref));if(input)cpu->set_input_multiplexage(input->use_multiplexage);
  std::atomic<int>zero{0};for(int n=0;n<70000;n++){if(n%32==0)yokoi_input_apply_controller_mask(input.get(),n>32768&&n<35000?CTL_START:0,0,zero,zero);cpu->step();}
  assert(state_store::save(cpu.get(),g,1));
  auto run=[&](){for(int n=0;n<8192;n++){if(n%32==0)yokoi_input_apply_controller_mask(input.get(),(n/256)%2?CTL_DPAD_LEFT|CTL_Y:CTL_A|CTL_DPAD_UP,0,zero,zero);cpu->step();}};
  run();std::vector<uint8_t>expected,actual;assert(state_store::capture(cpu.get(),expected));assert(state_store::load(cpu.get(),g,1));run();assert(state_store::capture(cpu.get(),actual));assert(expected==actual);
  auto name=state_store::path(g,1);std::fstream file(name,std::ios::in|std::ios::out|std::ios::binary);file.seekg(-1,std::ios::end);char ch;file.get(ch);ch^=1;file.seekp(-1,std::ios::end);file.put(ch);file.close();assert(!state_store::load(cpu.get(),g,1));actual.clear();assert(state_store::capture(cpu.get(),actual));assert(actual==expected);
  printf("PASS: full save/reload/replay and corruption rollback: %s (%zu bytes)\n",g->name.c_str(),actual.size());
 }
 Recording recording(nullptr);std::atomic<int>zero{0};yokoi_input_apply_controller_mask(&recording,CTL_Y,0,zero,zero);assert(recording.rightLeft);yokoi_input_apply_controller_mask(&recording,CTL_DPAD_LEFT,0,zero,zero);assert(recording.rightLeft);
 TouchGesture gesture;gesture.down(.8,.5,100);assert(gesture.up(.2,.5,220,1).swipe==1);gesture.down(.5,.5,500);assert(gesture.up(.5,.5,550,1).tap);gesture.down(.5,.5,700);assert(gesture.up(.5,.5,750,1).doubled);
 std::filesystem::remove_all("saves");assert(chdir("/")==0);rmdir(folder);puts("PASS: Donald/Squish right-player left input and swipe/double tap");
}
