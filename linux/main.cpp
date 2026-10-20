// Linux SDL2 frontend for Yokoi. Experimental RGDS+ port.
#include <SDL.h>
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <signal.h>
#include <time.h>
#include <cerrno>
#include "std/gw_pack.h"
#include "std/platform_paths.h"
#include "virtual_i_o/virtual_input.h"
#include "yokoi_cpu_utils.h"
#include "yokoi_input_mapping.h"
#include "third_party/lodepng.h"
#include "ui.h"
#include "menu_model.h"
using menu::tr;
#include "state_store.h"
#include "touch_input.h"
#include "rgds_input.h"
#include "rgds_volume.h"
#include "button_profile.h"
#include "emu_worker.h"
static volatile sig_atomic_t stopped=0;
static void stop(int){stopped=1;}
struct Panel {SDL_Window* w=nullptr; SDL_Renderer* r=nullptr; SDL_Texture *seg=nullptr,*bg=nullptr,*console=nullptr,*staticLayer=nullptr,*font=nullptr;int cachedScreen=-1;bool cachedCombined=false;};
static Panel panels[2];
static int np=0;
static bool swapPanels=false;
static void text(Panel&p,int x,int y,const std::string&s,int scale=2){ui::label(p.r,p.font,x,y,s,scale>=5?4:scale>=3?3:1);}
static void label(Panel&p,int x,int y,const std::string&s,int face=1,uint32_t color=0xeef2f5,int width=940){ui::label(p.r,p.font,x,y,s,face,color,width);}
static void selectionRow(Panel&p,int x,int y,int w,int h,bool selected){if(selected){ui::rect(p.r,x,y,w,h,menu::selection);ui::rect(p.r,x,y,5,h,menu::highlight);}}
static void menuHeader(Panel&p,int x,int y,int w,const std::string&title,int face=2){label(p,x,y,title,face,menu::fg,w);ui::rect(p.r,x,y+62,w,1,menu::line);}
static void clear(Panel& p){SDL_RenderSetLogicalSize(p.r,1024,768);SDL_SetRenderDrawColor(p.r,25,27,34,255);SDL_RenderClear(p.r);}
static void drop(Panel& p){for(auto t:{p.seg,p.bg,p.console,p.staticLayer})if(t)SDL_DestroyTexture(t);p.seg=p.bg=p.console=p.staticLayer=nullptr;p.cachedScreen=-1;}
static SDL_Texture* texture(Panel& p,const std::string& path,bool white=false){
 if(path.empty())return nullptr;
 const uint8_t* data=nullptr;size_t n=0; std::string name=path.substr(path.find_last_of("/\\")+1); if(!gw_pack::get_file_bytes(name,data,n)){fprintf(stderr,"Texture missing: %s\n",name.c_str());return nullptr;}
 unsigned w=0,h=0;std::vector<unsigned char> rgba;unsigned err=lodepng::decode(rgba,w,h,data,n);
 if(err){fprintf(stderr,"PNG %s: %s\n",path.c_str(),lodepng_error_text(err));return nullptr;}
 if(white)for(size_t i=0;i<rgba.size();i+=4)rgba[i]=rgba[i+1]=rgba[i+2]=255;
 auto t=SDL_CreateTexture(p.r,SDL_PIXELFORMAT_ABGR8888,SDL_TEXTUREACCESS_STATIC,w,h);
 if(t){SDL_UpdateTexture(t,nullptr,rgba.data(),w*4);SDL_SetTextureBlendMode(t,SDL_BLENDMODE_BLEND);}return t;
}
static void fit(Panel&p,SDL_Texture*t,const SDL_Rect&src,float x,float y,float w,float h,float cw,float ch){
 if(!t||cw<=0||ch<=0)return;float z=std::min(1024.f/cw,768.f/ch);
 SDL_Rect dst{int((1024-cw*z)/2+x*z),int((768-ch*z)/2+y*z),std::max(1,int(w*z)),std::max(1,int(h*z))};int tw=0,th=0;SDL_QueryTexture(t,nullptr,nullptr,&tw,&th);
 // Yokoi pack rectangles use bottom-left Y, and each source image is mirrored.
 SDL_Rect crop=src;crop.y=th-src.y-src.h;
 SDL_RenderCopyEx(p.r,t,&crop,&dst,0,nullptr,SDL_FLIP_HORIZONTAL);
}
static void drawGame(Panel&p,const GW_rom*g,const std::vector<Segment>&segs,int screen,bool combined,int pass=0){
 if(pass==0){
  if(p.staticLayer&&(p.cachedScreen!=screen||p.cachedCombined!=combined)){SDL_DestroyTexture(p.staticLayer);p.staticLayer=nullptr;}
  if(!p.staticLayer){
   SDL_RendererInfo info{};SDL_GetRendererInfo(p.r,&info);
   if(info.flags&SDL_RENDERER_TARGETTEXTURE){
    auto*t=SDL_CreateTexture(p.r,SDL_PIXELFORMAT_ARGB8888,SDL_TEXTUREACCESS_TARGET,1024,768);
    if(t){auto*old=SDL_GetRenderTarget(p.r);if(SDL_SetRenderTarget(p.r,t)==0){drawGame(p,g,segs,screen,combined,1);SDL_SetRenderTarget(p.r,old);p.staticLayer=t;p.cachedScreen=screen;p.cachedCombined=combined;}else SDL_DestroyTexture(t);}
   }
  }
  if(p.staticLayer)SDL_RenderCopy(p.r,p.staticLayer,nullptr,nullptr);else drawGame(p,g,segs,screen,combined,1);
  drawGame(p,g,segs,screen,combined,2);return;
 }
 const auto si=g->segment_info;float w0=si[4],h0=si[5];float w1=combined?si[6]:0,h1=combined?si[7]:0;
 float cw=combined?w0+w1:si[4+2*screen],ch=combined?std::max(h0,h1):si[5+2*screen];
 if(pass==1){bool dark=(si[3]&1)!=0;SDL_SetRenderDrawColor(p.r,dark?8:184,dark?9:192,dark?8:165,255);SDL_RenderClear(p.r);
 for(int s=0;s<2;s++){if((!combined&&s!=screen)||(s==1&&!combined&&screen==0))continue;
  float x=combined&&s==1?w0:0,w=si[4+s*2],h=si[5+s*2];
  if(p.bg&&g->background_info){auto bi=g->background_info+2+s*4;SDL_Rect src{bi[0],bi[1],bi[2],bi[3]};fit(p,p.bg,src,x,0,w,h,cw,ch);}
 }
 }
 static const uint32_t colors[]={0x161b16,0x9992e7,0x58b9a0,0xff677c,0x3db8e4};
 bool mask=si[3]&1;
 for(auto&s:segs){if(!combined&&s.screen!=screen)continue;if((pass==1&&mask)||(pass==2&&!s.buffer_state))continue;
  if(!p.seg)continue;uint32_t c=mask?0xffffff:colors[s.color_index<5?s.color_index:0];
  SDL_SetTextureColorMod(p.seg,c>>16,(c>>8)&255,c&255);SDL_SetTextureAlphaMod(p.seg,pass==2?255:15);
  SDL_Rect src{s.pos_tex[0],s.pos_tex[1],s.size_tex[0],s.size_tex[1]};
  fit(p,p.seg,src,s.pos_scr[0]+(combined&&s.screen==1?w0:0),s.pos_scr[1],s.size_tex[0],s.size_tex[1],cw,ch);
 }
}
static uint32_t buttonBit(Uint8 button){
 switch(button){
 case SDL_CONTROLLER_BUTTON_DPAD_UP:return CTL_DPAD_UP;
 case SDL_CONTROLLER_BUTTON_DPAD_DOWN:return CTL_DPAD_DOWN;
 case SDL_CONTROLLER_BUTTON_DPAD_LEFT:return CTL_DPAD_LEFT;
 case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:return CTL_DPAD_RIGHT;
 case SDL_CONTROLLER_BUTTON_A:return CTL_A;case SDL_CONTROLLER_BUTTON_B:return CTL_B;
 case SDL_CONTROLLER_BUTTON_X:return CTL_X;case SDL_CONTROLLER_BUTTON_Y:return CTL_Y;
 case SDL_CONTROLLER_BUTTON_START:return CTL_START;case SDL_CONTROLLER_BUTTON_BACK:return CTL_SELECT;
 case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:return CTL_L1;default:return 0;}
}
static uint32_t mask(SDL_GameController*c,const ButtonProfile&profile){
 auto k=SDL_GetKeyboardState(nullptr);uint32_t m=0;
 auto b=[&](SDL_GameControllerButton x){
  if(!c)return false;if(profile.ready){auto binding=SDL_GameControllerGetBindForButton(c,x);if(binding.bindType==SDL_CONTROLLER_BINDTYPE_BUTTON&&profile.reserved(binding.value.button))return false;}
  return SDL_GameControllerGetButton(c,x)!=0;
 };
 if(k[SDL_SCANCODE_UP]||b(SDL_CONTROLLER_BUTTON_DPAD_UP))m|=CTL_DPAD_UP;
 if(k[SDL_SCANCODE_DOWN]||b(SDL_CONTROLLER_BUTTON_DPAD_DOWN))m|=CTL_DPAD_DOWN;
 if(k[SDL_SCANCODE_LEFT]||b(SDL_CONTROLLER_BUTTON_DPAD_LEFT))m|=CTL_DPAD_LEFT;
 if(k[SDL_SCANCODE_RIGHT]||b(SDL_CONTROLLER_BUTTON_DPAD_RIGHT))m|=CTL_DPAD_RIGHT;
 if(k[SDL_SCANCODE_Z]||(!profile.ready&&b(SDL_CONTROLLER_BUTTON_A)))m|=CTL_A;
 if(k[SDL_SCANCODE_X]||(!profile.ready&&b(SDL_CONTROLLER_BUTTON_B)))m|=CTL_B;
 if(k[SDL_SCANCODE_A]||(!profile.ready&&b(SDL_CONTROLLER_BUTTON_X)))m|=CTL_X;
 if(k[SDL_SCANCODE_S]||(!profile.ready&&b(SDL_CONTROLLER_BUTTON_Y)))m|=CTL_Y;
 if(profile.ready)m|=profile.held(c);
 else {if(b(SDL_CONTROLLER_BUTTON_START))m|=CTL_START;if(b(SDL_CONTROLLER_BUTTON_BACK))m|=CTL_SELECT;if(b(SDL_CONTROLLER_BUTTON_LEFTSHOULDER))m|=CTL_L1;}
 if(k[SDL_SCANCODE_RETURN])m|=CTL_START;
 if(k[SDL_SCANCODE_BACKSPACE])m|=CTL_SELECT;
 if(k[SDL_SCANCODE_Q])m|=CTL_L1;
 return m;
}
static bool calibrateButtons(ButtonProfile&profile,SDL_GameController*ctl,RgdsInput&raw,RgdsVolume&volume,bool full=false){
 auto*j=SDL_GameControllerGetJoystick(ctl);int stage=full?0:(profile.ready?8:profile.loadLegacy(ctl));profile.ready=false;bool armed=false;uint64_t releasedSince=0,skipSince=0;
 const char*labels[]={"START","SELECT","L1","R1","A","B","X","Y","R (ANBERNIC)"};
 while(!stopped&&stage<9){
  uint64_t now=SDL_GetTicks64();raw.poll(now);raw.takePresses();auto keys=raw.takeKeys();volume.poll(now);volume.change(raw.volumeDelta(now),now);
  SDL_Event e;
  while(SDL_PollEvent(&e)){
   if(e.type==SDL_QUIT||(e.type==SDL_KEYDOWN&&e.key.keysym.sym==SDLK_ESCAPE&&stage!=8))return false;
   if(armed&&e.type==SDL_JOYBUTTONDOWN&&e.jbutton.which==SDL_JoystickInstanceID(j)){
    int key=e.jbutton.button;bool duplicate=false;for(int n=0;n<stage;n++)if(profile.buttons[n]==key)duplicate=true;
    if(stage==8&&!duplicate){profile.special=key;profile.specialType='B';stage++;}
    else if(!duplicate){profile.buttons[stage]=key;printf("Calibration physical %s -> raw joystick button %d\n",labels[stage],key);stage++;}
    armed=false;releasedSince=0;break;
   }
   if(armed&&stage==8&&e.type==SDL_KEYDOWN&&!e.key.repeat){profile.special=e.key.keysym.scancode;profile.specialType='K';stage++;armed=false;break;}
  }
  if(armed&&stage==8)for(auto key:keys)if(key!=KEY_VOLUMEUP&&key!=KEY_VOLUMEDOWN){profile.special=key;profile.specialType='E';stage++;armed=false;break;}
  bool held=false;for(int n=0;n<SDL_JoystickNumButtons(j);n++)held|=SDL_JoystickGetButton(j,n)!=0;
  if(!armed){if(held)releasedSince=0;else{if(!releasedSince)releasedSince=now;if(now-releasedSince>=160)armed=true;}}
  if(stage==8&&SDL_JoystickGetButton(j,profile.buttons[0])&&SDL_JoystickGetButton(j,profile.buttons[1])){if(!skipSince)skipSince=now;if(now-skipSince>800){profile.specialType='N';profile.special=0;stage++;}}else skipSince=0;
  if(stage>=9)break;
  for(int i=0;i<np;i++){auto&p=panels[i];clear(p);menuHeader(p,40,45,944,tr("Definir botões","Configure buttons"));
   text(p,40,160,std::string(tr("Passo ","Step "))+std::to_string(stage+1)+" / 9");
   text(p,40,245,armed?tr("Premir o botão físico:","Press the physical button:"):tr("Soltar todos os botões...","Release all buttons..."));
   selectionRow(p,28,315,968,90,true);label(p,49,337,labels[stage],2,menu::fg);
   ui::rect(p.r,40,510,944,1,menu::line);text(p,40,540,tr("A configuração fica guardada na consola.","Settings are saved on the console."));
   if(stage==8)text(p,40,655,tr("Se R não responder: manter START+SELECT para ignorar.","If R does not respond: hold START+SELECT to skip."));
   text(p,40,600,tr("Usar os botões indicados no corpo da RGDS+.","Follow the button labels on the RGDS+."));SDL_RenderPresent(p.r);}
  SDL_Delay(8);
 }
 if(stopped)return false;
 if(!profile.save(ctl)||!profile.saveSpecial(ctl)){fprintf(stderr,"Nao foi possivel guardar buttons-rgds-v2.cfg\n");return false;}
 return profile.load(ctl);
}
int main(int argc,char**argv){
 const Uint64 startupStart=SDL_GetTicks64();
 setvbuf(stdout,nullptr,_IONBF,0);signal(SIGTERM,stop);signal(SIGINT,stop);
 if(argc>1&&!strcmp(argv[1],"--version")){puts("Yokoi RGDS+ Linux 1.1 Final / upstream f273025");return 0;}
 bool smoke=argc>1&&(!strcmp(argv[1],"--smoke-test")||!strcmp(argv[1],"--test-game"));
 bool testGame=argc>1&&!strcmp(argv[1],"--test-game");
 const char*pack=smoke&&argc>2?argv[2]:(argc>1&&!smoke?argv[1]:"yokoi_pack_rgds.ykp");
 SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");
 if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_GAMECONTROLLER|SDL_INIT_EVENTS)){fprintf(stderr,"SDL: %s\n",SDL_GetError());return 1;}
 SDL_version v;SDL_GetVersion(&v);printf("SDL %d.%d.%d video=%s displays=%d\n",v.major,v.minor,v.patch,SDL_GetCurrentVideoDriver(),SDL_GetNumVideoDisplays());
 SDL_ShowCursor(SDL_DISABLE);int displays=SDL_GetNumVideoDisplays();
 int panelCount=smoke&&getenv("YOKOI_TEST_DUAL")?2:std::min(2,displays);
 for(int i=0;i<panelCount;i++){
  Panel&p=panels[i];SDL_Rect r;SDL_GetDisplayBounds(i%displays,&r);printf("Output %d: %d,%d %dx%d\n",i,r.x,r.y,r.w,r.h);
  p.w=SDL_CreateWindow("Yokoi RGDS+",SDL_WINDOWPOS_CENTERED_DISPLAY(i%displays),SDL_WINDOWPOS_CENTERED_DISPLAY(i%displays),1024,768,SDL_WINDOW_SHOWN);
  if(!p.w)break;
  if(!smoke)SDL_SetWindowFullscreen(p.w,SDL_WINDOW_FULLSCREEN_DESKTOP);
  p.r=SDL_CreateRenderer(p.w,-1,SDL_RENDERER_ACCELERATED);if(!p.r)p.r=SDL_CreateRenderer(p.w,-1,SDL_RENDERER_SOFTWARE);
  if(!p.r){SDL_DestroyWindow(p.w);p.w=nullptr;break;}np++;clear(p);
 }
 if(!np){fprintf(stderr,"No renderer: %s\n",SDL_GetError());SDL_Quit();return 2;}
 SDL_GameController*ctl=nullptr;
 auto openController=[&](){if(ctl&&SDL_GameControllerGetAttached(ctl))return;if(ctl)SDL_GameControllerClose(ctl);ctl=nullptr;for(int i=0;i<SDL_NumJoysticks();i++){char guid[64];SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(i),guid,sizeof guid);printf("Input %d: %s GUID=%s mapped=%d\n",i,SDL_JoystickNameForIndex(i),guid,SDL_IsGameController(i));if(SDL_IsGameController(i)&&!ctl)ctl=SDL_GameControllerOpen(i);}};
 SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");openController();
 if(ctl){char*m=SDL_GameControllerMapping(ctl);printf("SDL mapping (setup uses calibrated raw buttons): %s\n",m?m:"");SDL_free(m);}
 menu::loadLanguage();
 RgdsInput raw;raw.openDevices();RgdsVolume volume;volume.init();PressLatch latch;ButtonProfile profile;RawTouch touch;touch.openDevice();TouchGesture sdlGesture;
 bool mapped=ctl&&profile.load(ctl);bool specialMapped=mapped&&profile.loadSpecial(ctl);
 if(!smoke&&ctl&&(!mapped||!specialMapped)&&!calibrateButtons(profile,ctl,raw,volume)){
  for(int i=0;i<np;i++){if(panels[i].font)SDL_DestroyTexture(panels[i].font);SDL_DestroyRenderer(panels[i].r);SDL_DestroyWindow(panels[i].w);}SDL_GameControllerClose(ctl);SDL_Quit();return 0;
 }
 printf("Emulation=independent wall-clock thread; nominal render=60 Hz; audio=1024 frames + 40ms prefill + DC/lowpass filter\n");
 const Uint64 packStart=SDL_GetTicks64();
 std::string err;bool loaded=gw_pack::load(pack,&err);printf("Pack: %s / %s\n",pack,loaded?"OK":err.c_str());
 printf("Startup: SDL/input=%llums pack=%llums\n",(unsigned long long)(packStart-startupStart),(unsigned long long)(SDL_GetTicks64()-packStart));
 size_t count=gw_pack::game_count(),selected=0;bool playing=false,paused=false;int pauseSelection=0,slot=1,advancedSelection=0;bool managing=false,advanced=false;int confirmation=0;bool confirmYes=false;std::string toast;uint64_t toastUntil=0;std::unique_ptr<SM5XX>cpu;std::unique_ptr<Virtual_Input>input;const GW_rom*g=nullptr;std::vector<Segment>segs;
 SDL_AudioDeviceID audio=0;SDL_AudioStream*stream=nullptr;uint32_t prev=0;EmuWorker worker;
 auto silence=[&](){worker.stop();if(audio){SDL_ClearQueuedAudio(audio);SDL_CloseAudioDevice(audio);audio=0;}if(stream){SDL_FreeAudioStream(stream);stream=nullptr;}};
 auto selectGame=[&](){g=gw_pack::game_at(selected);for(int i=0;i<np;i++){drop(panels[i]);if(g&&i==1)panels[i].console=texture(panels[i],g->path_console);}};
 auto prepareGameGraphics=[&](){for(int i=0;i<np;i++){auto&p=panels[i];if(!p.seg)p.seg=texture(p,g->path_segment,!(g->segment_info[3]&1));if(!p.bg)p.bg=texture(p,g->path_background);if(!p.console)p.console=texture(p,g->path_console);}};
 auto startGame=[&](int mode=0){if(!g)return;silence();prepareGameGraphics();input.reset();cpu.reset();if(!yokoi_cpu_create_instance(cpu,g->rom,g->size_rom)){err="CPU nao suportado";return;}
  if(cpu->get_cpu_type_id()==2&&(!g->melody||g->size_melody<256)){err="ROM de melodia ausente/incompleta";fprintf(stderr,"%s: %s\n",g->name.c_str(),err.c_str());cpu.reset();return;}
  cpu->load_rom(g->rom,g->size_rom);cpu->load_rom_melody(g->melody,g->size_melody);cpu->init();cpu->load_rom_time_addresses(g->ref);yokoi_cpu_set_time_if_needed(cpu.get());input.reset(get_input_config(cpu.get(),g->ref));if(input)cpu->set_input_multiplexage(input->use_multiplexage);
  segs.assign(g->segment,g->segment+g->size_segment);for(auto&s:segs)s.state=s.buffer_state=false;
  SDL_AudioSpec want{},have{};want.freq=48000;want.format=AUDIO_S16SYS;want.channels=1;want.samples=1024;
  audio=SDL_OpenAudioDevice(nullptr,0,&want,&have,0);if(audio){stream=SDL_NewAudioStream(AUDIO_S16SYS,1,cpu->frequency/std::max(1u,cpu->sound_divide_frequency),have.format,have.channels,have.freq);if(stream){std::vector<int16_t>prefill(have.freq*40/1000,0);SDL_QueueAudio(audio,prefill.data(),prefill.size()*2);SDL_PauseAudioDevice(audio,0);}else silence();}else fprintf(stderr,"Audio: %s\n",SDL_GetError());
  printf("Game: %s (%s), mode=%d\n",g->name.c_str(),g->ref.c_str(),mode);worker.gain=volume.gain();worker.start(cpu.get(),input.get(),segs,audio,stream,mode);playing=true;paused=false;
 };
 auto resume=[&](){
  if(!playing)return;if(stream)SDL_AudioStreamClear(stream);
  if(audio){SDL_ClearQueuedAudio(audio);std::vector<int16_t>z(1920,0);SDL_QueueAudio(audio,z.data(),z.size()*2);SDL_PauseAudioDevice(audio,0);}
  worker.gain=volume.gain();worker.start(cpu.get(),input.get(),segs,audio,stream,-1);paused=false;
 };
 auto pause=[&](){if(!playing||paused)return;worker.stop();worker.snapshot(segs);if(audio){SDL_PauseAudioDevice(audio,1);SDL_ClearQueuedAudio(audio);}paused=true;pauseSelection=0;};
 auto notify=[&](const std::string&message){toast=message;toastUntil=SDL_GetTicks64()+2200;printf("Yokoi: %s\n",message.c_str());};
 auto refresh=[&](){for(auto&s:segs)s.state=s.buffer_state=cpu->get_segments_state(s.id[0],s.id[1],s.id[2]);};
 if(smoke&&argc>3)selected=std::min(count?count-1:0,size_t(atoi(argv[3])));
 if(count)selectGame();if(testGame&&count)startGame();
 bool startupReported=false;
 uint64_t frame=0,ticks=0;double next=SDL_GetPerformanceCounter()/(double)SDL_GetPerformanceFrequency();
 while(!stopped){
  uint64_t nowMs=SDL_GetTicks64();raw.poll(nowMs);raw.takePresses();auto rawKeys=raw.takeKeys();
  std::vector<Gesture>gestures=touch.poll(nowMs);SDL_Event e;bool quit=false,toggle=false,swap=false,previousLetter=false,nextLetter=false;uint32_t presses=0;
  for(auto key:rawKeys){if(profile.specialType=='E'&&int(key)==profile.special)toggle=true;printf("RGDS key down: %u\n",key);}
  while(SDL_PollEvent(&e)){
   if(e.type==SDL_QUIT){if(playing){pause();confirmation=1;confirmYes=false;}else{confirmation=1;confirmYes=false;}}
   if(e.type==SDL_CONTROLLERDEVICEADDED||e.type==SDL_CONTROLLERDEVICEREMOVED)openController();
   if(e.type==SDL_KEYDOWN&&!e.key.repeat){if(e.key.keysym.sym==SDLK_ESCAPE||e.key.keysym.sym==SDLK_p)toggle=true;if(e.key.keysym.sym==SDLK_F2)swap=true;if(profile.specialType=='K'&&e.key.keysym.scancode==profile.special)toggle=true;if(e.key.keysym.sym==SDLK_PAGEUP)previousLetter=true;if(e.key.keysym.sym==SDLK_PAGEDOWN)nextLetter=true;}
   if((e.type==SDL_KEYDOWN||e.type==SDL_KEYUP)&&!raw.hasVolume){
    if(e.key.keysym.scancode==SDL_SCANCODE_VOLUMEUP)raw.key(KEY_VOLUMEUP,e.type==SDL_KEYUP?0:(e.key.repeat?2:1),nowMs,false,true);
    if(e.key.keysym.scancode==SDL_SCANCODE_VOLUMEDOWN)raw.key(KEY_VOLUMEDOWN,e.type==SDL_KEYUP?0:(e.key.repeat?2:1),nowMs,false,true);
   }
   if(e.type==SDL_CONTROLLERBUTTONDOWN){
    if((profile.special<0||profile.specialType=='N')&&e.cbutton.button==SDL_CONTROLLER_BUTTON_GUIDE)toggle=true;
    uint32_t b=buttonBit(e.cbutton.button);if(profile.ready)b&=CTL_DPAD_UP|CTL_DPAD_DOWN|CTL_DPAD_LEFT|CTL_DPAD_RIGHT;presses|=b;
   }
   if(e.type==SDL_JOYBUTTONDOWN&&ctl&&e.jbutton.which==SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(ctl))){
    if(profile.ready){presses|=profile.press(e.jbutton.button);if(profile.swapPress(e.jbutton.button))nextLetter=true;if(e.jbutton.button==profile.buttons[2])previousLetter=true;}
    if(profile.specialType=='B'&&e.jbutton.button==profile.special)toggle=true;printf("RGDS joystick down: %u\n",e.jbutton.button);
   }
   if(e.type==SDL_CONTROLLERAXISMOTION&&(e.caxis.axis==SDL_CONTROLLER_AXIS_TRIGGERLEFT||e.caxis.axis==SDL_CONTROLLER_AXIS_TRIGGERRIGHT)){
    static bool triggerHeld[2]{};int n=e.caxis.axis==SDL_CONTROLLER_AXIS_TRIGGERRIGHT;bool held=e.caxis.value>16000;if(held&&!triggerHeld[n])swap=true;triggerHeld[n]=held;
   }
   if(!touch.available()&&(e.type==SDL_FINGERDOWN||e.type==SDL_FINGERUP)){
    if(e.type==SDL_FINGERDOWN)sdlGesture.down(e.tfinger.x,e.tfinger.y,nowMs);else{int panel=1;for(int i=0;i<np;i++)if(SDL_GetWindowID(panels[i].w)==e.tfinger.windowID)panel=i;gestures.push_back(sdlGesture.up(e.tfinger.x,e.tfinger.y,nowMs,panel));}
   }
   if(!touch.available()&&(e.type==SDL_MOUSEBUTTONDOWN||e.type==SDL_MOUSEBUTTONUP)&&e.button.which!=SDL_TOUCH_MOUSEID){
    int panel=0;for(int i=0;i<np;i++)if(SDL_GetWindowID(panels[i].w)==e.button.windowID)panel=i;
    float x=e.button.x/1024.f,y=e.button.y/768.f;if(e.type==SDL_MOUSEBUTTONDOWN)sdlGesture.down(x,y,nowMs);else gestures.push_back(sdlGesture.up(x,y,nowMs,panel));
   }
  }
  if(quit)break;
  volume.poll(nowMs);volume.change(raw.volumeDelta(nowMs),nowMs);
  latch.press(presses,nowMs);uint32_t m=latch.apply(mask(ctl,profile),nowMs),edge=m&~prev;prev=m;
  if((m&(CTL_START|CTL_SELECT))==(CTL_START|CTL_SELECT)&&(edge&(CTL_START|CTL_SELECT)))toggle=true;
  if(toggle){if(confirmation){confirmation=0;}else if(advanced){advanced=false;}else if(managing){managing=false;}else if(playing){if(paused)resume();else pause();}else{confirmation=1;confirmYes=false;}}
  if(swap&&playing&&!paused)swapPanels=!swapPanels;
  bool activate=false;
  for(auto&gesture:gestures){
   if(confirmation&&gesture.tap){if(gesture.y>.55f&&gesture.y<.69f){confirmYes=gesture.x>.5f;activate=true;}continue;}
   if(!playing&&count){
    if(advanced){if(gesture.tap&&gesture.y*768>=240&&gesture.y*768<240+3*85){advancedSelection=int((gesture.y*768-240)/85);activate=true;}}
    else if(gesture.swipe){selected=(selected+count+gesture.swipe)%count;selectGame();}
    else if(gesture.doubled)activate=true;
   }else if(paused&&gesture.tap){
    if(managing){if(gesture.y>.24f&&gesture.y<.65f)slot=std::clamp(int((gesture.y*768-190)/95)+1,1,3);if(gesture.y>.72f)activate=true;}
    else if(gesture.y*768>=112&&gesture.y*768<112+8*56){pauseSelection=int((gesture.y*768-112)/56);activate=true;}
   }
  }
  if(confirmation){
   if(edge&(CTL_DPAD_LEFT|CTL_DPAD_RIGHT))confirmYes=!confirmYes;
   if(edge&CTL_B)confirmation=0;
   else if(activate||(edge&CTL_A)){if(confirmYes){if(confirmation==1)quit=true;else notify(state_store::erase(g,slot)?tr("Estado apagado","State deleted"):tr("Não foi possível apagar","Could not delete state"));}confirmation=0;}
  }else if(!playing&&count&&!toggle){
   if(advanced){
    if(edge&CTL_DPAD_UP)advancedSelection=(advancedSelection+2)%3;
    if(edge&CTL_DPAD_DOWN)advancedSelection=(advancedSelection+1)%3;
    if(edge&CTL_B)advanced=false;
    else if(activate||(edge&(CTL_A|CTL_START))){
     if(advancedSelection==0){menu::english=!menu::english;if(!menu::saveLanguage())notify(tr("Erro ao guardar idioma","Could not save language"));}
     else if(advancedSelection==1&&ctl){ButtonProfile revised=profile;if(calibrateButtons(revised,ctl,raw,volume,true))profile=revised;prev=0;latch=PressLatch();raw.takeKeys();}
     else if(advancedSelection==2)advanced=false;
    }
   }else if(edge&CTL_X){advanced=true;advancedSelection=0;}
   else{
    size_t old=selected;
    if(edge&(CTL_DPAD_DOWN|CTL_DPAD_RIGHT))selected=(selected+1)%count;
    if(edge&(CTL_DPAD_UP|CTL_DPAD_LEFT))selected=(selected+count-1)%count;
    if(previousLetter)selected=menu::jump(selected,-1);if(nextLetter)selected=menu::jump(selected,1);
    if(old!=selected)selectGame();
    if(activate||(edge&(CTL_A|CTL_START)))startGame(0);
    else if(edge&CTL_SELECT)startGame(1);
    else if(edge&CTL_Y){
     if(!state_store::exists(g,slot))for(int n=1;n<=3;n++)if(state_store::exists(g,n)){slot=n;break;}
     startGame(-1);pause();if(state_store::load(cpu.get(),g,slot)){refresh();resume();notify(tr("Estado carregado","State loaded"));}else notify(tr("Sem estado guardado","No saved state"));
    }
   }
  }else if(playing&&paused&&!toggle){
   if(managing){
    if(edge&(CTL_DPAD_UP|CTL_DPAD_LEFT))slot=(slot+1)%3+1;
    if(edge&(CTL_DPAD_DOWN|CTL_DPAD_RIGHT))slot=slot%3+1;
    if(edge&CTL_B)managing=false;
    else if(activate||(edge&CTL_A)){if(state_store::exists(g,slot)){confirmation=2;confirmYes=false;}else notify(tr("Slot vazio","Empty slot"));}
   }else{
    if(edge&CTL_DPAD_UP)pauseSelection=(pauseSelection+7)%8;if(edge&CTL_DPAD_DOWN)pauseSelection=(pauseSelection+1)%8;
    if(edge&CTL_DPAD_LEFT)slot=(slot+1)%3+1;if(edge&CTL_DPAD_RIGHT)slot=slot%3+1;
    if(edge&CTL_B)resume();
    else if(activate||(edge&CTL_A))switch(pauseSelection){
     case 0:resume();break;
     case 1:silence();playing=false;paused=false;advancedSelection=0;break;
     case 2:notify(state_store::save(cpu.get(),g,slot)?std::string(tr("Estado salvo no slot ","State saved in slot "))+std::to_string(slot):tr("Não foi possível salvar","Could not save state"));break;
     case 3:if(state_store::load(cpu.get(),g,slot)){refresh();resume();notify(tr("Estado carregado","State loaded"));}else notify(tr("Estado vazio, inválido ou de outro jogo","Empty, invalid or mismatched state"));break;
     case 4:managing=true;break;
     case 5:startGame(0);break;case 6:startGame(1);break;
     case 7:confirmation=1;confirmYes=false;break;
    }
   }
  }
  if(quit)break;
  if(playing&&!paused){worker.controls=m&~(CTL_START|CTL_SELECT);worker.gain=volume.gain();}
  if(smoke&&playing&&!paused&&getenv("YOKOI_TEST_PAUSE_AT_FRAME")&&frame==uint64_t(atoi(getenv("YOKOI_TEST_PAUSE_AT_FRAME"))))pause();
  if(ticks%4==0){
   if(playing&&!paused)worker.snapshot(segs);
   for(int i=0;i<np;i++){
    auto&p=panels[i];int role=swapPanels?1-i:i;clear(p);
    if(!loaded){label(p,40,50,"Yokoi: Game & Watch Emulator",2);label(p,40,145,tr("Falta o pack yokoi_pack_rgds.ykp","Missing yokoi_pack_rgds.ykp"));label(p,40,215,err.substr(0,65),0);}
    else if(!playing){
     if(i==0||np==1){
      label(p,30,22,"Yokoi: Game & Watch Emulator",2,menu::fg,720);
      std::string edition="RGDS+ Edition";label(p,994-ui::width(edition,0),31,edition,0,menu::orange);
      ui::rect(p.r,30,82,964,1,0x363b45);
      label(p,815,98,std::to_string(selected+1)+" / "+std::to_string(count),0,menu::muted);
      size_t first=selected>3?selected-3:0;
      for(size_t row=0;row<7&&first+row<count;row++){size_t index=first+row;int y=137+row*63;bool active=index==selected;
       selectionRow(p,28,y,968,59,active);
       label(p,49,y+10,gw_pack::game_at(index)->name,active?2:1,active?menu::fg:menu::muted,918);
      }
      ui::rect(p.r,30,594,964,1,0x363b45);
      label(p,35,607,std::string(tr("Data de lançamento: ","Release date: "))+g->date,0,menu::muted);
      int x=35;for(int n=1;n<=3;n++){std::string title="Slot "+std::to_string(n)+": ";label(p,x,641,title,0,menu::muted);x+=ui::width(title,0);bool occupied=state_store::exists(g,n);std::string status=occupied?tr("Sim","Yes"):tr("Não","No");label(p,x,641,status,0,occupied?menu::highlight:menu::muted);x+=ui::width(status,0);if(n<3){label(p,x,641,"   |   ",0,menu::muted);x+=ui::width("   |   ",0);}}
      label(p,35,690,tr("A / START: GAME A   SELECT: GAME B   Y: Carregar estado","A / START: GAME A   SELECT: GAME B   Y: Load state"),0,menu::muted);
      label(p,35,727,tr("L1/R1: Salto   X: Opções avançadas   R: Sair","L1/R1: Jump   X: Advanced options   R: Exit"),0,menu::muted);
     }else if(g&&p.console&&g->console_info){
      auto ci=g->console_info;SDL_Rect src{ci[2],ci[3],ci[4],ci[5]};int tw,th;SDL_QueryTexture(p.console,nullptr,nullptr,&tw,&th);src.y=th-src.y-src.h;SDL_Rect dest{0,0,1024,768};SDL_RenderCopyEx(p.r,p.console,&src,&dest,0,nullptr,SDL_FLIP_HORIZONTAL);
     }
    }else{
     bool two=false;for(auto&s:segs)if(s.screen==1)two=true;bool horizontal=two&&(g->segment_info[3]&2);
     if(two&&!horizontal)drawGame(p,g,segs,np==1?0:role,false);
     else if(role==0||np==1)drawGame(p,g,segs,0,horizontal);
     else if(p.console&&g->console_info){auto ci=g->console_info;SDL_Rect src{ci[2],ci[3],ci[4],ci[5]};fit(p,p.console,src,0,0,ci[4],ci[5],ci[4],ci[5]);}
     if(paused){
      ui::rect(p.r,0,0,1024,768,menu::bg,230);ui::rect(p.r,80,25,864,managing?713:665,menu::bg);
      menuHeader(p,120,36,784,managing?tr("Gerir slots","Manage slots"):tr("Pausa","Pause"),managing?2:3);
      if(managing){
       for(int n=1;n<=3;n++){int y=190+(n-1)*95;bool active=n==slot;bool occupied=state_store::exists(g,n);selectionRow(p,104,y,816,80,active);label(p,125,y+17,"Slot "+std::to_string(n)+"   "+(occupied?tr("Ocupado","Occupied"):tr("Vazio","Empty")),active?2:1,active?menu::fg:menu::muted);}
       ui::rect(p.r,120,530,784,1,menu::line);label(p,125,560,tr("A: Apagar slot selecionado","A: Delete selected slot"),1,menu::orange);
       label(p,125,655,tr("Direcional: Escolher     B: Voltar","D-pad: Select     B: Back"),0,menu::muted);
      }else{
       for(int j=0;j<8;j++){int y=112+j*56;selectionRow(p,104,y,816,52,j==pauseSelection);label(p,125,y+7,menu::pauseItem(j),j==pauseSelection?2:1,j==pauseSelection?menu::fg:menu::muted);}
       ui::rect(p.r,120,574,784,1,menu::line);label(p,120,589,"Slot "+std::to_string(slot)+" / 3   "+(state_store::exists(g,slot)?state_store::stamp(g,slot):tr("Vazio","Empty")),0,menu::muted);
       label(p,120,638,tr("Esq./Dir.: Slot     A: Confirmar     B: Voltar","Left/Right: Slot     A: Confirm     B: Back"),0,menu::muted);
      }
     }
    }
    if(advanced&&!playing&&i==0){
     clear(p);menuHeader(p,40,40,944,tr("Opções avançadas","Advanced options"));
     std::string items[]={std::string(tr("Idioma: ","Language: "))+(menu::english?"EN":"PT")+"   PT / EN",tr("Definir botões","Configure buttons"),tr("Voltar","Back")};
     for(int j=0;j<3;j++){int y=240+j*85;selectionRow(p,40,y,944,70,j==advancedSelection);label(p,65,y+17,items[j],j==advancedSelection?2:1,j==advancedSelection?menu::fg:menu::muted);}
     ui::rect(p.r,40,660,944,1,menu::line);label(p,40,685,tr("A: Confirmar     B: Voltar","A: Confirm     B: Back"),0,menu::muted);
    }
    if(confirmation&&(playing||i==0)){
     ui::rect(p.r,0,0,1024,768,0x000000,180);ui::rect(p.r,130,235,764,300,menu::bg);ui::rect(p.r,165,319,694,1,menu::line);
     label(p,170,267,confirmation==1?tr("Sair do Yokoi?","Exit Yokoi?"):tr("Apagar este estado?","Delete this state?"),2,menu::fg,690);
     label(p,170,332,confirmation==1?tr("Os progressos não salvos serão perdidos.","Unsaved progress will be lost."):("Slot "+std::to_string(slot)+tr(" - esta ação não pode ser anulada."," - this cannot be undone.")),0,menu::muted,690);
     selectionRow(p,175,425,300,62,!confirmYes);selectionRow(p,548,425,300,62,confirmYes);
     label(p,205,438,tr("Cancelar","Cancel"),!confirmYes?2:1,!confirmYes?menu::fg:menu::muted);label(p,580,438,confirmation==1?tr("Sair","Exit"):tr("Apagar","Delete"),confirmYes?2:1,confirmYes?menu::fg:menu::muted);
    }
    if(nowMs<volume.visibleUntil&&(playing||i==0)){ui::rect(p.r,300,8,424,70,menu::bg,245);label(p,324,14,"Volume "+std::to_string(volume.level())+" / 10",0);for(int v=0;v<10;v++)ui::rect(p.r,324+v*37,52,31,9,v<volume.level()?menu::highlight:menu::selection);}
    if(nowMs<toastUntil&&(playing||i==0)){ui::rect(p.r,60,8,904,53,menu::bg,245);label(p,80,17,toast,0,menu::fg,860);}
    if(smoke&&((testGame&&frame==240)||(!testGame&&frame==1))){auto*surface=SDL_CreateRGBSurfaceWithFormat(0,1024,768,32,SDL_PIXELFORMAT_ARGB8888);if(surface){SDL_RenderReadPixels(p.r,nullptr,surface->format->format,surface->pixels,surface->pitch);std::string file="/tmp/yokoi-test-"+std::to_string(i)+".bmp";SDL_SaveBMP(surface,file.c_str());SDL_FreeSurface(surface);}}
    if(smoke&&getenv("YOKOI_TEST_RENDER_DELAY_MS"))SDL_Delay(atoi(getenv("YOKOI_TEST_RENDER_DELAY_MS")));SDL_RenderPresent(p.r);if(!startupReported&&i==np-1){printf("Startup: first frame=%llums\n",(unsigned long long)(SDL_GetTicks64()-startupStart));startupReported=true;}
   }
   if(smoke&&++frame>=(testGame?(getenv("YOKOI_TEST_FRAMES")?atoi(getenv("YOKOI_TEST_FRAMES")):300):3))break;
  }
  ++ticks;next+=1./240;double now=SDL_GetPerformanceCounter()/(double)SDL_GetPerformanceFrequency();
  if(next>now){timespec delay{0,long((next-now)*1e9)};while(nanosleep(&delay,&delay)&&errno==EINTR&&!stopped){}}else if(now-next>.05)next=now;
 }
 silence();input.reset();cpu.reset();if(ctl)SDL_GameControllerClose(ctl);for(int i=0;i<np;i++){drop(panels[i]);if(panels[i].font)SDL_DestroyTexture(panels[i].font);SDL_DestroyRenderer(panels[i].r);SDL_DestroyWindow(panels[i].w);}gw_pack::unload();SDL_Quit();return 0;
}
