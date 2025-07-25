#定义一大堆变量，实质就是将需要多次重复用到的语句定义一个变量方便使用与替换
BUILD_DIR=./build
ENTRY_POINT=0xc0001500
HD60M_PATH=./hd3M.img
#只需要把hd60m.img路径改成自己环境的路径，整个代码直接make all就完全写入了，能够运行成功
AS=nasm
CC=gcc
LD=ld
LIB= -I tool/ -I device/vga/ -I net/ -I device/net -I lib/ -I lib/kernel/ -I lib/user/ -I kernel/ -I device/ -I thread/ -I userprog/	-I fs/ -I shell/
ASFLAGS= -f elf -g
CFLAGS= -Wall $(LIB) -c -fno-builtin -W -Wstrict-prototypes -Wmissing-prototypes -m32 -fno-stack-protector -g
#-Wall warning all的意思，产生尽可能多警告信息，-fno-builtin不要采用内部函数，
#-W 会显示警告，但是只显示编译器认为会出现错误的警告
#-Wstrict-prototypes 要求函数声明必须有参数类型，否则发出警告。-Wmissing-prototypes 必须要有函数声明，否则发出警告

LDFLAGS= -Ttext $(ENTRY_POINT) -e main -Map $(BUILD_DIR)/kernel.map -m elf_i386
#-Map,生成map文件，就是通过编译器编译之后，生成的程序、数据及IO空间信息的一种映射文件
#里面包含函数大小，入口地址等一些重要信息

OBJS=$(BUILD_DIR)/main.o $(BUILD_DIR)/init.o \
	$(BUILD_DIR)/interrupt.o $(BUILD_DIR)/timer.o $(BUILD_DIR)/kernel.o \
	$(BUILD_DIR)/print.o $(BUILD_DIR)/debug.o $(BUILD_DIR)/string.o $(BUILD_DIR)/bitmap.o \
	$(BUILD_DIR)/memory.o $(BUILD_DIR)/thread.o	$(BUILD_DIR)/list.o	$(BUILD_DIR)/switch.o \
	$(BUILD_DIR)/sync.o	$(BUILD_DIR)/console.o $(BUILD_DIR)/keyboard.o $(BUILD_DIR)/ioqueue.o \
	$(BUILD_DIR)/tss.o	$(BUILD_DIR)/process.o	$(BUILD_DIR)/syscall.o $(BUILD_DIR)/syscall-init.o \
	$(BUILD_DIR)/stdio.o $(BUILD_DIR)/stdio-kernel.o $(BUILD_DIR)/ide.o $(BUILD_DIR)/fs.o $(BUILD_DIR)/inode.o \
	$(BUILD_DIR)/file.o $(BUILD_DIR)/dir.o $(BUILD_DIR)/fork.o	$(BUILD_DIR)/shell.o $(BUILD_DIR)/buildin_cmd.o \
	$(BUILD_DIR)/exec.o $(BUILD_DIR)/assert.o $(BUILD_DIR)/wait_exit.o $(BUILD_DIR)/pipe.o\
$(BUILD_DIR)/pci.o $(BUILD_DIR)/e1000.o $(BUILD_DIR)/net.o $(BUILD_DIR)/eth.o $(BUILD_DIR)/arp.o\
$(BUILD_DIR)/dhcp.o $(BUILD_DIR)/ipv4.o $(BUILD_DIR)/socket.o $(BUILD_DIR)/udp.o $(BUILD_DIR)/icmp.o\
$(BUILD_DIR)/font.o $(BUILD_DIR)/vga.o $(BUILD_DIR)/psf2.o $(BUILD_DIR)/math.o
#顺序最好是调用在前，实现在后

######################编译两个启动文件的代码#####################################
boot:$(BUILD_DIR)/mbr.o $(BUILD_DIR)/loader.o
$(BUILD_DIR)/mbr.o:boot/mbr.S
	$(AS) -I boot/include/ -o build/mbr.o boot/mbr.S
	
$(BUILD_DIR)/loader.o:boot/loader.S
	$(AS) -I boot/include/ -o build/loader.o boot/loader.S
	
######################编译C内核代码###################################################
$(BUILD_DIR)/main.o:kernel/main.c
	$(CC) $(CFLAGS) -o $@ $<	
# $@表示规则中目标文件名的集合这里就是$(BUILD_DIR)/main.o  $<表示规则中依赖文件的第一个，这里就是kernle/main.c 

$(BUILD_DIR)/init.o:kernel/init.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/interrupt.o:kernel/interrupt.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/timer.o:device/timer.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/debug.o:kernel/debug.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/string.o:lib/string.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/bitmap.o:lib/kernel/bitmap.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/memory.o:kernel/memory.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/thread.o:thread/thread.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/list.o:lib/kernel/list.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/sync.o:thread/sync.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/console.o:device/console.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/keyboard.o:device/keyboard.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/ioqueue.o:device/ioqueue.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/tss.o:userprog/tss.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/process.o:userprog/process.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/syscall.o:lib/user/syscall.c
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/syscall-init.o:userprog/syscall-init.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/stdio.o:lib/stdio.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/stdio-kernel.o:lib/kernel/stdio-kernel.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/ide.o:device/ide.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/fs.o:fs/fs.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/inode.o:fs/inode.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/file.o:fs/file.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/dir.o:fs/dir.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/fork.o:userprog/fork.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/shell.o:shell/shell.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/buildin_cmd.o:shell/buildin_cmd.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/exec.o:userprog/exec.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/assert.o:lib/user/assert.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/wait_exit.o:userprog/wait_exit.c
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/pipe.o:shell/pipe.c
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/pci.o:device/pci.c
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/e1000.o:device/net/e1000.c
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/net.o:net/net.c
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/eth.o:net/eth.c
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/arp.o:net/arp.c
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/dhcp.o:net/dhcp.c
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/ipv4.o:net/ipv4.c
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/socket.o:net/socket.c
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/udp.o:net/udp.c
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/icmp.o:net/icmp.c
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/font.o:tool/font.c
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/vga.o:device/vga/vga.c
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/psf2.o:tool/psf2.c
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/math.o:math/math.c
	$(CC) $(CFLAGS) -o $@ $<
###################编译汇编内核代码#####################################################
$(BUILD_DIR)/kernel.o:kernel/kernel.S 
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/print.o:lib/kernel/print.S
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD_DIR)/switch.o:thread/switch.S
	$(AS) $(ASFLAGS) -o $@ $<

##################链接所有内核目标文件##################################################
$(BUILD_DIR)/kernel.bin:$(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^
# $^表示规则中所有依赖文件的集合，如果有重复，会自动去重

.PHONY:mk_dir hd clean build all boot gdb_symbol	#定义了7个伪目标
mk_dir:
	if [ ! -d $(BUILD_DIR) ];then mkdir $(BUILD_DIR);fi 
#判断build文件夹是否存在，如果不存在，则创建

hd:
	dd if=build/mbr.o of=$(HD60M_PATH) count=1 bs=512 conv=notrunc && \
	dd if=build/loader.o of=$(HD60M_PATH) count=10 bs=512 seek=2 conv=notrunc && \
	dd if=$(BUILD_DIR)/kernel.elf of=$(HD60M_PATH) bs=512 count=700 seek=9 conv=notrunc
	
clean:
	@cd $(BUILD_DIR) && rm -f ./* && echo "remove ./build all done"
#-f, --force忽略不存在的文件，从不给出提示，执行make clean就会删除build下所有文件

build:$(BUILD_DIR)/kernel.bin
#执行build需要依赖kernel.bin，但是一开始没有，就会递归执行之前写好的语句编译kernel.bin
qemu:
	sudo qemu-system-i386 -cpu pentium -drive file=hd3M.img,media=disk,index=0,format=raw\
		-drive file=hd80M.img,media=disk,index=1,format=raw\
		-no-reboot -no-shutdown\
		-netdev tap,id=net0,ifname=tap0,script=no,downscript=no\
		-device e1000,netdev=net0,mac=00:FF:26:E4:55:94\
		-vga std
qemu-gdb:
	sudo qemu-system-i386 -cpu pentium -drive file=hd3M.img,media=disk,index=0,format=raw\
		-drive file=hd80M.img,media=disk,index=1,format=raw\
		-no-reboot -no-shutdown -s -S\
    -netdev tap,id=net0,ifname=tap0,script=no,downscript=no\
		-device e1000,netdev=net0,mac=00:FF:26:E4:55:94\
		-vga std
#生成可以被GDB理解的符号表，用于GDB调试
gdb_symbol:
	objcopy --only-keep-debug $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/kernel.sym
	objcopy --strip-debug $(BUILD_DIR)/kernel.bin $(BUILD_DIR)/kernel.elf
run:
	sudo /home/eehf/bochs-e1000-2.8/bin/bochs 
all:mk_dir boot build gdb_symbol hd qemu
gdb:mk_dir boot build gdb_symbol hd qemu-gdb
#make all 就是依次执行mk_dir build hd gdb_symbol
