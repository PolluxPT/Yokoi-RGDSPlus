#include <atomic>
#include <cstddef>
#include <unistd.h>
#include <sys/wait.h>
static std::atomic<size_t> selection{0};static std::atomic<int> maker{-1};static std::atomic<bool> inGame{false};
#define YOKOI_TEST_LIBRARY_SELECTION(s,m,p) do{selection=s;maker=m;inGame=p;}while(0)
#define main yokoi_program
#include "../main.cpp"
#undef main
#include <cassert>
#include <filesystem>
#include <fstream>
int main(int argc,char**argv){
 assert(argc>=2);std::string pack=std::filesystem::absolute(argv[1]),executable=std::filesystem::absolute(argv[0]);
 if(argc==3){char name[]="test",mode[]="--smoke-test";char*args[]={name,mode,pack.data(),nullptr};assert(yokoi_program(3,args)==0);assert(selection==20&&maker==1);puts("PASS: fresh process restored last game and manufacturer");return 0;}
 char dir[]="/tmp/yokoi14-ui-XXXXXX";assert(mkdtemp(dir));assert(chdir(dir)==0);
 assert(SDL_Init(SDL_INIT_GAMECONTROLLER)==0);int index=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,0,16,1);assert(index>=0);char guid[64];SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(index),guid,sizeof guid);
 std::string map=std::string(guid)+",test14,a:b0,b:b1,x:b2,y:b3,start:b7,back:b6,guide:b12,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,";assert(SDL_GameControllerAddMapping(map.c_str())>=0);auto*joy=SDL_JoystickOpen(index);auto*ctl=SDL_GameControllerOpen(index);assert(joy&&ctl);ButtonProfile profile;int b[]={7,6,4,5,0,1,2,3};for(int i=0;i<8;i++)profile.buttons[i]=b[i];assert(profile.save(ctl));profile.special=12;profile.specialType='B';assert(profile.saveSpecial(ctl));SwapProfile swaps;swaps.ready=true;swaps.bindings[0]={'B',8};swaps.bindings[1]={'B',9};assert(swaps.save(ctl));SDL_GameControllerClose(ctl);
 std::thread feeder([=]{
  auto button=[&](int n){SDL_JoystickSetVirtualButton(joy,n,1);SDL_Delay(70);SDL_JoystickSetVirtualButton(joy,n,0);SDL_Delay(130);};auto hat=[&](Uint8 h){SDL_JoystickSetVirtualHat(joy,0,h);SDL_Delay(70);SDL_JoystickSetVirtualHat(joy,0,SDL_HAT_CENTERED);SDL_Delay(130);};
  SDL_Delay(350);
  // Advanced options now begins with Language (no display options).
  button(2);button(0);{std::ifstream f("language.cfg");std::string v;f>>v;assert(v=="PT");}button(0);button(1);
  button(9);for(int n=0;n<8;n++)hat(SDL_HAT_DOWN);assert(maker==1&&selection==20);button(0);assert(inGame);SDL_Delay(2600);
  button(12);for(int n=0;n<7;n++)hat(SDL_HAT_DOWN);button(0); // Segments -> Dark
  hat(SDL_HAT_DOWN);hat(SDL_HAT_RIGHT); // Background -> Light blue
  {std::ifstream f("display.cfg");int a,c;f>>a>>c;assert(a==2&&c==1);}
  // Touch selects and changes the same live setting without starting/reloading the game.
  SDL_Event tap{};tap.type=SDL_FINGERDOWN;tap.tfinger.x=.5f;tap.tfinger.y=440.f/768;tap.tfinger.touchId=1;tap.tfinger.fingerId=1;SDL_PushEvent(&tap);SDL_Delay(70);tap.type=SDL_FINGERUP;SDL_PushEvent(&tap);SDL_Delay(180);
  {std::ifstream f("display.cfg");int a,c;f>>a>>c;assert(a==0&&c==1);}
  // CPU remains paused, same game retained; resume rather than recreate.
  button(1);assert(inGame&&selection==20);button(12);for(int n=0;n<5;n++)hat(SDL_HAT_DOWN);button(0); // explicit GAME A
  SDL_Delay(450);button(12);for(int n=0;n<6;n++)hat(SDL_HAT_DOWN);button(0); // explicit GAME B
  SDL_Delay(450);button(12);hat(SDL_HAT_DOWN);button(0);assert(!inGame&&selection==20&&maker==1);
  // Browsing another entry alone must not replace the last opened game.
  hat(SDL_HAT_DOWN);assert(selection==21);button(12);hat(SDL_HAT_RIGHT);button(0);
 });
 char name[]="test";char*args[]={name,pack.data(),nullptr};assert(yokoi_program(2,args)==0);feeder.join();
 // Start an entirely new executable process with the saved configuration.
 pid_t child=fork();assert(child>=0);if(!child){execl(executable.c_str(),executable.c_str(),pack.c_str(),"--reopen",nullptr);_exit(127);}int status;assert(waitpid(child,&status,0)==child);assert(WIFEXITED(status)&&WEXITSTATUS(status)==0);
 assert(chdir("/")==0);std::filesystem::remove_all(dir);puts("PASS: watch entry; explicit GAME A/B; live paused display settings; advanced Language first; return selection; restart persistence; browsing does not change last game");
}
