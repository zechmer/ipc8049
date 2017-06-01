; -----------------------------------------------------------------------------
; IPCOM
; -----------------------------------------------------------------------------
.org #0x200
;
; get IPCOM cmd or else scan keyboard 
            IN   A, P2       
            ANL  A, #0x80      
            JZ   COMDATA     ; comdata=L?
            JMP  SCAN_KEYBOARD       ; scan keyboard
COMDATA:    CALL GET_4BIT    ; get 4bit data from ZX8302,  ret A.lsb
            ADD  A, #0x0C      
            JMPP @A         ;jump according IPCOM table $20C
;
; IPCOM jump table
            .DB  #0x1C       ; 0: init IPC
            .DB  #0x28       ; 1: get interrupt status
            .DB  #0x1E       ; 2: open ser1
            .DB  #0x1E       ; 3: open ser2
            .DB  #0x20       ; 4: close ser1
            .DB  #0x20       ; 5: close ser2
            .DB  #0x89       ; 6: serial1 receive
            .DB  #0x89       ; 7: serial2 receive
            .DB  #0x4D       ; 8: read keyboard
            .DB  #0x76       ; 9: keyrow
            .DB  #0x22       ; a: set sound
            .DB  #0x24       ; b: kill sound
            .DB  #0xC3       ; c: set P2.3
            .DB  #0xCF       ; d: set serial baudrate
            .DB  #0xE3       ; e: get random 
            .DB  #0xF0       ; f: test
            
            
            JMP  IPCInit
            JMP  SER_OPEN      ; IPCOM 2/3,  SER1, 2 open
            JMP  SER_CLOSE     ; IPCOM 4/5,  SER1, 2 close 
            JMP  SET_SOUND     ; IPCOM A,  set sound
            JMP  KILL_SOUND    ; IPCOM B,  kill sound
            JMP  SCAN_KEYBOARD ; scan keyboard
; -------------------------------------
; IPCOM 1,  get IPC status (from 50Hz poll)
; -------------------------------------
            IN   A, P2       
            ANL  A, #0x40      ; P2.6,  not connected; MDV writeprot never asserted
            MOV  R0, IPCSTAT     
            ORL  A, @R0      
            ANL  A, #0xCF      ; clr b5, 4: ser1/2 open
            JB3  REPEAT        ; Bit 3 set: repeat key (key pressed)
            JMP  NOREPEAT      ; no
REPEAT:     ORL  A, #0x01      ; yes, set keyboard interrupt
NOREPEAT:   MOV  R2, A       
            MOV  R0, SER1NUM   ; SER1NUM
            MOV  A, @R0      
            JZ   NUM_SER2      
            MOV  A, R2       
            ORL  A, #0x10      ; SER1 data available
            MOV  R2, A       
NUM_SER2:   INC  R0            ; SER2NUM
            MOV  A, @R0      
            JZ   #0x0248      
            MOV  A, R2       
            ORL  A, #0x20      ; SER2 data available
            MOV  R2, A       
            MOV  A, R2       
            CALL SEND_8BIT     ; send byte to ZX8302
            JMP  SCAN_KEYBOARD ; scan keyboard
; -------------------------------------
; IPCOM 8,  getkey
; -------------------------------------
GETKEY:     MOV  R0, BUFPTR     ; current free pos
            MOV  A, @R0      
            ADD  A, #0xD4       ; -#0x2C (keybuf start)
            CLR  C          
            RRC  A          
            MOV  R2, A       
            MOV  R1, IPCSTAT     
            MOV  A, @R1      
            ANL  A, #0x08      
            ORL  A, R2           ; bit 3=repeatkey, b2-0=nr of keys
            CALL SEND_LSB        ; send A.lsb to ZX8302
            MOV  A, R2       
            JZ   NOKEY      
            MOV  R1, KEYBUF_STRT ;   keybuf ptr
SCAN:       MOV  A, @R1          ; b3 ovf,  b2 SHFT,  b1 CNTL,  b0 ALT
            CALL SEND_LSB        ; send A.lsb to ZX8302
            INC  R1         
            MOV  A, @R1          ; key
            CALL SEND_8BIT       ; send byte to ZX8302
            INC  R1         
            DJNZ R2, SCAN   
            MOV  @R0, KEYBUF_STRT;  reset ptr
NOKEY:      MOV  R0, IPCSTAT     
            MOV  A, @R0      
            ANL  A, #0xFE      
            MOV  @R0, A          ; clear keyb. interrupt status
            JMP  SCAN_KEYBOARD  
; -------------------------------------
; IPCOM 9,  keyrow
; -------------------------------------
            CALL GET_4BIT       ; get 4bit data from ZX8302,  ret A.lsb
            ANL  A, #0x07      
            INC  A          
            MOV  R2, A       
            MOV  A, #0x00       ; (CLR A takes 1 byte more!)    
            CLR  C          
            CPL  C          
NEXT_ROW:   RLC  A              ; A=1<<row
            DJNZ R2, NEXT_ROW   
            CALL SCAN_KEYB_A    ; scan keyboard,  entry A (one bit=1),  ret A
            CALL SEND_8BIT      ; send byte to ZX8302
            JMP  SCAN_KEYBOARD  
; -------------------------------------
; IPCOM 6/7,  SER1, 2 rcv
; -------------------------------------
            JB0  SER2      
            MOV  R0, SER1NUM
            MOV  R4, #0x97     ; SER1 Buffer End
            MOV  R5, SER1SPTR
            JMP  $299       
SER2:       MOV  R0, SER2NUM
            MOV  R4, #0x80   ; SER2 Buffer End
            MOV  R5, SER2SPTR
            MOV  A, @R0      ; get SERxNUM
            CPL  A          
            INC  A           ; A=-SERxNUM
            ANL  A, #0x3F      
            MOV  R3, A       
            MOV  R1, IOSTAT     
            MOV  A, @R1      
            ANL  A, #0xC0    ; select SER12 stopbits from IOSTAT
            ORL  A, R3       
            CALL SEND_8BIT   ; send byte to ZX8302
            MOV  A, R3       
            JZ   #0x02C1     ; stop if no bytes available
            MOV  A, R5       ; get SERxSPTR
            MOV  R1, A       
            MOV  A, @R1      
            MOV  R1, A       ; R1 points to current byte to be sent
SEND:       MOV  A, @R1      
            CALL SEND_8BIT   ; send byte to ZX8302
            INC  @R0         ; increment buffer length (=negative)
            INC  R1         
            MOV  A, R1       
            ADD  A, R4       ; add SERx Buffer Eend
            JNC  NEXT_BYTE   ; jump if not at end of buffer
            MOV  A, R1       
            ADD  A, #0xE9      
            MOV  R1, A       ; ptr=ptr-$17 (SERrcv buffer length=$17)
NEXT_BYTE:  DJNZ R3, SEND ;  jump if not all bytes were sent yet
            MOV  A, R5       
            MOV  R0, A       
            MOV  A, R1       
            MOV  @R0, A      ; SERxSPTR now points to next byte in buffer
            JMP  SCAN_KEYBOARD       ; scan keyboard
; -------------------------------------
; IPCOM C,  set IPL1L as in A.b0
; -------------------------------------
            CALL GET_4BIT       ; get 4bit data from ZX8302,  ret A.lsb
            JB0  HIGH4      
            ANL  P2, #0xF7   ;  P2.3=L,  IPL1L
            JMP  SCAN_KEYBOARD       ;
HIGH4:      ORL  P2, #0x08   ;  P2.3=H,  IPL1L
            JMP  SCAN_KEYBOARD       ;
; -------------------------------------
; IPCOM D,  baudrate
; -------------------------------------
            CALL GET_4BIT       ; get 4bit data from ZX8302,  ret A.lsb
            ANL  A, #0x07      
            ADD  A, #0xDB      
            MOVP A, @A       
            MOV  R0, BAUDRATE     
            MOV  @R0, A      
            JMP  SCAN_KEYBOARD

.equ baud19200  #0x01
.equ baud9600  #0x03

;
; BAUD table
.DB  baud19200      ;   19200
.DB  baud9600       ;    9600
.DB  #0x07      ;    4800
.DB  #0x0F      ;    2400
.DB  #0x1F      ;    1200
.DB  #0x3F      ;     600
.DB  #0x7F      ;     300
.DB  #0xFF      ;      75
; -------------------------------------
; IPCOM E,  random
; -------------------------------------
            CALL GEN_RANDOM      ; generate RANDOM nr in #0x25, #0x26
            MOV  R1, RANDOM_H     
            MOV  A, @R1      
            CALL SEND_8BIT       ; send byte to ZX8302
            INC  R1              ; RANDOM_L   
            MOV  A, @R1      
            CALL SEND_8BIT       ; send byte to ZX8302
            JMP  SCAN_KEYBOARD     
            ;
            ; IPCOM F,  test
            CALL GET_8BIT        ; get byte from ZX8302,  ret A
            CALL SEND_8BIT       ; send byte to ZX8302
            JMP  SCAN_KEYBOARD     

.org #0x300
; -------------------------------------
; IPCOM A,  set sound
; -------------------------------------
SET_SOUND:  MOV  R0, SOUND_PITCH1 ; first sound parameter     
            MOV  R1, #0x08       ; 8 parameters 
NEXT_PARAM: CALL GET_8BIT        ; get byte from ZX8302,  ret A
            MOV  @R0, A      
            INC  R0         
            DJNZ R1, NEXT_PARAM  ; store all 8 params at #0x3A and up
            CALL CHOOSE_PITCH    ; choose pitch,  ret R0=42
            INC  R0              ; prepare #0x42..#0x49 sound variables
            MOV  @R0, A      
            INC  R0         
            MOV  @R0, A      
            MOV  R1, #0x05     
NEXT_PORT:  INC  R0         
            MOV  @R0, #0x00    
            DJNZ R1, NEXT_PORT   
            MOV  R0, IPCSTAT     
            MOV  A, @R0      
            ORL  A, #0x02        ; set sound on
            MOV  @R0, A      
            JMP  SCAN_KEYBOARD   
; -------------------------------------
; IPCOM B,  kill sound
; -------------------------------------
KILL_SOUND: MOV  R0, IPCSTAT     
            MOV  A, @R0      
            ANL  A, #0xFD        ; sound off
            MOV  @R0, A      
            JMP  SCAN_KEYBOARD       
;
; timer expiry interrupt F1=0,  IOSTATb4?
TIMER_EXP_INT:
            STOP TCNT       
            MOV  R0, IPCSTAT     
            MOV  A, @R0      
            ANL  A, #0x02      
            JNZ  DURATION     ; jump if sound on
            MOV  R0, IOSTAT     
            MOV  A, @R0      
            ANL  A, #0xFB     ; clr IOSTAT.b2
            ANL  A, #0xDF     ; clr IOSTAT.b5
            MOV  @R0, A      
            ANL  P2, #0xFD    ; P2.1=L,  speaker
            JMP  $04A       
DURATION:   MOV  R0, SOUND_DURATION    ; beep duration
            MOV  A, @R0      
            JNZ  #0x0344      
            INC  R0         
            MOV  A, @R0      
            JZ   #0x0365      
            MOV  R0, #0x4A     
            INC  @R0        
            INC  @R0        
            MOV  R1, #0x47     
            MOV  R0, #0x44     
            MOV  A, @R0      
            JNZ  #0x0351      
            MOV  A, #0xFF      
            CALL memADD       ; memory ADD: ((R1), (R1+1))+=A,  keep R1 (16bit little endian)
            MOV  R0, #0x4A     
            MOV  A, @R0      
            CALL memADD       ; memory ADD: ((R1), (R1+1))+=A,  keep R1 (16bit little endian)
            MOV  R0, #0x3F     
            INC  R1         
            CALL memSUB16       ; memory SUB16: ret. A=((R1+1), (R1))-((R0+1), (R0)),  keep R0/R1
            JNC  #0x0365      
            MOV  R0, #0x22     
            MOV  A, @R0      
            ORL  A, #0x20      
            MOV  @R0, A      
            MOV  R1, #0x45     
            MOV  R0, #0x44     
            MOV  A, @R0      
            JNZ  #0x036E      
            MOV  A, #0xFF      
            CALL memADD       ; memory ADD: ((R1), (R1+1))+=A,  keep R1 (16bit little endian)
            MOV  R0, #0x4A     
            MOV  A, @R0      
            CALL memADD       ; memory ADD: ((R1), (R1+1))+=A,  keep R1 (16bit little endian)
            MOV  R0, #0x3D     
            INC  R1         
            CALL memSUB16       ;memory SUB16: ret. A=((R1+1), (R1))-((R0+1), (R0)),  keep R0/R1
            JNC  #0x03E4    ;  update speaker with #0x22, b4 inverted
            MOV  R0, #0x4A  ; last sound parameter   
            MOV  A, @R0      
            ADD  A, #0x03      
            MOV  @R0, A      
            MOV  R0, #0x45     
            MOV  @R0, #0x00    
            INC  R0         
            MOV  @R0, #0x00    
            MOV  R0, #0x42     
            MOV  R1, #0x40     
            MOV  A, @R1      
            SWAP A          
            ORL  A, #0xF0      
            JB3  #0x0395      
            ANL  A, #0x0F      
            ADD  A, @R0      
            MOV  @R0, A      
            MOV  R1, #0x3A     
            CALL memSUB8       ;memory SUB8: ret. A=(R1)-(R0),  keep R0/R1
            JC   #0x03A5      
            MOV  R1, #0x3B     
            CALL memSUB8       ;memory SUB8: ret. A=(R1)-(R0),  keep R0/R1
            JZ   #0x03A5      
            JC   #0x03D3      
            MOV  R0, #0x4A     
            INC  @R0        
            INC  @R0        
            MOV  R0, #0x40     
            MOV  A, @R0      
            MOV  R0, #0x49     
            ORL  A, #0xF0      
            INC  A          
            JZ   #0x03BD      
            DEC  A          
            ANL  A, #0x0F      
            JZ   #0x03C2      
            CPL  A          
            INC  A          
            ADD  A, @R0      
            JC   #0x03C2      
            INC  @R0        
            CALL CHOOSE_PITCH       ;choose pitch,  ret R0=42
            JMP  $3D3       
            MOV  @R0, #0x00    
            MOV  R1, #0x40     
            MOV  A, @R1      
            SWAP A          
            CPL  A          
            INC  A          
            ANL  A, #0x0F      
            SWAP A          
            MOV  R2, A       
            MOV  A, @R1      
            ANL  A, #0x0F      
            ORL  A, R2       
            MOV  @R1, A      
            MOV  R1, #0x41     
            MOV  A, @R1      
            SWAP A          
            MOV  @R1, A      
            CALL UPDATE_SOUND       ;update sound random/fuzz
            MOV  R1, #0x42     
            ADD  A, @R1      
            INC  R1         
            MOV  @R1, A      
            MOV  R1, #0x41     
            MOV  A, @R1      
            SWAP A          
            MOV  @R1, A      
; -----------------------------------------------------------------------------
; update speaker with #0x22, b4 inverted
; -----------------------------------------------------------------------------
UPDATE_SPEAKER: MOV  R0, #0x22     
                MOV  A, @R0      
                ANL  A, #0x10      
                JZ   #0x03F3      
                ANL  P2, #0xFD     ;P2.1=L,  speaker
                MOV  A, @R0      
                ANL  A, #0xEF      
                MOV  @R0, A      
                JMP  UPDATE_SND       
                ORL  P2, #0x02     ;P2.1=H,  speaker
                MOV  A, @R0      
                ORL  A, #0x10      
                MOV  @R0, A      
                JMP  UPDATE_SND       

.org #0x400
UPDATE_SND:     CALL UPDATE_SOUND       ; update sound random/fuzz
                MOV  R1, #0x43          ; id
                ADD  A, @R1      
                INC  R1         
                MOV  @R1, A      
                CPL  A          
                INC  A          
                STOP TCNT       
                MOV  T, A        
                DIS  TCNTI      
                EN   TCNTI      
                STRT T          ;start timer @11Mhz/32 -> 1 tick/2.9 usec -> timeout=A*2.9 usec
                MOV  R0, #0x4A     
                MOV  @R0, #0x04    
                MOV  A, R7       
                SEL  RB0        
                RETR            
;
; choose pitch,  ret R0=42
CHOOSE_PITCH:   MOV  R0, SOUND_WRAP     
                MOV  A, @R0      
                MOV  R1, SOUND_PITCH2    ; pitch2
                JB7  #0x041D     ; pitch, b3?
                DEC  R1         
                MOV  R0, #0x42     
                MOV  A, @R1      
                MOV  @R0, A      ;chosen pitchx in #0x42
                RET             
;
; update sound random/fuzz
UPDATE_SOUND:   MOV  R0, SOUND_FUZZ     
                MOV  A, @R0      
                JB3  #0x0429      
                CLR  A          
                RET             
                MOV  R0, #0x4A     
                MOV  A, @R0      
                ADD  A, #0x07      
                MOV  @R0, A      
                CALL GEN_RANDOM       ; generate RANDOM nr in #0x25, #0x26
                MOV  R0, SOUND_FUZZ     
                MOV  R1, #0x25     
                MOV  A, @R0      
                ANL  A, #0x07      
                INC  A                ; #26
                MOV  R2, A       
                CLR  A          
                CLR  C          
CHG_AC:         CPL  C          
                RLC  A          
                DJNZ R2, CHG_AC   
                ANL  A, @R1      
                RET
