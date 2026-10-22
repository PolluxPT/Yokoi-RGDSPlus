#include <cassert>
#include <cstdio>
#include <memory>
#include <vector>
#include <atomic>
#include <cstdlib>
#include <ctime>
#include "std/gw_pack.h"
#include "SM5XX/SM5XX.h"
#include "virtual_i_o/virtual_input.h"
#include "yokoi_cpu_utils.h"
#include "yokoi_input_mapping.h"
#include "clock_sync.h"
extern "C" time_t time(time_t*out) noexcept{time_t n=1792670961;if(out)*out=n;return n;}
using Bytes=std::vector<unsigned char>;
Bytes save(SM5XX&c){FILE*f=tmpfile();assert(c.save_state(f));long n=ftell(f);rewind(f);Bytes b(n);assert(fread(b.data(),1,n,f)==size_t(n));fclose(f);return b;}
void load(SM5XX&c,const Bytes&b){FILE*f=tmpfile();fwrite(b.data(),1,b.size(),f);rewind(f);assert(c.load_state(f));fclose(f);}
Bytes ram(SM5XX&c){Bytes b;for(int x=0;x<c.debug_ram_adress_size_col();x++)for(int y=0;y<c.debug_ram_adress_size_line();y++)b.push_back(c.debug_get_elem_ram(x,y));return b;}
Bytes lcd(SM5XX&c){Bytes b;for(int x=0;x<4;x++)for(int y=0;y<16;y++)for(int z=0;z<4;z++)b.push_back(c.get_segments_state(x,y,z));return b;}
size_t diff(const Bytes&a,const Bytes&b){size_t n=0;for(size_t i=0;i<a.size();i++)n+=a[i]!=b[i];return n;}
int main(int argc,char**argv){assert(argc==2);std::string e;assert(gw_pack::load(argv[1],&e));std::atomic<int>zero{0};
for(size_t i=0;i<gw_pack::game_count();i++){
 auto*g=gw_pack::game_at(i);std::unique_ptr<SM5XX>c;assert(yokoi_cpu_create_instance(c,g->rom,g->size_rom));c->load_rom(g->rom,g->size_rom);c->load_rom_melody(g->melody,g->size_melody);c->init();c->load_rom_time_addresses(g->ref);warmClockRom(c.get(),true);std::unique_ptr<Virtual_Input>in(get_input_config(c.get(),g->ref));if(!in)continue;c->set_input_multiplexage(in->use_multiplexage);
 for(unsigned t=0;t<c->frequency*8;t+=32){yokoi_input_apply_controller_mask(in.get(),t>=c->frequency*3&&t<c->frequency*13/4?CTL_START:0,0,zero,zero);for(int n=0;n<32;n++)c->step();}
 Bytes state=save(*c);bool asleep=c->sleeping();Bytes referenceRam,referenceLcd;
 printf("%s %s sleep=%d",g->ref.c_str(),g->name.c_str(),asleep);
 for(unsigned setup:{0u,unsigned(CTL_START),unsigned(CTL_SELECT)}){
  load(*c,state);for(unsigned t=0;t<c->frequency*2;t+=32){yokoi_input_apply_controller_mask(in.get(),t<c->frequency/4?setup:0,0,zero,zero);for(int n=0;n<32;n++)c->step();}
  auto r=ram(*c),l=lcd(*c);if(!setup){referenceRam=r;referenceLcd=l;continue;}printf(" %s ramdiff=%zu lcddiff=%zu",setup==CTL_START?"START":"SELECT",diff(referenceRam,r),diff(referenceLcd,l));
 }
 puts("");
}
}
