# 1 "boot/mbr.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "boot/mbr.S"
                                    ;Ö÷Òýµ¼³ÌÐò
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

                                    ; ÇåÆÁ
                                    ;ÀûÓÃ0x06ºÅ¹¦ÄÜ£¬ÉÏ¾íÈ«²¿ÐÐ£¬Ôò¿ÉÇåÆÁ¡£
                                    ; -----------------------------------------------------------
                                    ;INT 0x10 ¹¦ÄÜºÅ:0x06 ¹¦ÄÜÃèÊö:ÉÏ¾í´°¿Ú
                                    ;------------------------------------------------------
                                    ;ÊäÈë£º
                                    ;AH ¹¦ÄÜºÅ= 0x06
                                    ;AL = ÉÏ¾íµÄÐÐÊý(Èç¹ûÎª0,±íÊ¾È«²¿)
                                    ;BH = ÉÏ¾íÐÐÊôÐÔ
                                    ;(CL,CH) = ´°¿Ú×óÉÏ½ÇµÄ(X,Y)Î»ÖÃ
                                    ;(DL,DH) = ´°¿ÚÓÒÏÂ½ÇµÄ(X,Y)Î»ÖÃ
                                    ;ÎÞ·µ»ØÖµ£º
    mov ax, 0600h
    mov bx, 0700h
    mov cx, 0 ; ×óÉÏ½Ç: (0, 0)
    mov dx, 184fh ; ÓÒÏÂ½Ç: (80,25),
                        ; ÒòÎªVGAÎÄ±¾Ä£Ê½ÖÐ£¬Ò»ÐÐÖ»ÄÜÈÝÄÉ80¸ö×Ö·û,¹²25ÐÐ¡£
                        ; ÏÂ±ê´Ó0¿ªÊ¼£¬ËùÒÔ0x18=24,0x4f=79
    int 10h ; int 10h

                                    ; Êä³ö×Ö·û´®:MBR
    mov byte [gs:0x00],'1'
    mov byte [gs:0x01],0xA4

    mov byte [gs:0x02],' '
    mov byte [gs:0x03],0xA4

    mov byte [gs:0x04],'M'
    mov byte [gs:0x05],0xA4 ;A±íÊ¾ÂÌÉ«±³¾°ÉÁË¸£¬4±íÊ¾Ç°¾°É«ÎªºìÉ«

    mov byte [gs:0x06],'B'
    mov byte [gs:0x07],0xA4

    mov byte [gs:0x08],'R'
    mov byte [gs:0x09],0xA4
    mov esi,LOADER_START_SECTOR   ; ¿¿¿¿lba¿¿
    mov di,LOADER_BASE_ADDR            ; ¿¿¿¿¿
    mov cl,4  

    call rd_disk_m_16 ; ÒÔÏÂ¶ÁÈ¡³ÌÐòµÄÆðÊ¼²¿·Ö£¨Ò»¸öÉÈÇø£©

    jmp LOADER_BASE_ADDR + 0x300
rd_disk_m_16:
    ; 1: ¿¿disk status
    mov dx,0x1f7     ; 0x1f7=primary channel's status
.not_ready1:
    nop              ; ¿¿¿¿¿¿¿¿
    in al,dx
    and al,0xc0      ; 0xc0=1100_0000b¿bit 6~7
    cmp al,0x40      ; ¿¿bit 6, ¿¿¿¿¿¿
    jnz .not_ready1  ;¿¿¿¿¿¿¿¿¿
    ; 2: ¿¿¿¿¿¿¿¿¿
    mov dx,0x1f2         ; 0x1f2=primary channel's sector count, 8 ¿¿¿¿¿¿¿¿¿ 255 ¿¿¿¿¿ 0¿¿¿¿¿¿¿ 256 ¿¿¿, ¿`¿¿¿¿¿¿¿¿¿¿¿¿`
    mov al,cl
    out dx,al            ;¿¿¿¿¿¿
    ; 3: ¿LBA¿¿¿¿0x1f3 ~ 0x1f6
    mov eax,esi
    ;LBA¿¿7~0¿¿¿¿¿0x1f3
    mov dx,0x1f3      ;   0x1f3=primary channel's lba low
    out dx,al

    ;LBA¿¿15~8¿¿¿¿¿0x1f4
    shr eax,8         ;   eax¿¿¿8¿
    mov dx,0x1f4      ;   0x1f4=primary channel's lba mid
    out dx,al

    ;LBA¿¿23~16¿¿¿¿¿0x1f5
    shr eax,8
    mov dx,0x1f5      ;   0x1f5=primary channel's lba high
    out dx,al

    ; 4: ¿¿device¿¿
    shr eax,8
    and al,0x0f      ; lba¿24~27¿, ¿¿bit¿¿0
    or al,0xe0       ; ¿¿7¿4¿¿1110,¿¿lba¿¿, ¿¿¿¿¿
    mov dx,0x1f6     ; 0x1f6=primary channel's device
    out dx,al

    ; 5¿¿0x1f7¿¿¿¿¿¿¿¿0x20
    mov dx,0x1f7     ; 0x1f7=primary channel's status
    mov al,0x20      ; 0x20, ¿¿¿¿
    out dx,al

    mov bl,cl

.next_sector:
    ; 6: ¿¿disk status
.not_ready2:
    mov dx,0x1f7
    in al,dx         ; ¿¿status ¿¿¿¿¿¿ 0x1f7 ¿¿, ¿¿¿¿¿¿¿dx ¿¿¿¿
    and al,0x88      ;¿4¿¿1¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿7¿¿1¿¿¿¿¿
    cmp al,0x08
    jnz .not_ready2       ;¿¿¿¿¿¿¿¿¿

    ; 7¿¿0x1f0¿¿¿¿¿. data ¿¿¿¿ 16 ¿¿¿¿¿ in ¿¿¿¿¿ 2 ¿¿
    mov cx, 256       ; cx¿¿¿¿¿. ¿¿¿¿¿512¿¿¿¿¿¿¿2¿¿¿¿¿512/2¿=256
    mov dx, 0x1f0    ; 0x1f0=primary channel's data
.go_on_read:
    in ax,dx
    mov [di],ax      ; di¿¿¿¿DISK_BUFFER
    add di,2
    loop .go_on_read ; loop¿cx-=1, ¿¿¿cx¿¿¿0¿¿¿¿¿¿¿¿¿¿¿
    dec bl
    cmp bl,0
    jnz .next_sector
    ret

    times 510-($-$$) db 0
    db 0x55,0xaa
