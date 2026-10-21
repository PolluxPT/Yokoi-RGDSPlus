#include <SDL.h>
#include <cassert>
#include <vector>

static Uint8 deviceChannels=1;
static bool negotiateChannels=false;
static std::vector<Uint8>openAttempts;
static SDL_AudioDeviceID openTestAudioDevice(const char*device,int capture,const SDL_AudioSpec*want,SDL_AudioSpec*have,int allowed){
 openAttempts.push_back(want->channels);
 assert(allowed==SDL_AUDIO_ALLOW_CHANNELS_CHANGE);
 if(!deviceChannels||(!negotiateChannels&&want->channels!=deviceChannels)){
  SDL_SetError("ALSA: Couldn't set audio channels");return 0;
 }
 SDL_AudioSpec supported=*want;supported.channels=deviceChannels;
 return SDL_OpenAudioDevice(device,capture,&supported,have,0);
}
#define SDL_OpenAudioDevice openTestAudioDevice
#define main yokoi_program
#include "../main.cpp"
#undef main
#undef SDL_OpenAudioDevice

int main(){
 assert(SDL_setenv("SDL_AUDIODRIVER","dummy",1)==0);
 assert(SDL_Init(SDL_INIT_AUDIO)==0);
 for(bool negotiate:{false,true})for(Uint8 channels:{1,2}){
  deviceChannels=channels;negotiateChannels=negotiate;openAttempts.clear();
  SDL_AudioSpec spec{};auto audio=openAudioOutput(spec);assert(audio);
  assert(openAttempts.front()==2);
  if(!negotiate&&channels==1){assert(openAttempts.size()==2);assert(openAttempts.back()==1);}
  else assert(openAttempts.size()==1);
  assert(spec.channels==channels);assert(spec.freq==48000);assert(spec.format==AUDIO_S16SYS);
  for(int pass=0;pass<2;pass++){
   SDL_ClearQueuedAudio(audio);assert(prefillAudio(audio,spec));
   assert(SDL_GetQueuedAudioSize(audio)==1920u*channels*sizeof(int16_t));
  }
  auto*stream=SDL_NewAudioStream(AUDIO_S16SYS,1,spec.freq,spec.format,spec.channels,spec.freq);assert(stream);
  int16_t mono[]={1000,-2000,3000,-4000};
  assert(SDL_AudioStreamPut(stream,mono,sizeof mono)==0);
  std::vector<int16_t>converted(4*channels);
  assert(SDL_AudioStreamGet(stream,converted.data(),int(converted.size()*sizeof(int16_t)))==int(converted.size()*sizeof(int16_t)));
  for(size_t frame=0;frame<4;frame++)for(Uint8 channel=0;channel<channels;channel++)assert(converted[frame*channels+channel]==mono[frame]);
  SDL_ClearQueuedAudio(audio);
  assert(SDL_QueueAudio(audio,converted.data(),Uint32(converted.size()*sizeof(int16_t)))==0);
  assert(SDL_GetQueuedAudioSize(audio)==converted.size()*sizeof(int16_t));
  SDL_FreeAudioStream(stream);SDL_CloseAudioDevice(audio);
 }
 deviceChannels=0;openAttempts.clear();
 SDL_AudioSpec spec{};assert(openAudioOutput(spec)==0);
 assert((openAttempts==std::vector<Uint8>{2,1}));
 SDL_Quit();
 puts("PASS: stereo-first opening, mono fallback, negotiation, total failure, sample conversion and 40ms prefill");
}
