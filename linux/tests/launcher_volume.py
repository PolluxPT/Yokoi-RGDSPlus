"""Run the real launcher against isolated fake sysfs files; never access device sysfs."""
from pathlib import Path
import os,signal,subprocess,tempfile,time
source=(Path(__file__).resolve().parents[1]/'Yokoi.sh').read_text()
for entry,code,terminate in [(0,0,False),(4,7,False),(10,0,True)]:
 with tempfile.TemporaryDirectory(prefix='yokoi-launcher-') as d:
  root=Path(d);app=root/'Yokoi_RGDSPlus';app.mkdir();volume=root/'volume';volume.write_text(str(entry));touch=root/'touch';touch.write_text('1')
  launcher=root/'Yokoi.sh';launcher.write_text(source.replace('/sys/class/anbernic_misc/openbor_volume',str(volume)).replace('/sys/class/anbernic_misc/tpctrl',str(touch)).replace('[ -S "$XDG_RUNTIME_DIR/$WAYLAND_DISPLAY" ]','[ -e "$XDG_RUNTIME_DIR/$WAYLAND_DISPLAY" ]'))
  binpath=root/'bin';binpath.mkdir();uname=binpath/'uname';uname.write_text('#!/bin/sh\nprintf "aarch64\\n"\n');uname.chmod(0o755)
  ready=root/'ready';program=app/'yokoi';program.write_text('#!/usr/bin/python3\nfrom pathlib import Path\nimport time\n'+f'Path({str(volume)!r}).write_text("9")\nPath({str(ready)!r}).touch()\n'+('time.sleep(30)\n' if terminate else f'raise SystemExit({code})\n'));program.chmod(0o755)
  (root/'wayland-0').touch() # Placeholder for the Wayland presence check in this isolated test.
  env=dict(os.environ,PATH=str(binpath)+':'+os.environ['PATH'],XDG_RUNTIME_DIR=str(root),WAYLAND_DISPLAY='wayland-0')
  process=subprocess.Popen(['bash',str(launcher)],env=env)
  if terminate:
   deadline=time.monotonic()+5
   while not ready.exists() and time.monotonic()<deadline:time.sleep(.01)
   assert ready.exists();process.send_signal(signal.SIGTERM)
  result=process.wait(timeout=8)
  assert result==(143 if terminate else code),(result,(app/'Yokoi_Arranque.txt').read_text())
  assert volume.read_text().strip()==str(entry);assert touch.read_text().strip()=='1'
print('PASS: launcher restores entry volume 0/4/10 and touch on normal exit, child failure and SIGTERM')
