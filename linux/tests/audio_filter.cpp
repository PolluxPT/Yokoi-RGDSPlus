#include "emu_worker.h"
#include <cassert>
int main(){
 BuzzerFilter filter(32768);std::vector<int16_t>samples;double mean=0,energy=0;
 for(int i=0;i<65536;i++){int16_t s=filter.sample((i&7)<4,.35f);if(i>=32768){samples.push_back(s);mean+=s;energy+=double(s)*s;}}
 assert(std::abs(mean/32768)<3);assert(sqrt(energy/32768)>1000);
 FILE*f=fopen("/tmp/yokoi03-tone.pcm","wb");assert(f);fwrite(samples.data(),2,samples.size(),f);fclose(f);
 int16_t tail=0;for(int i=0;i<32768;i++)tail=filter.sample(false,.35f);assert(tail==0);
 for(int i=0;i<32768;i++)tail=filter.sample((i&7)<4,0);assert(tail==0);
 puts("PASS: DC suppression, stable 4096Hz tone, silence and muted output");
}
