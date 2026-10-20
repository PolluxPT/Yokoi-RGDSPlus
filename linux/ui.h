#pragma once
#include <SDL.h>
#include <string>
#include <vector>
#include "third_party/lodepng.h"
#include "ui_font_data.h"
namespace ui {
inline std::vector<unsigned> codepoints(const std::string&s){std::vector<unsigned>out;for(size_t i=0;i<s.size();){unsigned c=(uint8_t)s[i++];if(c>=0xc0&&c<0xe0&&i<s.size()){c=((c&31)<<6)|((uint8_t)s[i++]&63);}else if(c>=0xe0){while(i<s.size()&&((uint8_t)s[i]&0xc0)==0x80)i++;c='?';}out.push_back(c>=32&&c<256?c:'?');}return out;}
inline int width(const std::string&s,int face){int w=0;for(auto c:codepoints(s))w+=ui_glyphs[face][c-32].advance;return w;}
inline void rect(SDL_Renderer*r,int x,int y,int w,int h,uint32_t rgb,int alpha=255){SDL_SetRenderDrawBlendMode(r,SDL_BLENDMODE_BLEND);SDL_SetRenderDrawColor(r,rgb>>16,(rgb>>8)&255,rgb&255,alpha);SDL_Rect b{x,y,w,h};SDL_RenderFillRect(r,&b);}
inline void label(SDL_Renderer*r,SDL_Texture*&font,int x,int y,std::string s,int face=1,uint32_t rgb=0xeef2f5,int maxWidth=960){
 if(!font){std::vector<uint8_t>b;unsigned w,h;if(lodepng::decode(b,w,h,ui_font_png,sizeof ui_font_png))return;font=SDL_CreateTexture(r,SDL_PIXELFORMAT_ABGR8888,SDL_TEXTUREACCESS_STATIC,w,h);if(!font)return;SDL_UpdateTexture(font,nullptr,b.data(),w*4);SDL_SetTextureBlendMode(font,SDL_BLENDMODE_BLEND);}
 if(width(s,face)>maxWidth){while(!s.empty()&&width(s+"...",face)>maxWidth){s.pop_back();while(!s.empty()&&((uint8_t)s.back()&0xc0)==0x80)s.pop_back();}s+="...";}
 SDL_SetTextureColorMod(font,rgb>>16,(rgb>>8)&255,rgb&255);SDL_SetTextureAlphaMod(font,255);
 for(auto c:codepoints(s)){auto g=ui_glyphs[face][c-32];SDL_Rect src{g.x,g.y,g.w,g.h},dst{x,y,g.w,g.h};SDL_RenderCopy(r,font,&src,&dst);x+=g.advance;}
}
}
