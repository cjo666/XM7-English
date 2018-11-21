#
# FM-7 EMULATOR "XM7"
#
# Copyright (C) 1999-2017 ＰＩ．(Twitter:@xm6_original)
# Copyright (C) 2001-2017 Ryu Takegami (Twitter:@RyuTakegami)
# Copyright (C) 2010-2014 Ben.JP
#
# [ Makefile for Microsoft Visual C++ 2005/2008/2010/2012 ]
#

#
# 環境定義
#
LIBPATH		= ".\\sdk-directx9\\Lib"
			# ご自身の環境に合わせて変更してください。

#
# バージョン定義
#
XM7_VER		= 3

#
# オプション定義
#
OPTION		= -DMOUSE -DMR2 -DROMEO -DMIDI -DFDDSND -DRSC -DKBDPASTE -DLPRINT -DJASTSOUND

#
# ツール定義
#
#AS			= nasmw
AS			= ".\\nasm\\nasm"	
# nasmw 0.98 が調達できず、NASM 2.x で代用する場合
AFLAGS		= -d _WIN32 -f win32
ASMVER		= -d XM7_VER=$(XM7_VER) $(OPTION)

CC			= cl
CFLAGS		= /c /DNDEBUG /O2 /wd4430
CVER		= -DXM7_VER=$(XM7_VER) $(OPTION)

LD			= link
LFLAGS		= /LIBPATH:$(LIBPATH) /MAP

RC   		= rc
RFLAGS		= /c932
RCVER		= -dXM7_VER=$(XM7_VER) $(OPTION)

#
# ファイル定義
#
TARGET		= XM7.exe



ASM_SRCS	= \
		cpu_x86.asm \
		vm_sub.asm \
		w32_sub.asm \
		ddrend16.asm \
		ddrend24.asm \
		ddrend32.asm \
		gdi_rend.asm

ASM_OBJS	= \
		cpu_x86.obj \
		vm_sub.obj \
		w32_sub.obj \
		ddrend16.obj \
		ddrend24.obj \
		ddrend32.obj \
		gdi_rend.obj

C_SRCS		= \
		aluline.c \
		apalet.c \
		disasm.c \
		disasm80.c \
		display.c \
		fdc.c \
		jcard.c \
		jsubsys.c \
		kanji.c \
		keyboard.c \
		kmz80.c \
		kmz80c.c \
		kmz80t.c \
		maincpu.c \
		mainetc.c \
		mainmem.c \
		mouse.c \
		mmr.c \
		multipag.c \
		opn.c \
		ptm.c \
		rs232c.c \
		rtc.c \
		schedule.c \
		subcpu.c \
		subctrl.c \
		submem.c \
		system.c \
		tapelp.c \
		tools.c \
		ttlpalet.c \
		dmac.c \
		midi.c \
		w32_bar.c \
		w32_cfg.c \
		w32_cmd.c \
		w32_comm.c \
		w32_draw.c \
		w32_file.c \
		w32_gdi.c \
		w32_main.c \
		w32_midi.c \
		w32_sch.c \
		w32_sub1.c \
		w32_sub2.c \
		w32_sub3.c \
		w32_ver.c

C_OBJS		= \
		aluline.obj \
		apalet.obj \
		disasm.obj \
		disasm80.obj \
		display.obj \
		fdc.obj \
		jcard.obj \
		jsubsys.obj \
		kanji.obj \
		keyboard.obj \
		kmz80.obj \
		kmz80c.obj \
		kmz80t.obj \
		maincpu.obj \
		mainetc.obj \
		mainmem.obj \
		mouse.obj \
		mmr.obj \
		multipag.obj \
		opn.obj \
		ptm.obj \
		rs232c.obj \
		rtc.obj \
		schedule.obj \
		subcpu.obj \
		subctrl.obj \
		submem.obj \
		system.obj \
		tapelp.obj \
		tools.obj \
		ttlpalet.obj \
		dmac.obj \
		midi.obj \
		w32_bar.obj \
		w32_cfg.obj \
		w32_cmd.obj \
		w32_comm.obj \
		w32_draw.obj \
		w32_file.obj \
		w32_gdi.obj \
		w32_main.obj \
		w32_midi.obj \
		w32_sch.obj \
		w32_sub1.obj \
		w32_sub2.obj \
		w32_sub3.obj \
		w32_ver.obj

CPP_SRCS	= \
		fmgen.cpp \
		opna.cpp \
		psg.cpp \
		w32_dd.cpp \
		w32_kbd.cpp \
		w32_lpr.cpp \
		w32_snd.cpp \
		juliet.cpp

CPP_OBJS	= \
		fmgen.obj \
		opna.obj \
		psg.obj \
		w32_dd.obj \
		w32_kbd.obj \
		w32_lpr.obj \
		w32_snd.obj \
		juliet.obj

RC_SRCS		= \
		w32_res.rc

RC_OBJS		= \
		w32_res.res

LIBFILES	= \
		  kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib \
		  winmm.lib imm32.lib ddraw.lib dinput.lib dsound.lib dxguid.lib ole32.lib

MAPFILES	= 

DEFFILES	=

#
# デフォルト生成ルール
#
.asm.obj:
		$(AS) $(AFLAGS) $(ASMVER) $<

.c.obj:
		$(CC) $(CFLAGS) $(CVER) $<

.cpp.obj:
		$(CC) $(CFLAGS) $(CVER) $<

.rc.res:
		$(RC) $(RFLAGS) $(RCVER) $<

#
# ソースファイル→ターゲット
#
$(TARGET) : $(ASM_OBJS) $(C_OBJS) $(CPP_OBJS) $(RC_OBJS)
		$(LD) $(LFLAGS) \
		/OUT:$@ \
		$(ASM_OBJS) \
		$(C_OBJS) \
		$(CPP_OBJS) \
		$(RC_OBJS) \
		$(LIBFILES) \
		$(MAPFILES) \
		$(DEFFILES) 

#
# 依存関係
#
aluline.obj	: aluline.c xm7.h aluline.h device.h event.h display.h subctrl.h multipag.h
apalet.obj	: apalet.c xm7.h apalet.h device.h display.h subctrl.h
disasm.obj	: disasm.c xm7.h
display.obj	: display.c xm7.h display.h device.h subctrl.h ttlpalet.h multipag.h mainetc.h aluline.h keyboard.h kanji.h event.h
dmac.obj	: dmac.c xm7.h dmac.h device.h event.h mainetc.h fdc.h mmr.h
fdc.obj		: fdc.c xm7.h fdc.h device.h event.h mainetc.h dmac.h
jcard.obj	: jcard.c xm7.h jcard.h device.h kanji.h
kanji.obj	: kanji.c xm7.h kanji.h device.h jcard.h
keyboard.obj: keyboard.c xm7.h keyboard.h device.h event.h mainetc.h rtc.h
maincpu.obj	: maincpu.c xm7.h device.h mainetc.h subctrl.h keyboard.h tapelp.h
mainetc.obj	: mainetc.c xm7.h mainetc.h device.h event.h keyboard.h opn.h
mainmem.obj	: mainmem.c xm7.h device.h subctrl.h ttlpalet.h fdc.h mainetc.h multipag.h kanji.h tapelp.h opn.h mmr.h apalet.h jcard.h dmac.h
mmr.obj		: mmr.c xm7.h mmr.h device.h subctrl.h jcard.h
mouse.obj	: mouse.c xm7.h mouse.h device.h event.h mainetc.h
multipag.obj: multipag.c xm7.h multipag.h device.h display.h ttlpalet.h subctrl.h apalet.h
opn.obj		: opn.c xm7.h opn.h device.h event.h mainetc.h mouse.h
rtc.obj		: rtc.c xm7.h rtc.h device.h event.h
schedule.obj: schedule.c xm7.h device.h event.h subctrl.h display.h mmr.h dmac.h
subcpu.obj	: subcpu.c xm7.h device.h event.h subctrl.h mainetc.h
subctrl.obj	: subctrl.c xm7.h subctrl.h device.h keyboard.h display.h aluline.h ttlpalet.h multipag.h
submem.obj	: submem.c xm7.h device.h display.h subctrl.h keyboard.h multipag.h aluline.h
system.obj	: system.c xm7.h device.h event.h display.h ttlpalet.h subctrl.h keyboard.h fdc.h mainetc.h multipag.h kanji.h tapelp.h display.h opn.h mmr.h aluline.h apalet.h rtc.h mouse.h jcard.h dmac.h
tapelp.obj	: tapelp.c xm7.h tapelp.h device.h event.h mainetc.h
#thg.obj		: thg.c xm7.h opn.h device.h event.h mainetc.h thg.h
tools.obj	: tools.c xm7.h tools.h device.h multipag.h ttlpalet.h apalet.h subctrl.h display.h
ttlpalet.obj: ttlpalet.c xm7.h ttlpalet.h device.h display.h subctrl.h
#whg.obj		: whg.c xm7.h opn.h device.h event.h mainetc.h whg.h
fmgen.obj	: fmgen.cpp cisc.h fmgen.h fmgeninl.h
opna.obj	: opna.cpp cisc.h opna.h fmgeninl.h
psg.obj		: psg.cpp cisc.h psg.h
w32_bar.obj	: w32_bar.c xm7.h w32.h w32_bar.h keyboard.h tapelp.h display.h ttlpalet.h apalet.h subctrl.h fdc.h w32_sch.h w32_draw.h w32_res.h
w32_cfg.obj	: w32_cfg.c xm7.h w32.h w32_cfg.h device.h opn.h keyboard.h mmr.h mouse.h aluline.h w32_sch.h w32_snd.h w32_kbd.h w32_dd.h w32_draw.h w32_res.h
w32_cmd.obj	: w32_cmd.c xm7.h w32.h fdc.h tapelp.h tools.h mouse.h rtc.h w32_bar.h w32_draw.h w32_snd.h w32_sch.h w32_sub.h w32_cfg.h w32_res.h
w32_dd.obj	: w32_dd.cpp xm7.h w32.h w32_dd.h subctrl.h display.h multipag.h ttlpalet.h apalet.h fdc.h tapelp.h keyboard.h opn.h w32_bar.h w32_sch.h w32_res.h w32_draw.h w32_kbd.h
w32_draw.obj: w32_draw.c xm7.h w32.h w32_draw.h device.h subctrl.h display.h multipag.h w32_gdi.h w32_dd.h
w32_file.obj: w32_file.c xm7.h device.h
w32_gdi.obj	: w32_gdi.c xm7.h w32.h w32_gdi.h subctrl.h display.h multipag.h ttlpalet.h apalet.h mouse.h w32_draw.h w32_kbd.h
w32_kbd.obj	: w32_kbd.cpp xm7.h w32.h w32_kbd.h device.h event.h mainetc.h keyboard.h mouse.h w32_sch.h w32_bar.h w32_res.h
w32_lpr.obj	: w32_lpr.cpp xm7.h w32.h device.h w32_lpr.h w32_sch.h w32_snd.h
w32_main.obj: w32_main.c xm7.h w32.h mouse.h w32_bar.h w32_draw.h w32_dd.h w32_gdi.h w32_kbd.h w32_sch.h w32_snd.h w32_res.h w32_sub.h w32_cfg.h
w32_sch.obj	: w32_sch.c xm7.h w32.h w32_sch.h device.h tapelp.h display.h subctrl.h w32_draw.h w32_snd.h w32_kbd.h w32_bar.h w32_sub.h
w32_snd.obj	: w32_snd.cpp xm7.h w32.h w32_snd.h device.h mainetc.h opn.h tapelp.h cisc.h opna.h psg.h w32_sch.h
w32_sub1.obj: w32_sub1.c xm7.h w32.h w32_sub.h event.h w32_res.h w32_sch.h
w32_sub2.obj: w32_sub2.c xm7.h w32.h w32_sub.h fdc.h opn.h subctrl.h dmac.h display.h aluline.h w32_res.h w32_snd.h w32_kbd.h w32_draw.h
w32_sub3.obj: w32_sub3.c xm7.h w32.h w32_sub.h keyboard.h rtc.h mmr.h jcard.h dmac.h w32_res.h
w32_ver.obj	: w32_ver.c xm7.h w32.h w32_res.h

#
# その他
#
clean:
	del *.obj
	del *.res

w32cln:
	del w32_*.obj
	del w32_*.res

allclean: clean
	del *.map
	del *.INI
	del *.DAT

clobber: allclean
	del *.exe
