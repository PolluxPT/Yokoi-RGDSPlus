#pragma once
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <filesystem>
#include <map>
#include <chrono>
#include <sys/stat.h>
#include <unistd.h>
#include "std/GW_ROM.h"
#include "SM5XX/SM5XX.h"

namespace state_store {
inline uint32_t hash(const uint8_t*p,size_t n){uint32_t h=2166136261u;while(n--)h=(h^*p++)*16777619u;return h;}
inline std::string path(const GW_rom*g,int slot){return "saves/"+g->ref+".slot"+std::to_string(slot)+".yks";}
inline bool exists(const GW_rom*g,int slot){return std::filesystem::is_regular_file(path(g,slot));}
inline bool erase(const GW_rom*g,int slot){return std::remove(path(g,slot).c_str())==0;}
inline std::string stamp(const GW_rom*g,int slot){using Clock=std::chrono::steady_clock;static std::map<std::string,std::pair<Clock::time_point,std::string>>cache;auto p=path(g,slot);auto now=Clock::now();auto i=cache.find(p);if(i!=cache.end()&&now-i->second.first<std::chrono::seconds(1))return i->second.second;
 struct stat st{};std::string value="Vazio";if(!stat(p.c_str(),&st)){char s[48];strftime(s,sizeof s,"%d/%m %H:%M",localtime(&st.st_mtime));value=s;}cache[p]={now,value};return value;}
inline bool capture(SM5XX*c,std::vector<uint8_t>&out){FILE*f=tmpfile();if(!f)return false;bool ok=c->save_state(f);long n=ftell(f);if(n<=0||n>16384)ok=false;if(ok){out.resize(n);rewind(f);ok=fread(out.data(),1,n,f)==size_t(n);}fclose(f);return ok;}
inline bool restore(SM5XX*c,const std::vector<uint8_t>&data){FILE*f=tmpfile();if(!f)return false;bool ok=fwrite(data.data(),1,data.size(),f)==data.size();rewind(f);if(ok)ok=c->load_state(f);fclose(f);return ok;}
struct Header {char magic[8];uint32_t version,cpu,romHash,size,checksum;char ref[48];};
inline bool save(SM5XX*c,const GW_rom*g,int slot){
 std::vector<uint8_t>data;if(!capture(c,data))return false;
 Header h{};memcpy(h.magic,"YKRGDS4",7);h.version=1;h.cpu=c->get_cpu_type_id();h.romHash=hash(g->rom,g->size_rom);h.size=data.size();h.checksum=hash(data.data(),data.size());snprintf(h.ref,sizeof h.ref,"%s",g->ref.c_str());
 std::error_code ec;std::filesystem::create_directories("saves",ec);std::string p=path(g,slot),temp=p+".new";FILE*f=fopen(temp.c_str(),"wb");if(!f)return false;
 bool ok=fwrite(&h,sizeof h,1,f)==1&&fwrite(data.data(),1,data.size(),f)==data.size();if(fflush(f)||fsync(fileno(f)))ok=false;if(fclose(f))ok=false;if(ok)ok=rename(temp.c_str(),p.c_str())==0;if(!ok)remove(temp.c_str());return ok;
}
inline bool load(SM5XX*c,const GW_rom*g,int slot){
 FILE*f=fopen(path(g,slot).c_str(),"rb");if(!f)return false;Header h{};
 bool ok=fread(&h,sizeof h,1,f)==1&&!memcmp(h.magic,"YKRGDS4",8)&&h.version==1&&h.cpu==c->get_cpu_type_id()&&h.romHash==hash(g->rom,g->size_rom)&&!strncmp(h.ref,g->ref.c_str(),sizeof h.ref)&&h.size>0&&h.size<=16384;
 std::vector<uint8_t>data,backup;if(ok){data.resize(h.size);ok=fread(data.data(),1,data.size(),f)==data.size()&&fgetc(f)==EOF&&hash(data.data(),data.size())==h.checksum;}fclose(f);
 if(!ok||!capture(c,backup)||backup.size()!=data.size())return false;
 if(!restore(c,data)){restore(c,backup);return false;}return true;
}
}
