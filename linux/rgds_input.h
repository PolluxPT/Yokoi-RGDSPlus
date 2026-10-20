#pragma once
#include <SDL.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <glob.h>
#include <array>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "yokoi_input_mapping.h"

// Kernel key identities remain stable when SDL's generic joystick mapping differs.
class RgdsInput {
 struct Device {int fd;bool setup,volume,dropped=false;std::string path;};
 std::vector<Device> devices;
 bool start=false,select=false;
 uint32_t pressed=0;
 bool volumeUp=false,volumeDown=false;
 uint64_t nextRepeat=0;
 int pendingVolume=0;
 std::vector<unsigned> keyPresses;
 template<size_t N> static bool bit(const std::array<unsigned long,N>& bits,unsigned n){
  return n/(8*sizeof(unsigned long))<N && (bits[n/(8*sizeof(unsigned long))]&(1ul<<(n%(8*sizeof(unsigned long)))));
 }
 void resync(Device&d){
  std::array<unsigned long,(KEY_MAX+8*sizeof(unsigned long))/(8*sizeof(unsigned long))> keys{};
  if(ioctl(d.fd,EVIOCGKEY(sizeof keys),keys.data())<0)return;
  if(d.setup){start=bit(keys,BTN_START);select=bit(keys,BTN_SELECT);}
  if(d.volume){volumeUp=bit(keys,KEY_VOLUMEUP);volumeDown=bit(keys,KEY_VOLUMEDOWN);}
 }
public:
 bool hasSetup=false,hasVolume=false;
 ~RgdsInput(){for(auto&d:devices)close(d.fd);}
 void openDevices(){
  glob_t paths{};
  if(glob("/dev/input/event*",0,nullptr,&paths)!=0){globfree(&paths);return;}
  for(size_t i=0;i<paths.gl_pathc;i++){
   int fd=::open(paths.gl_pathv[i],O_RDONLY|O_NONBLOCK|O_CLOEXEC);if(fd<0)continue;
   char name[256]{};ioctl(fd,EVIOCGNAME(sizeof name),name);
   std::array<unsigned long,(KEY_MAX+8*sizeof(unsigned long))/(8*sizeof(unsigned long))> keys{};
   if(ioctl(fd,EVIOCGBIT(EV_KEY,sizeof keys),keys.data())<0){close(fd);continue;}
   bool setup=!hasSetup&&strstr(name,"ANBERNIC")&&bit(keys,BTN_START)&&bit(keys,BTN_SELECT);
   bool volume=bit(keys,KEY_VOLUMEUP)||bit(keys,KEY_VOLUMEDOWN);
   if(!setup&&!volume&&!strstr(name,"ANBERNIC")&&!strstr(name,"gpio")&&!strstr(name,"rk8")&&!strstr(name,"rk3")&&!strstr(name,"adc-keys")){close(fd);continue;}
   devices.push_back({fd,setup,volume,false,paths.gl_pathv[i]});resync(devices.back());
   hasSetup|=setup;hasVolume|=volume;
   printf("RGDS evdev: %s (%s) setup=%d volume=%d\n",paths.gl_pathv[i],name,setup,volume);
  }
  globfree(&paths);
  if(hasSetup)printf("RGDS raw key node found; game setup uses calibrated joystick buttons\n");
 }
 // Separated event handling permits regression tests with recorded kernel events.
 void key(unsigned code,int value,uint64_t now,bool setup=true,bool volume=true){
  if(value==1)keyPresses.push_back(code);
  if(setup&&(code==BTN_START||code==BTN_SELECT)){
   bool&held=code==BTN_START?start:select;
   if(value==1&&!held)pressed|=code==BTN_START?CTL_START:CTL_SELECT;
   held=value!=0;
  }
  if(volume&&(code==KEY_VOLUMEUP||code==KEY_VOLUMEDOWN)){
   bool&held=code==KEY_VOLUMEUP?volumeUp:volumeDown;
   if(value==1&&!held){pendingVolume+=code==KEY_VOLUMEUP?1:-1;nextRepeat=now+450;}
   held=value!=0; // ignore kernel repeat counts; use one controlled repeat timer
  }
 }
 void poll(uint64_t now){
  for(auto&d:devices){
   input_event events[32];ssize_t n;
   while((n=read(d.fd,events,sizeof events))>0){
    for(size_t i=0;i<size_t(n)/sizeof(input_event);i++){
     auto&e=events[i];
     if(e.type==EV_SYN&&e.code==SYN_DROPPED){d.dropped=true;continue;}
     if(d.dropped){if(e.type==EV_SYN&&e.code==SYN_REPORT){resync(d);d.dropped=false;}continue;}
     if(e.type==EV_KEY)key(e.code,e.value,now,d.setup,d.volume);
    }
   }
  }
 }
 std::vector<unsigned> takeKeys(){auto result=keyPresses;keyPresses.clear();return result;}
 uint32_t setupMask() const {return (start?CTL_START:0)|(select?CTL_SELECT:0);}
 uint32_t takePresses(){uint32_t p=pressed;pressed=0;return p;}
 int volumeDelta(uint64_t now){
  if(volumeUp!=volumeDown&&now>=nextRepeat){pendingVolume+=volumeUp?1:-1;nextRepeat=now+120;}
  int n=pendingVolume;pendingVolume=0;return n;
 }
};

class PressLatch {
 std::array<uint64_t,11> until{};
public:
 void press(uint32_t bits,uint64_t now){for(unsigned i=0;i<until.size();i++)if(bits&(1u<<i))until[i]=now+16;}
 uint32_t apply(uint32_t held,uint64_t now)const{for(unsigned i=0;i<until.size();i++)if(now<until[i])held|=1u<<i;return held;}
};
