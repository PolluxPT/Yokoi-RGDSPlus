"""Generate original diagnostic assets and zero-filled ROMs, not commercial games."""
import ast,struct,os
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[2]
source=(root/'CONVERT_ROM/convert_3ds.py').read_text()
node=next(n for n in ast.parse(source).body if isinstance(n,ast.FunctionDef) and n.name=='write_rom_pack_v1')
ns=dict(os=os,struct=struct,ROMPACK_FORMAT_VERSION=3,ROMPACK_CONTENT_VERSION=3,MANUFACTURER_NINTENDO='Nintendo',_manufacturer_to_id=lambda _:0)
exec(compile(ast.Module(body=[node],type_ignores=[]),'pack_writer','exec'),ns)
out=Path('/tmp/yokoi-fixtures');out.mkdir(exist_ok=True)
for typ,size,two,flags in [('sm5a',1856,False,0),('sm510',4096,True,0),('sm511',4096,True,2)]:
 rom=bytearray(size)
 if typ=='sm511':rom[704]=1
 (out/'test.rom').write_bytes(rom)
 (out/'test.melody').write_bytes(bytes(256))
 for prefix,color in [('segment',(255,255,255,255)),('background',(160,180,140,255)),('console',(30,80,100,255))]:
  im=Image.new('RGBA',(128,128),color);ImageDraw.Draw(im).rectangle((10,10,100,100),outline=(255,0,0,255),width=4);im.save(out/f'{prefix}_test.png')
 segs=[dict(id0=0,id1=0,id2=0,pos_scr_x=15,pos_scr_y=15,pos_tex_x=0,pos_tex_y=0,size_tex_x=32,size_tex_y=32,color_index=0,screen=s) for s in range(2 if two else 1)]
 g=dict(key='test',display_name='SYNTHETIC '+typ,ref='AC-01' if typ=='sm5a' else 'TEST',date='2026',rom_path=str(out/'test.rom'),melody_path=str(out/'test.melody') if typ=='sm511' else '',path_segment='segment_test.png',segments=segs,segment_info=[128,128,1,flags,128,128]+([128,128] if two else []),path_background='background_test.png',background_info=[128,128]+[0,0,128,128]*(2 if two else 1)+[0],path_console='console_test.png',console_info=[128,128,0,0,128,128])
 ns['write_rom_pack_v1']([g],str(out),str(out/f'{typ}.ykp'),platform=2)
