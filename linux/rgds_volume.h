#pragma once
#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <string>

class RgdsVolume {
 std::string stockPath,savedPath;
 int value=5,lastStock=-1;
 uint64_t nextPoll=0;
 bool read(const std::string&p,int&v){FILE*f=fopen(p.c_str(),"r");if(!f)return false;bool ok=fscanf(f,"%d",&v)==1;fclose(f);return ok&&v>=0&&v<=10;}
 bool write(const std::string&p,int v){FILE*f=fopen(p.c_str(),"w");if(!f)return false;bool ok=fprintf(f,"%d\n",v)>0;int result=fclose(f);return ok&&result==0;}
public:
 uint64_t visibleUntil=0;
 explicit RgdsVolume(std::string stock="/sys/class/anbernic_misc/openbor_volume",std::string saved="volume-level.txt"):stockPath(stock),savedPath(saved){}
 void init(){int n;if(read(stockPath,n)){value=lastStock=n;}else if(read(savedPath,n))value=n;printf("RGDS volume: %d/10 curve=0.35*(level/10)^2\n",value);}
 void change(int delta,uint64_t now){
  if(!delta)return;
  value=std::clamp(value+delta,0,10);visibleUntil=now+1500;
  bool saved=write(stockPath,value);if(saved)lastStock=value;
  write(savedPath,value);nextPoll=now+250;
  printf("RGDS volume: button=%d/10 gain=%.5f firmware_saved=%d\n",value,gain(),saved);
 }
 bool poll(uint64_t now){
  if(now<nextPoll)return false;nextPoll=now+250;int n;
  if(read(stockPath,n)&&n!=lastStock){lastStock=n;value=n;visibleUntil=now+1500;printf("RGDS volume: external=%d/10\n",value);return true;}return false;
 }
 int level()const{return value;}
 static float curve(int level){float x=std::clamp(level,0,10)/10.f;return .35f*x*x;}
 float gain()const{return curve(value);}
};
