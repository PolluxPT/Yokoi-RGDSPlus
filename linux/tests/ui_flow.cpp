#define YOKOI_TEST_SWAPS 1
#define main yokoi_program
#include "../main.cpp"
#undef main
#include <cassert>
#include <filesystem>
#include <fstream>
int main(int argc,char**argv){assert(argc>1);std::string pack=std::filesystem::absolute(argv[1]);char dir[]="/tmp/yokoi-ui-XXXXXX";assert(mkdtemp(dir));assert(chdir(dir)==0);
 assert(SDL_Init(SDL_INIT_GAMECONTROLLER)==0);int index=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,4,16,1);assert(index>=0);
 char guid[64];SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(index),guid,sizeof guid);
 std::string map=std::string(guid)+",RGDS-test,a:b0,b:b1,x:b3,y:b4,back:b10,start:b11,guide:b13,leftshoulder:b6,rightshoulder:b7,lefttrigger:a0,righttrigger:a3,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,";assert(SDL_GameControllerAddMapping(map.c_str())>=0);
 auto*joy=SDL_JoystickOpen(index);auto*ctl=SDL_GameControllerOpen(index);assert(joy&&ctl);ButtonProfile profile;int indices[]={7,6,4,5,0,1,2,3};for(int i=0;i<8;i++)profile.buttons[i]=indices[i];assert(profile.save(ctl));profile.special=getenv("YOKOI_TEST_SPECIAL_KEY")?SDL_SCANCODE_F13:12;profile.specialType=getenv("YOKOI_TEST_SPECIAL_KEY")?'K':'B';assert(profile.saveSpecial(ctl));SwapProfile swaps;swaps.ready=true;swaps.bindings[0]={'A',0,-32768,1};swaps.bindings[1]={'A',3,-32768,1};assert(swaps.save(ctl));SDL_GameControllerClose(ctl);
 SDL_JoystickSetVirtualAxis(joy,0,-32768);SDL_JoystickSetVirtualAxis(joy,3,-32768);
 std::string packError;assert(gw_pack::load(pack,&packError));std::string savedRef=gw_pack::game_at(0)->ref;gw_pack::unload();
 std::thread feeder([joy,savedRef]{
  auto button=[&](int n){if(n==12&&getenv("YOKOI_TEST_SPECIAL_KEY")){SDL_Event e{};e.type=SDL_KEYDOWN;e.key.keysym.sym=SDLK_F13;e.key.keysym.scancode=SDL_SCANCODE_F13;SDL_PushEvent(&e);SDL_Delay(80);e.type=SDL_KEYUP;SDL_PushEvent(&e);SDL_Delay(330);return;}SDL_JoystickSetVirtualButton(joy,n,1);SDL_Delay(80);SDL_JoystickSetVirtualButton(joy,n,0);SDL_Delay(330);};
  auto hat=[&](Uint8 v){SDL_JoystickSetVirtualHat(joy,0,v);SDL_Delay(80);SDL_JoystickSetVirtualHat(joy,0,SDL_HAT_CENTERED);SDL_Delay(330);};
  auto touch=[&](float x,float y,bool down){SDL_Event e{};e.type=down?SDL_FINGERDOWN:SDL_FINGERUP;e.tfinger.x=x;e.tfinger.y=y;e.tfinger.touchId=1;e.tfinger.fingerId=1;SDL_PushEvent(&e);SDL_Delay(70);};
  SDL_Delay(450);touch(.8,.5,true);touch(.2,.5,false);SDL_Delay(120);touch(.2,.5,true);touch(.8,.5,false);SDL_Delay(120);
  button(5);button(4); // next initial, previous initial: Ball
  button(2);button(0); // Language is the first Advanced option; EN -> PT
  {std::ifstream f("language.cfg");std::string value;f>>value;assert(value=="PT");}
  button(0); // Portuguese -> English
  {std::ifstream f("language.cfg");std::string value;f>>value;assert(value=="EN");}
  hat(SDL_HAT_DOWN);button(0);SDL_Delay(350); // reopen complete calibration from menu
  for(int n:{7,6,4,5,0,1,2,3,12})button(n);
  SDL_Delay(400);
  bool digital=getenv("YOKOI_TEST_DIGITAL");
  for(int n=0;n<2;n++){if(digital)button(8+n);else{SDL_JoystickSetVirtualAxis(joy,n?3:0,32767);SDL_Delay(120);SDL_JoystickSetVirtualAxis(joy,n?3:0,-32768);SDL_Delay(350);}}
  button(1);
  touch(.5,.5,true);touch(.5,.5,false);touch(.5,.5,true);touch(.5,.5,false);SDL_Delay(2700);
  for(int n=0;n<2;n++){
   if(digital)SDL_JoystickSetVirtualButton(joy,8+n,1);else SDL_JoystickSetVirtualAxis(joy,n?3:0,32767);
   SDL_Delay(400);assert(testSwapCount.load()==n+1);
   if(digital)SDL_JoystickSetVirtualButton(joy,8+n,0);else SDL_JoystickSetVirtualAxis(joy,n?3:0,-32768);
   SDL_Delay(200);assert(testSwapCount.load()==n+1);
  }
  button(12);hat(SDL_HAT_DOWN);hat(SDL_HAT_DOWN);hat(SDL_HAT_RIGHT);button(0); // save slot 2
  assert(std::ifstream(("saves/"+savedRef+".slot2.yks")).good());
  hat(SDL_HAT_DOWN);button(0);SDL_Delay(350); // load slot 2
  button(12);for(int i=0;i<4;i++)hat(SDL_HAT_DOWN);button(0); // manage
  button(0);button(0);assert(std::ifstream(("saves/"+savedRef+".slot2.yks")).good()); // default cancel
  button(0);hat(SDL_HAT_RIGHT);button(0);assert(!std::ifstream(("saves/"+savedRef+".slot2.yks")).good());
  button(1);hat(SDL_HAT_UP);hat(SDL_HAT_UP);hat(SDL_HAT_RIGHT);button(0); // save slot3
  assert(std::ifstream(("saves/"+savedRef+".slot3.yks")).good());
  hat(SDL_HAT_LEFT);hat(SDL_HAT_LEFT);hat(SDL_HAT_UP);button(0); // slot1, choose another game
  button(3);SDL_Delay(450); // Y finds occupied slot3, loads it
  button(12);hat(SDL_HAT_UP);button(0);button(0); // Exit: cancel by default
  button(0);hat(SDL_HAT_RIGHT);button(0); // confirm exit
 });
 char name[]="test";char*args[]={name,pack.data(),nullptr};int result=yokoi_program(2,args);feeder.join();assert(result==0);assert(!swapPanels);assert(testSwapCount.load()==2);
 assert(!std::ifstream(("saves/"+savedRef+".slot2.yks")).good());assert(std::ifstream(("saves/"+savedRef+".slot3.yks")).good());
 std::filesystem::remove_all("saves");for(auto p:{"buttons-rgds-v2.cfg","special-rgds.cfg","language.cfg","display.cfg","last-game.cfg","swap-rgds-v1.cfg"})remove(p);assert(chdir("/")==0);rmdir(dir);
 puts("PASS: language persistence, letter jumps, touch, calibrated special button despite wrong Guide mapping, L2/R2, slot2 save/load, deletion cancel/confirm, slot3 discovery and exit confirmation");
}
