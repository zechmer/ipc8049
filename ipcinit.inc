IPCInit:    STOP TCNT       
            DIS  I          
            DIS  TCNTI      
            CLR  F0         
            CPL  F0            ; In what state is P2?
            ORL  P2, #0x01     ; p2.0=H, RXDin&TXDin
            ANL  P2, #0xFD     ; p2.1=L, speaker
            ORL  P2, #0x04     ; p2.2=H, ipl0-2
            ORL  P2, #0x08     ; p2.3=H, ipl1
            ORL  P2, #0x10     ; p2.4=H, CTSout
            ORL  P2, #0x20     ; p2.5=H, DTRout
            ORL  P2, #0x40     ; p2.6=H, ??
            ORL  P2, #0x80     ; p2.7=H, comdata
            MOV  R0, #0x20     
            MOV  R2, #0x60

CLEAR_MEM:  MOV  @R0, #0x00    
            INC  R0         
            DJNZ R2, CLEAR_MEM   ; clear memory 020..07F
; -------------------------------------
; Init variables
; -------------------------------------
            MOV  R0, KEYCODE     
            MOV  @R0, #0xFF    ; KEY current key
            MOV  R0, BUFPTR     
            MOV  @R0, #0x2C    ; keybuf ptr=2C
            MOV  R0, BAUDRATE     
            MOV  @R0, baud9600 ; baud=9600
            MOV  R0, SER1SPTR     
            MOV  @R0, #0x52    ; SER1SPTR=#0x52
            INC  R0         
            MOV  @R0, #0x52    ; SER1RPTR=#0x52
            INC  R0         
            MOV  @R0, #0x69    ; SER2SPTR=#0x69
            INC  R0         
            MOV  @R0, #0x69    ; SER2RPTR=#0x69
            CLR  F1         
            CALL CALL_04A

WAIT_IPC:   IN   A, P2       ; wait for IPC cmd
            ANL  A, #0x80      
            JZ   WAIT_IPC      
            JMP  SCAN_KEYBOARD  

CALL_04A:   STOP TCNT       
            SEL  RB1        ; bank 1 from here on (till 08F, 0DA, 413)
            MOV  R0, IPCSTAT     
            MOV  A, @R0      
            ANL  A, #0x02      ; sound on/off
            MOV  R0, #0x22     
            JZ   NO_SOUND      ; jump if no sound
            MOV  A, @R0      
            ORL  A, #0x04      ; set #0x22, b2
            ANL  A, #0xEF      ; clr #0x22, b4
            MOV  @R0, A      
            JMP  UPDATE_SPEAKER       ; update speaker with #0x22, b4 inverted

NO_SOUND:   MOV  R2, #0x00     
            MOV  R0, SER1NUM     ; SER1NUM (one-complement #bytes?)
            MOV  A, @R0      
            CPL  A          
            INC  A               ; positive length
            ANL  A, #0x3F      
            ADD  A, #0xEC      
            JNC  LESS_14      ; jump if < #0x14 chars in buffer (watermark)
            MOV  A, R2       
            ORL  A, #0x10      
            MOV  R2, A
            
LESS_14:    INC  R0         ; SER2NUM
            MOV  A, @R0      
            CPL  A          ; same for SER2..
            INC  A          
            ANL  A, #0x3F      
            ADD  A, #0xEC      
            JNC  LESS_14_2  ; Less than 14 characters in buffer?      
            MOV  A, R2       
            ORL  A, #0x20      
            MOV  R2, A
            
LESS_14_2:  MOV  A, R2       
            CPL  A          ; bit 4, 5 indicate that SER1, 2 buffer is almost full
            MOV  R0, IPCSTAT   ;  IPCSTAT
            ANL  A, @R0      ; is SER1, 2 open?
            MOV  R0, #0x22     
            JB4  INIT_SER1    ;  SER1 buf almost fullinit SER1 rcv
            JB5  INIT_SER2    ;  SER2 buf almost fullinit SER2 rcv
            STOP TCNT       
            MOV  A, #0x00    ;  init for 256 ticks
            MOV  T, A        
            DIS  TCNTI      
            EN   TCNTI      
            STRT T          ; start timer @11Mhz/32 -> 1 tick/2.9 usec -> timeout=744 usec
            MOV  A, R7       
            SEL  RB0        
            RETR            

; -------------------------------------
; Init SER2 rcv
; -------------------------------------
INIT_SER2:  MOV  A, @R0      
            ORL  A, #0x01      ; set CHANOVF
            MOV  @R0, A        ; store IOSTAT
            MOV  R2, #0x51     ; SER2RPTR
            MOV  R1, #0x4D     ; SER2NUM
            MOV  R6, #0x80     ; SER2 Buffer End
            MOV  R4, #0x80     
            JMP  INIT_OK       
; -------------------------------------
; Init SER1 rcv
; -------------------------------------
INIT_SER1:  JB5  #0x00AF      
            MOV  A, @R0      
            ANL  A, #0xFE      ; clr CHANOVF
            MOV  @R0, A        ; store IOSTAT
            MOV  R2, #0x4F     ; SER1RPTR
            MOV  R1, #0x4C     ; SER1NUM
            MOV  R6, #0x97     ; SER1 Buffer End
            MOV  R4, #0x40     
            JMP  INIT_OK       

            MOV  A, @R0      
            JB0  #0x00A1      ; ?? check CHANOVF???
            JMP  INIT_SER2        ; init SER2 rcv

INIT_OK:    MOV  A, R4       
            CPL  A          
            ANL  A, @R0      
            ANL  A, #0xFD      
            ORL  A, #0x08      
            MOV  @R0, A      ; IOSTAT
            DEC  R0         
            MOV  @R0, #0x00    ; 0->#0x21
            INC  R0         
            INC  R0         
            MOV  @R0, #0x00    ; 0->#0x23
            MOV  R0, #0x4B     
            MOV  A, R2       
            MOV  @R0, A      ; store SERxRPTR in CRPTR
            MOV  R0, A       
            MOV  A, @R0      
            MOV  R2, A       ; R2 is value of current RPTR
            MOV  R0, #0x22     
            CPL  F1         ; set F1 flag (always 0 when 0A4 is called)
            MOV  R3, #0x00     
            EN   I          
            DIS  TCNTI      
            MOV  A, R4       
            JB7  #0x00D9      ; jump if SER2
            MOV  A, R7       
            SEL  RB0        
            ANL  P2, #0xEF     ; P2.4=L,  CTSout (other side should stop transmitting now...)
            RETR            
            MOV  A, R7       
            SEL  RB0        
            ANL  P2, #0xDF     ; P2.5=L,  DTRout (other side should stop transmitting now...)
            RETR