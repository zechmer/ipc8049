.equ IPCSTAT #0x20	
	; bit 0: keyboard interrupt
	; bit 1: sound on/off
	; bit 2: a (s/c/a?) key pressed
	; bit 3: repeat key?
	; bit 4: SER1 open/close
	; bit 5: SER2 open/close
	; (bit 6): for P2.6 in IPCOM.1 sa $229
	; bit 7: ? $624,642

.equ IOSTAT   #0x22	
	; bit 0: CHANOVF: 0=SER1, 1=SER2 almost overflown
	; bit 1: see $0B7,$17E,654
	; bit 2:
	; bit 3: see $0B9,$185,$104,614
	; bit 4: last speaker state H/L, LSP update in $3EB ff.
	; bit 4,5: buf SER1,2 almost overflown
	; bit 6: SER1 stopbit, $15E,161
	; bit 7: SER2 stopbit


.equ BAUDRATE  #0x24	; 1->19200baud, FF->75baud, init with #03 (=9600)

.equ RANDOM_H  #0x25
.equ RANDOM_L  #0x26 	; current random value
.equ KBDSEED   #0x27	; kbd seed for random? sa 500,5A7

.equ KEYCODE   #0x29	; KEY: keycode of current key; re-init as #FF, $547

.equ BUFPTR    #0x2B	; ptr to current free key pos

.equ KEYBUF_STRT  #0x2C ; buffer of 7 keys, 2 bytes each: ovf/sh/ctl/alt(SCA); keycode

; Sound parameter
.equ SOUND_PITCH1 #0x3A	
.equ SOUND_PITCH2 #0x3B	
.equ SOUND_INTERVAL #0x3C           ; #0x3D	step interval
.equ SOUND_DURATION #0x3E           ; #0x3F	duration
.equ SOUND_WRAP   #0x40	
.equ SOUND_FUZZ   #0x41	

.equ CRPTR    #0x4B	; current SERxPTR $0C6, $632
.equ SER1NUM  #0x4C	; negative number of bytesdata #bytes? $05F,$237
.equ SER2NUM  #0x4D	

.equ SER1SPTR #0x4E	; send to ZX8032 pointer, init as #52
.equ SER1RPTR #0x4F	; receive from serial line pointer, init as #52
.equ SER2SPTR #0x50	; send to ZX8032 pointer, init as #69
.equ SER2RPTR #0x51	; receive from serial line pointer, init as #69

.equ SER1BUF  #0x52	; Begin SER1 receive buffer. $17 (=23) bytes ..68
.equ SER2BUF  #0x69	; SER2 receive buffer ..7F