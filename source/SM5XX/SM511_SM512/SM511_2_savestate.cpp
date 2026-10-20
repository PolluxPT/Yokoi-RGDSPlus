#include "SM511_2.h"
// Complete RGDS snapshot, including execution phase and LCD/sound registers.
bool SM511_2::save_state(FILE* file){
 if(!save_common_state(file))return false;
 if(fwrite(&ram,sizeof ram,1,file)!=1)return false;
 if(fwrite(&segment_on,sizeof segment_on,1,file)!=1)return false;
 if(fwrite(&r_buffer_program_counter,sizeof r_buffer_program_counter,1,file)!=1)return false;
 if(fwrite(&rom_melody_address,sizeof rom_melody_address,1,file)!=1)return false;
 if(fwrite(&w_shift_register,sizeof w_shift_register,1,file)!=1)return false;
 if(fwrite(&s_pin,sizeof s_pin,1,file)!=1)return false;
 if(fwrite(&bc_lcd_stop,sizeof bc_lcd_stop,1,file)!=1)return false;
 if(fwrite(&l_bs,sizeof l_bs,1,file)!=1)return false;
 if(fwrite(&x_bs,sizeof x_bs,1,file)!=1)return false;
 if(fwrite(&y_bs,sizeof y_bs,1,file)!=1)return false;
 if(fwrite(&alternativ_col_ram,sizeof alternativ_col_ram,1,file)!=1)return false;
 if(fwrite(&me_melody_activate,sizeof me_melody_activate,1,file)!=1)return false;
 if(fwrite(&mes_melody_finish,sizeof mes_melody_finish,1,file)!=1)return false;
 if(fwrite(&melody_cycle_count,sizeof melody_cycle_count,1,file)!=1)return false;
 if(fwrite(&nb_cycle_need_for_change_note,sizeof nb_cycle_need_for_change_note,1,file)!=1)return false;
 if(fwrite(&curr_note_melody,sizeof curr_note_melody,1,file)!=1)return false;
 if(fwrite(&curr_phase,sizeof curr_phase,1,file)!=1)return false;
 if(fwrite(&cycle_in_curr_phase,sizeof cycle_in_curr_phase,1,file)!=1)return false;
 return true;
}
bool SM511_2::load_state(FILE* file){
 if(!load_common_state(file))return false;
 if(fread(&ram,sizeof ram,1,file)!=1)return false;
 if(fread(&segment_on,sizeof segment_on,1,file)!=1)return false;
 if(fread(&r_buffer_program_counter,sizeof r_buffer_program_counter,1,file)!=1)return false;
 if(fread(&rom_melody_address,sizeof rom_melody_address,1,file)!=1)return false;
 if(fread(&w_shift_register,sizeof w_shift_register,1,file)!=1)return false;
 if(fread(&s_pin,sizeof s_pin,1,file)!=1)return false;
 if(fread(&bc_lcd_stop,sizeof bc_lcd_stop,1,file)!=1)return false;
 if(fread(&l_bs,sizeof l_bs,1,file)!=1)return false;
 if(fread(&x_bs,sizeof x_bs,1,file)!=1)return false;
 if(fread(&y_bs,sizeof y_bs,1,file)!=1)return false;
 if(fread(&alternativ_col_ram,sizeof alternativ_col_ram,1,file)!=1)return false;
 if(fread(&me_melody_activate,sizeof me_melody_activate,1,file)!=1)return false;
 if(fread(&mes_melody_finish,sizeof mes_melody_finish,1,file)!=1)return false;
 if(fread(&melody_cycle_count,sizeof melody_cycle_count,1,file)!=1)return false;
 if(fread(&nb_cycle_need_for_change_note,sizeof nb_cycle_need_for_change_note,1,file)!=1)return false;
 if(fread(&curr_note_melody,sizeof curr_note_melody,1,file)!=1)return false;
 if(fread(&curr_phase,sizeof curr_phase,1,file)!=1)return false;
 if(fread(&cycle_in_curr_phase,sizeof cycle_in_curr_phase,1,file)!=1)return false;
 return true;
}
