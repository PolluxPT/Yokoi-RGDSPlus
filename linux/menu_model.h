#pragma once
#include <cstdio>
#include <string>
#include <algorithm>
#include <cctype>
#include "std/gw_pack.h"
namespace menu {
inline bool english=false;
inline const char* tr(const char*pt,const char*en){return english?en:pt;}
inline void loadLanguage(){FILE*f=fopen("language.cfg","r");if(f){char b[8]{};if(fscanf(f,"%7s",b)==1)english=std::string(b)=="EN";fclose(f);}}
inline bool saveLanguage(){FILE*f=fopen("language.cfg.new","w");if(!f)return false;bool ok=fputs(english?"EN\n":"PT\n",f)>=0;if(fclose(f))ok=false;return ok&&rename("language.cfg.new","language.cfg")==0;}
inline char initial(size_t n){auto*g=gw_pack::game_at(n);if(!g||g->name.empty())return '#';return std::toupper((unsigned char)g->name[0]);}
inline size_t jump(size_t current,int direction){size_t n=gw_pack::game_count();if(!n)return 0;char letter=initial(current);size_t i=current;do{i=(i+n+direction)%n;if(initial(i)!=letter){if(direction<0)while(i>0&&initial(i-1)==initial(i))--i;return i;}}while(i!=current);return current;}
inline std::string pauseItem(int n){switch(n){case 0:return tr("Continuar","Resume");case 1:return tr("Escolher outro jogo","Choose another game");case 2:return tr("Salvar estado","Save state");case 3:return tr("Carregar estado","Load state");case 4:return tr("Gerir slots","Manage slots");case 5:return tr("GAME A - Novo","GAME A - New");case 6:return tr("GAME B - Novo","GAME B - New");default:return tr("Sair","Exit");}}
inline constexpr unsigned bg=0x191b22,highlight=0x79b9ff,fg=0xeef2f5,muted=0xb8c2cc,orange=0xffa45b,selection=0x233b58,line=0x363b45;
}
