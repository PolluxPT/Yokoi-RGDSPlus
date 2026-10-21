#pragma once
#include <cstdio>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#include "std/gw_pack.h"
namespace menu {
inline bool english=true;
inline const char* tr(const char*pt,const char*en){return english?en:pt;}
inline void loadLanguage(){english=true;FILE*f=fopen("language.cfg","r");if(f){char b[8]{};if(fscanf(f,"%7s",b)==1)english=std::string(b)!="PT";fclose(f);}}
inline bool saveLanguage(){FILE*f=fopen("language.cfg.new","w");if(!f)return false;bool ok=fputs(english?"EN\n":"PT\n",f)>=0;if(fclose(f))ok=false;return ok&&rename("language.cfg.new","language.cfg")==0;}
inline int segmentPreset=1,backgroundPreset=0;
inline constexpr int segmentAlphas[]={0,5,10};
inline constexpr unsigned backgroundColors[]={0xdbe2bb,0xe0f0ff,0x98a09c,0xffffff,0xe0ffe0,0xfff8dc};
inline const char* segmentName(){const char*pt[]={"Nenhum","Claros","Escuros"};const char*en[]={"None","Light","Dark"};return english?en[segmentPreset]:pt[segmentPreset];}
inline const char* backgroundName(){const char*pt[]={"Amarelo claro","Azul claro","Acinzentado","Branco","Verde claro","Creme"};const char*en[]={"Light yellow","Light blue","Greyish","White","Light green","Cream"};return english?en[backgroundPreset]:pt[backgroundPreset];}
inline void loadDisplay(){FILE*f=fopen("display.cfg","r");if(!f)return;int a,b;if(fscanf(f,"%d %d",&a,&b)==2&&a>=0&&a<3&&b>=0&&b<6){segmentPreset=a;backgroundPreset=b;}fclose(f);}
inline bool saveDisplay(){FILE*f=fopen("display.cfg.new","w");if(!f)return false;bool ok=fprintf(f,"%d %d\n",segmentPreset,backgroundPreset)>0;if(fclose(f))ok=false;return ok&&rename("display.cfg.new","display.cfg")==0;}
inline const char* manufacturerName(int id){switch(id){case 0:return "Nintendo";case 1:return "Tronica";case 2:return "Elektronika";default:return tr("Outros","Other");}}
struct Library {
 std::vector<size_t> games; int manufacturer=0;
 void filter(int id){manufacturer=id;games.clear();for(size_t i=0;i<gw_pack::game_count();i++)if(gw_pack::game_at(i)->manufacturer==id)games.push_back(i);}
 void init(size_t selected){auto*g=gw_pack::game_at(selected);filter(g?g->manufacturer:0);}
 size_t position(size_t selected)const{auto i=std::find(games.begin(),games.end(),selected);return i==games.end()?0:size_t(i-games.begin());}
 size_t move(size_t selected,int direction)const{if(games.empty())return selected;return games[(position(selected)+games.size()+direction)%games.size()];}
 size_t changeManufacturer(size_t selected,int direction){for(int n=0;n<GW_rom::MANUFACTURER_COUNT;n++){int id=(manufacturer+GW_rom::MANUFACTURER_COUNT+direction)%GW_rom::MANUFACTURER_COUNT;filter(id);if(!games.empty())return games.front();}return selected;}
 size_t jump(size_t selected,int direction)const;
};
inline char initial(size_t n){auto*g=gw_pack::game_at(n);if(!g||g->name.empty())return '#';return std::toupper((unsigned char)g->name[0]);}
inline size_t jump(size_t current,int direction){size_t n=gw_pack::game_count();if(!n)return 0;char letter=initial(current);size_t i=current;do{i=(i+n+direction)%n;if(initial(i)!=letter){if(direction<0)while(i>0&&initial(i-1)==initial(i))--i;return i;}}while(i!=current);return current;}
inline size_t Library::jump(size_t selected,int direction)const{if(games.empty())return selected;size_t i=position(selected),start=i;char letter=initial(selected);do{i=(i+games.size()+direction)%games.size();if(initial(games[i])!=letter){if(direction<0)while(i>0&&initial(games[i-1])==initial(games[i]))--i;return games[i];}}while(i!=start);return selected;}
inline std::string pauseItem(int n){switch(n){case 0:return tr("Continuar","Resume");case 1:return tr("Escolher outro jogo","Choose another game");case 2:return tr("Salvar estado","Save state");case 3:return tr("Carregar estado","Load state");case 4:return tr("Gerir slots","Manage slots");case 5:return tr("GAME A - Novo","GAME A - New");case 6:return tr("GAME B - Novo","GAME B - New");default:return tr("Sair","Exit");}}
inline constexpr unsigned bg=0x191b22,highlight=0x79b9ff,fg=0xeef2f5,muted=0xb8c2cc,orange=0xffa45b,selection=0x233b58,line=0x363b45;
}
