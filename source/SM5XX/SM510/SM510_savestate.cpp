#include "SM510.h"
// Complete RGDS snapshot, including execution phase and LCD/sound registers.
bool SM510::save_state(FILE* file){
 if(!save_common_state(file))return false;
 if(fwrite(&ram,sizeof ram,1,file)!=1)return false;
 if(fwrite(&segment_on,sizeof segment_on,1,file)!=1)return false;
 if(fwrite(&r_buffer_program_counter,sizeof r_buffer_program_counter,1,file)!=1)return false;
 if(fwrite(&w_shift_register,sizeof w_shift_register,1,file)!=1)return false;
 if(fwrite(&r_buzzer_control,sizeof r_buzzer_control,1,file)!=1)return false;
 if(fwrite(&r_buzzer_output,sizeof r_buzzer_output,1,file)!=1)return false;
 if(fwrite(&bc_lcd_stop,sizeof bc_lcd_stop,1,file)!=1)return false;
 if(fwrite(&l_bs,sizeof l_bs,1,file)!=1)return false;
 if(fwrite(&y_bs,sizeof y_bs,1,file)!=1)return false;
 if(fwrite(&alternativ_col_ram,sizeof alternativ_col_ram,1,file)!=1)return false;
 return true;
}
bool SM510::load_state(FILE* file){
 if(!load_common_state(file))return false;
 if(fread(&ram,sizeof ram,1,file)!=1)return false;
 if(fread(&segment_on,sizeof segment_on,1,file)!=1)return false;
 if(fread(&r_buffer_program_counter,sizeof r_buffer_program_counter,1,file)!=1)return false;
 if(fread(&w_shift_register,sizeof w_shift_register,1,file)!=1)return false;
 if(fread(&r_buzzer_control,sizeof r_buzzer_control,1,file)!=1)return false;
 if(fread(&r_buzzer_output,sizeof r_buzzer_output,1,file)!=1)return false;
 if(fread(&bc_lcd_stop,sizeof bc_lcd_stop,1,file)!=1)return false;
 if(fread(&l_bs,sizeof l_bs,1,file)!=1)return false;
 if(fread(&y_bs,sizeof y_bs,1,file)!=1)return false;
 if(fread(&alternativ_col_ram,sizeof alternativ_col_ram,1,file)!=1)return false;
 return true;
}
