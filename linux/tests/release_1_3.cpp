#include <SDL.h>
#include <cassert>
#include <filesystem>
#include <fstream>
static int opens=0,queues=0;static bool testAudio=false;
static SDL_AudioDeviceID negotiatedOpen(const char*n,int capture,const SDL_AudioSpec*w,SDL_AudioSpec*h,int allowed){
 if(!testAudio)return SDL_OpenAudioDevice(n,capture,w,h,allowed);
 ++opens;assert(allowed&SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
 if(w->channels==1){SDL_SetError("ALSA: Couldn't set audio channels");return 0;}
 SDL_AudioSpec stereo=*w;stereo.channels=2;stereo.freq=44100;stereo.format=AUDIO_F32SYS;
 return SDL_OpenAudioDevice(n,capture,&stereo,h,0);
}
static int checkedQueue(SDL_AudioDeviceID d,const void*p,Uint32 bytes){if(testAudio){assert(bytes%8==0);if(queues==0)assert(bytes==14112);++queues;}return SDL_QueueAudio(d,p,bytes);}
#define SDL_OpenAudioDevice negotiatedOpen
#define SDL_QueueAudio checkedQueue
#define main yokoi_program
#include "../main.cpp"
#undef main
#undef SDL_OpenAudioDevice
#undef SDL_QueueAudio
int main(int argc,char**argv){
 assert(argc==2);std::string pack=std::filesystem::absolute(argv[1]);char dir[]="/tmp/yokoi-13-XXXXXX";assert(mkdtemp(dir));assert(chdir(dir)==0);
 menu::english=false;menu::loadLanguage();assert(menu::english);menu::english=false;assert(menu::saveLanguage());menu::english=true;menu::loadLanguage();assert(!menu::english);
 for(int language=0;language<2;language++){menu::english=language;assert(std::string(menu::segmentName())==(language?"Light":"Claros"));assert(std::string(menu::backgroundName())==(language?"Light yellow":"Amarelo claro"));}
 menu::segmentPreset=2;menu::backgroundPreset=5;assert(menu::saveDisplay());menu::segmentPreset=0;menu::backgroundPreset=0;menu::loadDisplay();assert(menu::segmentPreset==2&&menu::backgroundPreset==5);
 std::string error;assert(gw_pack::load(pack,&error));menu::Library library;library.init(0);assert(library.games.size()==2);auto first=library.games[0];assert(library.move(library.move(first,1),1)==first);assert(library.jump(first,1)==library.games[1]);
 auto next=library.changeManufacturer(first,1);assert(library.manufacturer==1&&library.games.size()==1);next=library.changeManufacturer(next,1);assert(library.manufacturer==2);next=library.changeManufacturer(next,1);assert(library.manufacturer==0);next=library.changeManufacturer(next,-1);assert(library.manufacturer==2);
 assert(SDL_Init(SDL_INIT_VIDEO)==0);Panel p;p.w=SDL_CreateWindow("test",0,0,1024,768,0);p.r=SDL_CreateRenderer(p.w,-1,SDL_RENDERER_SOFTWARE|SDL_RENDERER_TARGETTEXTURE);assert(p.r);clear(p);
 uint16_t info[]={128,128,1,0,200,100};GW_rom g("Test","Test","2026",nullptr,0,nullptr,0,"",nullptr,0,info,"",nullptr,"",nullptr);std::vector<Segment>segs;
 auto pixel=[&](int x,int y){Uint32 color;SDL_Rect rect{x,y,1,1};assert(SDL_RenderReadPixels(p.r,&rect,SDL_PIXELFORMAT_ARGB8888,&color,4)==0);return color&0xffffff;};
 for(int bg=0;bg<6;bg++){if(p.staticLayer){SDL_DestroyTexture(p.staticLayer);p.staticLayer=nullptr;}menu::backgroundPreset=bg;drawGame(p,&g,segs,0,false);assert(pixel(512,0)==0&&pixel(512,767)==0);assert(pixel(512,384)==menu::backgroundColors[bg]);drawGame(p,&g,segs,0,false);assert(pixel(512,0)==0);}
 drop(p);menu::backgroundPreset=3;info[3]=0;Uint32 white=0xffffffff;p.seg=SDL_CreateTexture(p.r,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_STATIC,1,1);assert(p.seg);SDL_UpdateTexture(p.seg,nullptr,&white,4);SDL_SetTextureBlendMode(p.seg,SDL_BLENDMODE_BLEND);
 Segment segment{};segment.pos_scr[0]=90;segment.pos_scr[1]=40;segment.size_tex[0]=segment.size_tex[1]=1;segs={segment};unsigned previous=0xffffff;
 for(int setting=0;setting<3;setting++){if(p.staticLayer){SDL_DestroyTexture(p.staticLayer);p.staticLayer=nullptr;}menu::segmentPreset=setting;drawGame(p,&g,segs,0,false);unsigned mark=pixel(462,334);if(setting==0)assert(mark==0xffffff);else assert(mark<previous);previous=mark;}
 segs.clear();info[3]=1;drop(p);drawGame(p,&g,segs,0,false);assert(pixel(512,0)==0&&pixel(512,384)==0x080908);drop(p);SDL_DestroyRenderer(p.r);SDL_DestroyWindow(p.w);SDL_Quit();gw_pack::unload();
 testAudio=true;setenv("YOKOI_TEST_FRAMES","90",1);char name[]="test",test[]="--test-game";char*args[]={name,test,pack.data(),nullptr};assert(yokoi_program(3,args)==0);assert(opens==2&&queues>2);
 std::filesystem::remove_all(dir);puts("PASS: EN first-run/PT preference, display persistence/PT-EN labels, manufacturer filter/wrap/jumps, black bars/cache/mask/all six colors, mono rejection -> stereo float 44.1kHz conversion + correct prefill");
}
