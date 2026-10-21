#include <atomic>
#include <cstddef>
static std::atomic<size_t> observedSelection{0};
static std::atomic<int> observedManufacturer{-1};
static std::atomic<bool> observedPlaying{false};
#define YOKOI_TEST_LIBRARY_SELECTION(s,m,p) do{observedSelection=s;observedManufacturer=m;observedPlaying=p;}while(0)
#define main yokoi_program
#include "../main.cpp"
#undef main
#include <cassert>
#include <filesystem>
int main(int argc,char**argv){
 assert(argc==2);std::string pack=std::filesystem::absolute(argv[1]);std::string error;assert(gw_pack::load(pack,&error));menu::Library lib;lib.filter(1);assert(lib.games.size()==12);size_t middle=lib.games[8],next=lib.games[9];gw_pack::unload();
 char dir[]="/tmp/yokoi-return-XXXXXX";assert(mkdtemp(dir));assert(chdir(dir)==0);
 assert(SDL_Init(SDL_INIT_GAMECONTROLLER)==0);int index=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,0,16,1);assert(index>=0);char guid[64];SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(index),guid,sizeof guid);
 std::string map=std::string(guid)+",return-test,a:b0,b:b1,x:b2,y:b3,start:b7,back:b6,guide:b12,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,";assert(SDL_GameControllerAddMapping(map.c_str())>=0);
 auto*joy=SDL_JoystickOpen(index);auto*ctl=SDL_GameControllerOpen(index);assert(joy&&ctl);ButtonProfile profile;int buttons[]={7,6,4,5,0,1,2,3};for(int i=0;i<8;i++)profile.buttons[i]=buttons[i];assert(profile.save(ctl));profile.special=12;profile.specialType='B';assert(profile.saveSpecial(ctl));SwapProfile swaps;swaps.ready=true;swaps.bindings[0]={'B',8};swaps.bindings[1]={'B',9};assert(swaps.save(ctl));SDL_GameControllerClose(ctl);
 std::thread feeder([=]{
  auto button=[&](int n){SDL_JoystickSetVirtualButton(joy,n,1);SDL_Delay(60);SDL_JoystickSetVirtualButton(joy,n,0);SDL_Delay(100);};
  auto hat=[&](Uint8 h){SDL_JoystickSetVirtualHat(joy,0,h);SDL_Delay(60);SDL_JoystickSetVirtualHat(joy,0,SDL_HAT_CENTERED);SDL_Delay(100);};
  SDL_Delay(350);button(9);assert(observedManufacturer==1);for(int n=0;n<8;n++)hat(SDL_HAT_DOWN);assert(observedSelection==middle);
  button(0);assert(observedPlaying);button(12);
  SDL_JoystickSetVirtualHat(joy,0,SDL_HAT_DOWN);SDL_Delay(100);button(0);SDL_Delay(250);
  assert(!observedPlaying&&observedManufacturer==1&&observedSelection==middle);
  SDL_JoystickSetVirtualHat(joy,0,SDL_HAT_CENTERED);SDL_Delay(100);hat(SDL_HAT_DOWN);assert(observedSelection==next);
  // Re-enter and return once more, using the touchscreen Pause-menu action.
  button(0);assert(observedPlaying);button(12);
  SDL_Event e{};e.type=SDL_FINGERDOWN;e.tfinger.x=.5f;e.tfinger.y=190.f/768;e.tfinger.touchId=1;e.tfinger.fingerId=1;SDL_PushEvent(&e);SDL_Delay(60);e.type=SDL_FINGERUP;SDL_PushEvent(&e);SDL_Delay(250);
  assert(!observedPlaying&&observedManufacturer==1&&observedSelection==next);
  button(12);hat(SDL_HAT_RIGHT);button(0);
 });
 char name[]="test";char*args[]={name,pack.data(),nullptr};int result=yokoi_program(2,args);feeder.join();assert(result==0);assert(chdir("/")==0);std::filesystem::remove_all(dir);
 puts("PASS: second manufacturer, ninth/tenth game retained after physical/touch Choose another game; held D-pad cannot move or wrap selection; next navigation continues from retained game");
}
