################################################################
#
# $Id:$
#
# $Log:$
#

ifeq ($(V),1)
	VB=''
else
	VB=@
endif


CC=gcc  # gcc or g++
CFLAGS+=-ggdb3 -o0
#LDFLAGS+=-Wl,--gc-sections -static -nostdlib -no-pie ,-m elf_i386

LDFLAGS += -Wl,--gc-sections,-melf_i386,-static,-nostdlib,-no-pie
#CFLAGS+=-ggdb3 -Wall -DNORMALUNIX -DLINUX -DSNDSERV -D_DEFAULT_SOURCE # -DUSEASM
#LIBS+=-lm -lc -lX11

CFLAGS+=-nostdinc -I ~/eehf-os/lib/ -I ~/eehf-os/lib/kernel/ -I ~/eehf-os/lib/user/ -I \
      ~/eehf-os/kernel/ -I ~/eehf-os/device/ -I ~/eehf-os/thread/ -I \
      ~/eehf-os/userprog/ -I ~/eehf-os/fs/ -I ~/eehf-os/shell/ \
			-I ~/eehf-os/net/ -I ~/eehf-os/device/ -I ~/eehf-os/math/ \
			-I ./
CFLAGS+=-Wall -fno-builtin -W -Wstrict-prototypes \
    -Wmissing-prototypes -Wsystem-headers -m32 -fno-stack-protector -static\
    -no-pie -ffreestanding -nostdlib -D_FORTIFY_SOURCE=0 -mno-mmx -mno-3dnow 

# subdirectory for objects
OBJDIR=build
OUTPUT=doomgeneric

OBJDIR_EEHF=~/eehf-os/build


SRC_DOOM = dummy.o am_map.o doomdef.o doomstat.o dstrings.o d_event.o d_items.o d_iwad.o d_loop.o d_main.o d_mode.o d_net.o f_finale.o f_wipe.o g_game.o hu_lib.o hu_stuff.o info.o i_cdmus.o i_endoom.o i_joystick.o i_scale.o i_sound.o i_system.o i_timer.o memio.o m_argv.o m_bbox.o m_cheat.o m_config.o m_controls.o m_fixed.o m_menu.o m_misc.o m_random.o p_ceilng.o p_doors.o p_enemy.o p_floor.o p_inter.o p_lights.o p_map.o p_maputl.o p_mobj.o p_plats.o p_pspr.o p_saveg.o p_setup.o p_sight.o p_spec.o p_switch.o p_telept.o p_tick.o p_user.o r_bsp.o r_data.o r_draw.o r_main.o r_plane.o r_segs.o r_sky.o r_things.o sha1.o sounds.o statdump.o st_lib.o st_stuff.o s_sound.o tables.o v_video.o wi_stuff.o w_checksum.o w_file.o w_main.o w_wad.o z_zone.o w_file_stdc.o i_input.o i_video.o doomgeneric.o doomgeneric_eehf.o
OBJS += $(addprefix $(OBJDIR)/, $(SRC_DOOM))
OBJS += ~/eehf-os/command/start.o
SRC_EEHF = lib/user/syscall.o math/math.o lib/stdio.o lib/string.o lib/user/assert.o fs/file.o lib/strings.o lib/ctype.o 
OBJS += $(addprefix $(OBJDIR_EEHF)/, $(SRC_EEHF))


all:	 $(OUTPUT)

clean:
	rm -rf $(OBJDIR)
	rm -f $(OUTPUT)
	rm -f $(OUTPUT).gdb
	rm -f $(OUTPUT).map

$(OUTPUT):	$(OBJS)
	@echo [Linking $@]
	$(VB)$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) \
	-o $(OUTPUT) $(LIBS) -Wl,-Map,$(OUTPUT).map
	@echo [Size]
	-$(CROSS_COMPILE)size $(OUTPUT)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o:	%.c
	@echo [Compiling $<]
	$(VB)$(CC) $(CFLAGS) -c $< -o $@

print:
	@echo OBJS: $(OBJS)

