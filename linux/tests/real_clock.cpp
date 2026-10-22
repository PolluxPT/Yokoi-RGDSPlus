// Optional validation with the user's local pack. No game data is included.
#define main yokoi_program
#include "../main.cpp"
#undef main
#include <cassert>
int main(int argc,char**argv){setvbuf(stdout,nullptr,_IONBF,0);assert(argc==2);std::string error;assert(gw_pack::load(argv[1],&error));int tested=0;
 for(size_t i=0;i<gw_pack::game_count();i++){auto*g=gw_pack::game_at(i);std::string ref=g->ref;std::replace(ref.begin(),ref.end(),'-','_');if(ref!="AC_01"&&ref!="DK_52"&&ref!="JR_55"&&ref!="MB_108"&&ref!="IM_53"&&ref!="SM_91")continue;
  auto*map=get_time_addresses(ref);assert(map);std::unique_ptr<SM5XX>cpu;assert(yokoi_cpu_create_instance(cpu,g->rom,g->size_rom));cpu->load_rom(g->rom,g->size_rom);cpu->load_rom_melody(g->melody,g->size_melody);cpu->init();cpu->load_rom_time_addresses(ref);warmClockRom(cpu.get(),true);std::unique_ptr<Virtual_Input>input(get_input_config(cpu.get(),g->ref));if(input)cpu->set_input_multiplexage(input->use_multiplexage);
  EmuWorker worker;std::vector<Segment>segs(g->segment,g->segment+g->size_segment);worker.start(cpu.get(),input.get(),segs,0,nullptr,-1,true);SDL_Delay(1250);worker.stop();
  FILE*f=tmpfile();assert(f);assert(cpu->save_common_state(f));long common=ftell(f);fclose(f);std::vector<uint8_t>state;assert(state_store::capture(cpu.get(),state));int stride=cpu->get_cpu_type_id()==0?13:16;auto digit=[&](int c,int l){return state.at(common+c*stride+l);};time_t n=time(nullptr);tm now{};localtime_r(&n,&now);int h=now.tm_hour,pm=0;if(map->pm_bit!=24){if(h>=12)pm=map->pm_bit;h%=12;if(!h)h=12;}
  assert(digit(map->col_hour_tens,map->line_hour_tens)==((h/10)|pm));assert(digit(map->col_hour_units,map->line_hour_units)==h%10);assert(digit(map->col_minute_tens,map->line_minute_tens)==now.tm_min/10);assert(digit(map->col_minute_units,map->line_minute_units)==now.tm_min%10);printf("PASS: clock RAM matches Linux local time after boot: %s (%s)\n",g->name.c_str(),g->ref.c_str());tested++;
 }printf("Clock titles tested: %d\n",tested);assert(tested>=4);gw_pack::unload();
}
