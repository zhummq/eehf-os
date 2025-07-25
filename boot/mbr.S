# 1 "boot/mbr.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "boot/mbr.S"
                                    ;主引导程序
                                    ;------------------------------------------------------------
%include "boot.inc"
SECTION MBR vstart=0x7c00
    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov fs,ax
    mov sp,0x7c00
    mov ax,0xb800
    mov gs,ax

                                    ; 清屏
                                    ;利用0x06号功能，上卷全部行，则可清屏。
                                    ; -----------------------------------------------------------
                                    ;INT 0x10 功能号:0x06 功能描述:上卷窗口
                                    ;------------------------------------------------------
                                    ;输入：
                                    ;AH 功能号= 0x06
                                    ;AL = 上卷的行数(如果为0,表示全部)
                                    ;BH = 上卷行属性
                                    ;(CL,CH) = 窗口左上角的(X,Y)位置
                                    ;(DL,DH) = 窗口右下角的(X,Y)位置
                                    ;无返回值：
    mov ax, 0600h
    mov bx, 0700h
    mov cx, 0 ; 左上角: (0, 0)
    mov dx, 184fh ; 右下角: (80,25),
                        ; 因为VGA文本模式中，一行只能容纳80个字符,共25行。
                        ; 下标从0开始，所以0x18=24,0x4f=79
    int 10h ; int 10h

                                    ; 输出字符串:MBR
    mov byte [gs:0x00],'1'
    mov byte [gs:0x01],0xA4

    mov byte [gs:0x02],' '
    mov byte [gs:0x03],0xA4

    mov byte [gs:0x04],'M'
    mov byte [gs:0x05],0xA4 ;A表示绿色背景闪烁，4表示前景色为红色

    mov byte [gs:0x06],'B'
    mov byte [gs:0x07],0xA4

    mov byte [gs:0x08],'R'
    mov byte [gs:0x09],0xA4
    mov esi,LOADER_START_SECTOR   ; 靠靠lba靠
    mov di,LOADER_BASE_ADDR            ; 靠靠�
    mov cl,4  

    call rd_disk_m_16 ; 以下读取程序的起始部分（一个扇区）

    jmp LOADER_BASE_ADDR + 0x300
rd_disk_m_16:
    ; 1: 靠disk status
    mov dx,0x1f7     ; 0x1f7=primary channel's status
.not_ready1:
    nop              ; 靠靠靠靠
    in al,dx
    and al,0xc0      ; 0xc0=1100_0000b縝it 6~7
    cmp al,0x40      ; 靠bit 6, 靠靠靠
    jnz .not_ready1  ;靠靠靠靠�
    ; 2: 靠靠靠靠�
    mov dx,0x1f2         ; 0x1f2=primary channel's sector count, 8 靠靠靠靠� 255 靠靠� 0靠靠靠� 256 靠�, 縛靠靠靠靠靠靠`
    mov al,cl
    out dx,al            ;靠靠靠
    ; 3: 縇BA靠靠0x1f3 ~ 0x1f6
    mov eax,esi
    ;LBA靠7~0靠靠�0x1f3
    mov dx,0x1f3      ;   0x1f3=primary channel's lba low
    out dx,al

    ;LBA靠15~8靠靠�0x1f4
    shr eax,8         ;   eax靠�8�
    mov dx,0x1f4      ;   0x1f4=primary channel's lba mid
    out dx,al

    ;LBA靠23~16靠靠�0x1f5
    shr eax,8
    mov dx,0x1f5      ;   0x1f5=primary channel's lba high
    out dx,al

    ; 4: 靠device靠
    shr eax,8
    and al,0x0f      ; lba�24~27�, 靠bit靠0
    or al,0xe0       ; 靠7�4靠1110,靠lba靠, 靠靠�
    mov dx,0x1f6     ; 0x1f6=primary channel's device
    out dx,al

    ; 5靠0x1f7靠靠靠靠0x20
    mov dx,0x1f7     ; 0x1f7=primary channel's status
    mov al,0x20      ; 0x20, 靠靠
    out dx,al

    mov bl,cl

.next_sector:
    ; 6: 靠disk status
.not_ready2:
    mov dx,0x1f7
    in al,dx         ; 靠status 靠靠靠 0x1f7 靠, 靠靠靠縟x 靠靠
    and al,0x88      ;�4靠1靠靠靠靠靠靠靠靠�7靠1靠靠�
    cmp al,0x08
    jnz .not_ready2       ;靠靠靠靠�

    ; 7靠0x1f0靠靠�. data 靠靠 16 靠靠� in 靠靠� 2 靠
    mov cx, 256       ; cx靠靠�. 靠靠�512靠靠靠�2靠靠�512/2�=256
    mov dx, 0x1f0    ; 0x1f0=primary channel's data
.go_on_read:
    in ax,dx
    mov [di],ax      ; di靠靠DISK_BUFFER
    add di,2
    loop .go_on_read ; loop縞x-=1, 靠縞x靠�0靠靠靠靠靠�
    dec bl
    cmp bl,0
    jnz .next_sector
    ret

    times 510-($-$$) db 0
    db 0x55,0xaa
