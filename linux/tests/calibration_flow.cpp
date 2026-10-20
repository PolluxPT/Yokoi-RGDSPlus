// Exercise the actual four-step UI using physical-style, mis-mapped SDL buttons.
#define main yokoi_main
#include "../main.cpp"
#undef main
#include <cassert>
#include <fstream>
int main(){
 char dir[]="/tmp/yokoi-calibration-XXXXXX";assert(mkdtemp(dir));assert(chdir(dir)==0);
 assert(SDL_Init(SDL_INIT_GAMECONTROLLER)==0);
 int index=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,0,16,0);assert(index>=0);
 char guid[64];SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(index),guid,sizeof guid);
 std::string mapping=std::string(guid)+",Virtual-calibration-test,a:b0,b:b1,x:b2,y:b3,leftshoulder:b4,rightshoulder:b5,back:b6,start:b7,leftstick:b8,rightstick:b9,";
 assert(SDL_GameControllerAddMapping(mapping.c_str())>=0);auto*joy=SDL_JoystickOpen(index);assert(joy);
 {std::ofstream f("buttons-rgds-v1.cfg");f<<guid<<" 7 6 4 5\n";}
 std::thread feeder([joy]{
  SDL_Delay(350);
  for(int b:{0,1,2,3,12}){SDL_JoystickSetVirtualButton(joy,b,1);SDL_Delay(90);SDL_JoystickSetVirtualButton(joy,b,0);SDL_Delay(260);}
  for(int n=0;n<100&&!std::ifstream("buttons-rgds-v2.cfg").good();n++)SDL_Delay(50);
  SDL_Event e{};e.type=SDL_QUIT;SDL_PushEvent(&e);SDL_Delay(160);SDL_Event key{};key.type=SDL_KEYDOWN;key.key.keysym.sym=SDLK_ESCAPE;SDL_PushEvent(&key);SDL_Delay(100);stopped=1;
 });
 char name[]="test",pack[]="/no-pack-needed-for-calibration";char*args[]={name,pack,nullptr};int result=yokoi_main(2,args);feeder.join();assert(result==0);
 std::ifstream f("buttons-rgds-v2.cfg");std::string g;int a,b,c,d;assert(f>>g>>a>>b>>c>>d);assert(a==7&&b==6&&c==4&&d==5);int e,x,y,z;assert(f>>e>>x>>y>>z);assert(e==0&&x==1&&y==2&&z==3);
 assert(std::ifstream("special-rgds.cfg").good());remove("special-rgds.cfg");remove("buttons-rgds-v1.cfg");remove("buttons-rgds-v2.cfg");assert(chdir("/")==0);rmdir(dir);
 puts("PASS: actual UI migrated four setup buttons and calibrated A/B/X/Y without the generic Y/L1 collision");
}
