.org #0x500
; =============================================================================
; KEYBOARD SUBROUTINES
; -----------------------------------------------------------------------------
; scan keyboard,  entry A (one bit=1),  ret A
; process key scan in A,  ret Cset if key processing necessary
; =============================================================================

; -----------------------------------------------------------------------------
; scan keyboard
; -----------------------------------------------------------------------------
SCAN_KEYBOARD:  MOV  R0, #0x27     
                INC  @R0        ;KBDSEED++
                MOV  R2, #0x08     
                MOV  A, #0x80    ;  scan ctl/alt/sh keyrow
                CALL SCAN_KEYB_A       ; entry A (one bit=1),  ret A
                MOV  R6, A       
                ANL  A, #0x07      
                ADD  A, #0xC0      
                MOVP A, @A       ;get SH/CTRL/ALT code from table $5C0
                MOV  R4, A       
                MOV  R0, #0x28     
                XRL  A, @R0      
                JZ   #0x0521    ;  same again? skip it
                MOV  R1, #0x20   ;  a new key combination
                MOV  A, @R1      
                ORL  A, #0x04    ;  set key pressed
                ANL  A, #0xF7    ;  clr repeat key
                MOV  @R1, A      
                MOV  A, R4       
                MOV  @R0, A      ;store it in #0x28
                JMP  $5BE       ;goto $600
                MOV  A, R6       ;restore 8bit scanned pattern
                ANL  A, #0xF8    ;  proceed with 5 MSB bits,  as for all next rows
                MOV  R6, #0xFF     
                CALL PROC_KEY_A       ;process key scan in A,  ret Cset if key processing necessary
                JC   #0x053F      
                DEC  R2         
                ;
                ; loop through other keyrows
                ;
                MOV  A, R3       ;R3 was stored in SCAN_KEYB_A ($5c8)
                RR   A          
                CALL SCAN_KEYB_A       ; entry A (one bit=1),  ret A
                CALL PROC_KEY_A       ;process key scan in A,  ret Cset if key processing necessary
                JC   #0x053F      
                DJNZ R2, #0x052B  ; loop through other keyrows
                MOV  A, R6       
                INC  A          
                JNZ  #0x054E     ; jump if R6 != #0xFF
                MOV  R0, #0x20    ; no keys; re-init all
                MOV  A, @R0      
                ANL  A, #0xFB      ;clr (s/c/a?) key pressed
                MOV  @R0, A      
                MOV  R0, #0x20     ;found key (in previous scan,  keycode in A)
                MOV  A, @R0      
                ANL  A, #0xF7      ;clr repeat key
                MOV  @R0, A      
                MOV  R0, #0x29     
                MOV  @R0, #0xFF    
                INC  R0         
                MOV  @R0, #0x00    
                JMP  $5BE       
                MOV  R0, #0x20     
                MOV  A, @R0      
                ANL  A, #0x04      ;key pressed?
                JNZ  #0x05BE      
                MOV  R0, #0x29     
                MOV  A, @R0      
                XRL  A, R6       
                JZ   #0x0562      ;jump if #0x29 == R6 (still same key?)
                MOV  A, R6       
                MOV  @R0, A      ;else store new key in #0x29
                INC  R0         
                MOV  @R0, #0x00   ; clear #0x2A
                JMP  $5BE       
                INC  R0         
                MOV  A, @R0      ;get #0x2A
                ADD  A, #0xFC    ;  -4
                JZ   #0x056D    ;  jump if #0x2A == 4
                JC   #0x05B8    ;  jump if #0x2A > 4handle repeat key
                INC  @R0        
                JMP  $5BE       
                INC  @R0        ; #0x2A++ (5)
                MOV  R0, #0x28     
                MOV  A, @R0      
                ADD  A, #0xFD      
                JNZ  #0x058D     ; jump if sca != 03 'ctl alt'store key in buffer
                MOV  A, R6       
                ADD  A, #0xC1      
                JNZ  #0x058D     ; jump if key != 3f '7'store key in buffer
                ;
                ; do the 'ctl-alt-7' thing
                ANL  P2, #0xFB    ; P2.2=L,  IPL0-2L -> int 5.
                DIS  I          
                DIS  TCNTI      
                STOP TCNT       
                ORL  P2, #0x10     ;P2.4=H,  CTSout
                ORL  P2, #0x20     ;P2.5=H,  DTRout
                MOV  R0, #0xFF     
                MOV  R1, #0xFF     
                DJNZ R1, #0x0587   
                DJNZ R0, #0x0585   
                JMP  IPCInit
                ;
                ; store key in buffer
                MOV  R0, #0x2B     
                MOV  A, @R0      
                MOV  R1, A       
                ADD  A, #0xC6      ;-3A
                JZ   #0x05B0      ;jump if curr ptr==#0x3A (overflow)handle buffer overflow
                MOV  R0, #0x28     
                MOV  A, @R0      
                MOV  @R1, A      ;store SCA in keybuf
                INC  R1         
                MOV  A, R6       
                MOV  @R1, A      ;store KEY in keybuf
                INC  R1         
                MOV  A, R1       
                MOV  R0, #0x2B     
                MOV  @R0, A      ;update BUFPTR
                MOV  R0, #0x20     
                MOV  A, @R0      
                ORL  A, #0x01    ;  set keyboard interrupt
                MOV  @R0, A      
                MOV  R0, #0x27   ;  use key press timing to create better randomness
                MOV  A, @R0      
                MOV  R1, #0x25   ;  update RANDOM
                CALL memADD       ;memory ADD: ((R1), (R1+1))+=A,  keep R1 (16bit little endian)
                JMP  $5BE       
                ;
                ; handle buffer overflow
                MOV  R0, #0x38     
                MOV  A, @R0      
                ORL  A, #0x08     ; set OVF bit in #0x38 (with 7th key)
                MOV  @R0, A      
                JMP  $5BE       
                ;
                ; handle repeat key
                MOV  R0, #0x20     
                MOV  A, @R0      
                ORL  A, #0x08      ;set repeat key
                MOV  @R0, A      
                JMP  $600       
                ;
                ; SH/CTRL/ALT conversion; scancode to SCAcode
                .DB  #0x00        ;none
                .DB  #0x04        ;SH
                .DB  #0x02        ;CTRL
                .DB  #0x06        ;SH/CTRL
                .DB  #0x01        ;ALT
                .DB  #0x05        ;SH/ALT
                .DB  #0x03        ;CTRL/ALT
                .DB  #0x07        ;SH/CTRL/ALT
; -----------------------------------------------------------------------------
; scan keyboard,  entry A (one bit=1),  ret A
; -----------------------------------------------------------------------------
SCAN_KEYB_A:    OUTL P1, A       ;output A on P1
                MOV  R3, A       
                MOVX A, @R0      ;put P0 in read mode=HI-Z ??
                NOP             ;wait a bit
                NOP             
                NOP             
                NOP             
                NOP             
                NOP             
                IN   A, P0       ;get P0 into A
                RETR
                
; -----------------------------------------------------------------------------
; process key scan in A,  ret Cset if key processing necessary
; -----------------------------------------------------------------------------
PROC_KEY_A:     JZ   #0x05F9     ; jump if no key pressed
                XCH  A, R6       
                INC  A          
                JZ   #0x05DC     ; jump if R6 == #0xFF
                CLR  C          ;else process previous and new scan data
                CPL  C          ;return with Cset
                RET             
                MOV  R4, #0x08     
                MOV  A, R6       ;restore scan
                JB7  #0x05E8    ;  key pressed?
                RL   A          
                DJNZ R4, #0x05DF   
                MOV  R6, #0xFF    ; reinit R6,  ret Cset. ??code cannot be not reached
                CLR  C          
                RET
                
                ANL  A, #0x7F      
                JNZ  #0x05D9     ; jump if more keys pressed
                MOV  A, R2       ;R2: current row
                DEC  A          
                CPL  A          
                ANL  A, #0x07      
                RL   A          
                RL   A          
                RL   A          
                MOV  R6, A       
                MOV  A, R4       ;R4: current column
                DEC  A          
                ORL  A, R6       ;A=(row-1)*8+(column-1)
                MOV  R6, A       ;R6 current pressed key
                CLR  C          
                RET             

.org #0x600
            JF1  #0x0611      
            MOV  R0, #0x22     
            MOV  A, @R0      
            ANL  A, #0x20      
            JZ   #0x060F      
            MOV  R0, #0x20     
            MOV  A, @R0      
            ANL  A, #0xFD    ;  clr sound off??
            MOV  @R0, A      
            JMP  $200       ;get IPCOM cmd or else scan keyboard 
            MOV  R0, #0x22     
            MOV  A, @R0      
            ANL  A, #0x08      
            JZ   #0x064A      
            MOV  R0, #0x24    ; BAUDRATE
            MOV  R1, #0x21     
            INC  @R1        
            CALL memSUB8       ;memory SUB8: ret. A=(R1)-(R0),  keep R0/R1
            JNC  #0x064A      
            MOV  R0, #0x20     
            MOV  A, @R0      
            ANL  A, #0x80    ;  ??b7 IPCSTAT
            JZ   #0x063E      
            CLR  F0         
            MOV  A, @R0      
            ANL  A, #0x7F      
            MOV  @R0, A      
            STOP TCNT       
            DIS  I          
            DIS  TCNTI      
            CPL  F1         
            SEL  RB1        
            MOV  R0, #0x4B     
            MOV  A, @R0      
            MOV  R1, A       ;(#0x4B)->R1, bank1 (CRPTR)
            MOV  A, R2       
            MOV  @R1, A      ;R2, bank1->(R1, bank1) (current pointer)
            SEL  RB0        
            CALL $04A       
            CPL  F0         
            JMP  $200       ;get IPCOM cmd or else scan keyboard 
            ;
            ; 
            ORL  P2, #0x30    ; P2.4/5=H,  CTSout/DTRout
            MOV  R0, #0x20     
            MOV  A, @R0      ;set b7 IPCSTAT
            ORL  A, #0x80      
            MOV  @R0, A      
            MOV  @R1, #0x00   ; clr #0x21
            JMP  $200       ;get IPCOM cmd or else scan keyboard 
            ;
            ; 
            MOV  R1, #0x23     
            MOV  A, @R1      
            ADD  A, #0xE9      
            JC   #0x063E     ; jump if #0x23>=$17
            MOV  R0, #0x22     
            MOV  A, @R0      
            ANL  A, #0x02      
            JNZ  #0x063E      
            JMP  $200       ;get IPCOM cmd or else scan keyboard
