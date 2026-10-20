#include "SM5A.h"
// Complete RGDS snapshot, including execution phase and LCD/sound registers.
bool SM5A::save_state(FILE* file){
 if(!save_common_state(file))return false;
 if(fwrite(&ram,sizeof ram,1,file)!=1)return false;
 if(fwrite(&segment_on,sizeof segment_on,1,file)!=1)return false;
 if(fwrite(&cb_debordement_rom_program_counter,sizeof cb_debordement_rom_program_counter,1,file)!=1)return false;
 if(fwrite(&w_screen_control,sizeof w_screen_control,1,file)!=1)return false;
 if(fwrite(&w_prime_screen_control,sizeof w_prime_screen_control,1,file)!=1)return false;
 if(fwrite(&w_size,sizeof w_size,1,file)!=1)return false;
 if(fwrite(&last_w_update,sizeof last_w_update,1,file)!=1)return false;
 if(fwrite(&cn_flag,sizeof cn_flag,1,file)!=1)return false;
 if(fwrite(&r_subroutine_flag,sizeof r_subroutine_flag,1,file)!=1)return false;
 if(fwrite(&e_temporar_flag,sizeof e_temporar_flag,1,file)!=1)return false;
 if(fwrite(&r_output_control,sizeof r_output_control,1,file)!=1)return false;
 if(fwrite(&m_flag_segment_decoder,sizeof m_flag_segment_decoder,1,file)!=1)return false;
 return true;
}
bool SM5A::load_state(FILE* file){
 if(!load_common_state(file))return false;
 if(fread(&ram,sizeof ram,1,file)!=1)return false;
 if(fread(&segment_on,sizeof segment_on,1,file)!=1)return false;
 if(fread(&cb_debordement_rom_program_counter,sizeof cb_debordement_rom_program_counter,1,file)!=1)return false;
 if(fread(&w_screen_control,sizeof w_screen_control,1,file)!=1)return false;
 if(fread(&w_prime_screen_control,sizeof w_prime_screen_control,1,file)!=1)return false;
 if(fread(&w_size,sizeof w_size,1,file)!=1)return false;
 if(fread(&last_w_update,sizeof last_w_update,1,file)!=1)return false;
 if(fread(&cn_flag,sizeof cn_flag,1,file)!=1)return false;
 if(fread(&r_subroutine_flag,sizeof r_subroutine_flag,1,file)!=1)return false;
 if(fread(&e_temporar_flag,sizeof e_temporar_flag,1,file)!=1)return false;
 if(fread(&r_output_control,sizeof r_output_control,1,file)!=1)return false;
 if(fread(&m_flag_segment_decoder,sizeof m_flag_segment_decoder,1,file)!=1)return false;
 return true;
}
