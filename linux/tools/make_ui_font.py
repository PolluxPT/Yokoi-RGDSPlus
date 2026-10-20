from PIL import Image,ImageDraw,ImageFont
from pathlib import Path
import io,math
im=Image.new('RGBA',(2048,2048));d=ImageDraw.Draw(im);x=y=2;rowh=0;allg=[]
for size,bold in [(20,False),(26,False),(32,True),(40,True),(52,True)]:
 font=ImageFont.truetype('/usr/share/fonts/truetype/dejavu/DejaVuSans'+('-Bold' if bold else '')+'.ttf',size);glyphs=[]
 for code in range(32,256):
  char=chr(code);w=math.ceil(font.getlength(char))+3;h=math.ceil(size*1.45)
  if x+w+2>2048:x=2;y+=rowh+2;rowh=0
  d.text((x+1,y),char,font=font,fill='white');glyphs.append((x,y,w,h,w-1));x+=w+2;rowh=max(rowh,h)
 allg.append(glyphs)
im=im.crop((0,0,2048,y+rowh+2));buf=io.BytesIO();im.save(buf,format='PNG');b=buf.getvalue()
s='#pragma once\n// DejaVu Sans glyph atlas; font license accompanies distribution.\nstruct UiGlyph{int x,y,w,h,advance;};\nstatic const UiGlyph ui_glyphs[5][224]={\n'
for g in allg:s+='{'+','.join('{'+','.join(map(str,v))+'}' for v in g)+'},\n'
s+='};\nstatic const unsigned char ui_font_png[]={'+','.join(map(str,b))+'};\n'
(Path(__file__).resolve().parents[1]/'ui_font_data.h').write_text(s)
print('font PNG bytes',len(b),'atlas',im.size)
