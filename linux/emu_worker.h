#pragma once
#include <SDL.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <cmath>
#include <algorithm>
#include "SM5XX/SM5XX.h"
#include "std/segment.h"
#include "virtual_i_o/virtual_input.h"
#include "yokoi_input_mapping.h"

// Native-rate conditioning: remove DC and suppress square-wave harmonics near Nyquist.
class BuzzerFilter {
 double b0,b1,b2,a1,a2,x1=0,x2=0,y1=0,y2=0,dc=0,previous=0;
 float gain=0;
public:
 explicit BuzzerFilter(double rate){
  double w=2*3.141592653589793*std::min(6500.,rate*.4)/rate,c=cos(w),alpha=sin(w)/sqrt(2.),a0=1+alpha;
  b0=(1-c)*.5/a0;b1=(1-c)/a0;b2=b0;a1=-2*c/a0;a2=(1-alpha)/a0;
 }
 int16_t sample(bool on,float target){
  double x=on?1.:0.;dc=x-previous+0.996*dc;previous=x;
  double y=b0*dc+b1*x1+b2*x2-a1*y1-a2*y2;x2=x1;x1=dc;y2=y1;y1=y;
  gain+=.004f*(target-gain); // ~8ms gain transition at 32768Hz
  return int16_t(std::clamp(y*26213.*gain,-32767.,32767.));
 }
};

class EmuWorker {
 std::thread thread;
 std::atomic<bool> stopRequested{false};
 std::atomic<uint64_t> cycles{0};
 std::mutex mutex;
 std::vector<Segment> latest;
public:
 std::atomic<uint32_t> controls{0};std::atomic<float> gain{0};
 ~EmuWorker(){stop();}
 void stop(){stopRequested=true;if(thread.joinable())thread.join();}
 uint64_t cycleCount()const{return cycles.load();}
 void snapshot(std::vector<Segment>&out){std::lock_guard<std::mutex>lock(mutex);out=latest;}
 void start(SM5XX*cpu,Virtual_Input*input,const std::vector<Segment>&metadata,SDL_AudioDeviceID audio,SDL_AudioStream*stream,int bootMode=-1){
  stop();latest=metadata;cycles=0;stopRequested=false;controls=0;
  thread=std::thread([=,this]{
   using clock=std::chrono::steady_clock;auto origin=clock::now();uint64_t done=0;
   auto lastReport=origin,lastSnapshot=origin,lastAudio=origin;
   std::vector<Segment>segs=metadata;std::vector<int16_t>pcm;pcm.reserve(1024);std::vector<uint8_t>converted(8192);
   std::atomic<int>pulseA{0},pulseB{0};unsigned soundWait=0;bool soundOn=false;
   BuzzerFilter filter(cpu->frequency/std::max(1u,cpu->sound_divide_frequency));
   uint64_t resets=0;uint32_t minQueue=~0u,maxQueue=0;
   while(!stopRequested){
    auto now=clock::now();double elapsed=std::chrono::duration<double>(now-origin).count();
    uint64_t target=uint64_t(elapsed*cpu->frequency);
    if(target>done+cpu->frequency/2){ // suspend/resume: restart the wall clock, never race through seconds of game time
     origin=now-std::chrono::duration_cast<clock::duration>(std::chrono::duration<double>(done/double(cpu->frequency)));target=done;resets++;
    }
    unsigned steps=unsigned(std::min<uint64_t>(target>done?target-done:0,cpu->frequency/20));
    uint32_t keys=controls.load()&~(CTL_START|CTL_SELECT);
    if(bootMode>=0&&done>=cpu->frequency*2&&done<cpu->frequency*2+cpu->frequency/4)keys|=bootMode==0?CTL_START:CTL_SELECT;
    if(input)yokoi_input_apply_controller_mask(input,keys,0,pulseA,pulseB);
    bool changed=false;
    for(unsigned i=0;i<steps;i++){
     cpu->step();++done;
     if(cpu->segments_state_are_update){
      for(auto&s:segs){bool n=cpu->get_segments_state(s.id[0],s.id[1],s.id[2]);bool b=(s.state&&(n||s.buffer_state))||(n&&s.buffer_state);s.state=n;s.buffer_state=b;}
      cpu->segments_state_are_update=false;changed=true;
     }
     soundOn|=cpu->get_active_sound();
     if(++soundWait>=std::max(1u,cpu->sound_divide_frequency)){pcm.push_back(filter.sample(soundOn,gain.load()));soundWait=0;soundOn=false;}
    }
    cycles=done;
    if(changed||now-lastSnapshot>=std::chrono::milliseconds(8)){
     std::lock_guard<std::mutex>lock(mutex);latest=segs;lastSnapshot=now;
    }
    if(now-lastAudio>=std::chrono::milliseconds(8)&&!pcm.empty()){
     if(audio&&stream){
      SDL_AudioStreamPut(stream,pcm.data(),int(pcm.size()*2));
      int available;while((available=SDL_AudioStreamAvailable(stream))>0){int n=SDL_AudioStreamGet(stream,converted.data(),std::min(available,int(converted.size())));if(n<=0)break;SDL_QueueAudio(audio,converted.data(),n);}
      uint32_t queued=SDL_GetQueuedAudioSize(audio);minQueue=std::min(minQueue,queued);maxQueue=std::max(maxQueue,queued);
     }
     pcm.clear();lastAudio=now;
    }
    if(now-lastReport>=std::chrono::seconds(5)){
     double real=std::chrono::duration<double>(now-origin).count();
     printf("Timing: emulated=%.3fs wall=%.3fs speed=%.1f%% audio_queue=%u..%u bytes clock_resets=%llu\n",done/double(cpu->frequency),real,real>0?100*done/cpu->frequency/real:100.,minQueue==~0u?0:minQueue,maxQueue,(unsigned long long)resets);
     lastReport=now;minQueue=~0u;maxQueue=0;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
   }
   double wall=std::chrono::duration<double>(clock::now()-origin).count();
   printf("Timing final: cycles=%llu emulated=%.3fs wall=%.3fs speed=%.1f%%\n",(unsigned long long)done,done/double(cpu->frequency),wall,wall>0?100*done/cpu->frequency/wall:100.);

  });
 }
};
