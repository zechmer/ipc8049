; -----------------------------------------------------------------------------
; ISR timer; get here when counter++ overflows from #0xFF->#0x00
; -----------------------------------------------------------------------------
.org #0x100
.equ  SER1open  #0x10
.equ  SER2open  #0x20

TIMER:      JF0  TIMER_2      
            RETR            
TIMER_2:    SEL  RB1        
            MOV  R7, A       
            STOP TCNT       ; stop counting
            MOV  A, #0xFC      
            MOV  T, A        
            STRT CNT        ; set counter for 4 cycles H->L on T1 input (= 4 * baudrate)
            JF1  HANDLE_SER    ;  handle serial data input after timer expiry interrupt
            MOV  R0, IOSTAT   ;  IOSTAT
            MOV  A, @R0      
            ANL  A, #0x04      
            JNZ  TEI                 ; cannot do conditional "long" jump
            JMP  $04A       
TEI:        JMP  TIMER_EXP_INT       ; timer expiry interrupt F1=0,  IOSTATb4?
;
; IPCOM 2/3,  SER1, 2 open
SER_OPEN:   MOV  R0, SER2open     
            JB0  #0x0121      
            MOV  A, @R0
;
; SER1 open
SER1_OPEN:  ORL  A, SER1open      
            MOV  @R0, A             ; set SER1open in IPCSTAT
            JMP  SCAN_KEYBOARD      ; scan keyboard
            MOV  A, @R0      
;
; SER2 open
SER2_OPEN:  ORL  A, SER2open      
            MOV  @R0, A             ; set SER2open in IPCSTAT
            JMP  SCAN_KEYBOARD      ; scan keyboard
;
; IPCOM 4/5,  SER1, 2 close 
SER_CLOSE:  MOV  R0, #0x20     
            JB0  #0x0131      
            MOV  A, @R0      
;
; SER1 close
SER1_CLOSE: ANL  A, #0xEF      
            MOV  @R0, A             ; clr SER1open in IPCSTAT
            JMP  SCAN_KEYBOARD      ; scan keyboard
            MOV  A, @R0
            
;
; SER2 close
SER2_CLOSE: ANL  A, #0xDF      
            MOV  @R0, A             ; clr SER1open in IPCSTAT
            JMP  SCAN_KEYBOARD      ; scan keyboard
;
; handle serial data input after timer expiry interrupt
HANDLE_SER: IN   A, P2       
            ANL  A, #0x01           ; P2.0: RXD in
            XCH  A, R3       
            JNZ  #0x0155      
            XCH  A, R3              ; R3=0,  no bits received yet.
            JNZ  #0x014C            ;  jump if RXD = H
            MOV  A, @R0             ; received a real start bit (L)
            ANL  A, #0xF7           ;  clr b3 IOSTAT
            MOV  @R0, A      
            DEC  R0         
            MOV  @R0, #0x00         ;  0->#0x21
            INC  R0         
            MOV  R3, #0x09          ; expect 9 bits
            MOV  A, R7       
            RETR            
            STOP TCNT               ; noise detected,  restart the receive process all over
            DIS  TCNTI      
            EN   I          
            MOV  A, @R0      
            ORL  A, #0x08           ; set b3 IOSTAT
            MOV  @R0, A      
            MOV  A, R7       
            RETR            
            XCH  A, R3              ; restore R3
            DJNZ R3, #0x018A   
            STOP TCNT               ; got the whole byte now
            DIS  TCNTI      
            EN   I          
            JNZ  #0x0164            ;  ???
            MOV  A, @R0             ; code never reached
            ANL  A, R4       
            JNZ  #0x0184      
            MOV  A, R4       
            ORL  A, @R0      ; bit 6/7 never set in IOSTAT???
            MOV  @R0, A      
            MOV  A, R2       ; current buf ptr
            MOV  R0, A       
            MOV  A, R5       
            MOV  @R0, A      ; store the received byte
            MOV  A, @R1      
            DEC  A          
            MOV  @R1, A      ; decrement SERxNUM (negative length)
            INC  R0         
            MOV  A, R0       ; current buf ptr
            ADD  A, R6       ; add Buffer End to see if end of RAM reached
            MOV  A, R0       
            JNC  #0x0173    ;  jump if not yet at end
            ADD  A, #0xE9    ;  back to buffer start,  $17 bytes less in RAM
            MOV  R2, A       ; update buf ptr
            MOV  R0, #0x22   ;  restore correct bank1 R0 (#0x22)
            MOV  A, @R1      
            ADD  A, #0x14      
            JNZ  BUF_neq14    ;  jump if num of char in buffer != $14 (watermark)
            ORL  P2, #0x30   ;  P2.4/5=H CTSout, DTRout (other side may resume now)
            MOV  A, @R0      
            ORL  A, #0x02      
            MOV  @R0, A      
BUF_neq14:  INC  R0         ; #bytes in buffer not equal 14
            INC  @R0        ; #0x23++
            DEC  R0         
            MOV  A, @R0      
            ORL  A, #0x08      
            MOV  @R0, A      
            MOV  A, R7       
            RETR            
            RRC  A          ; shift this bit into R5
            MOV  A, R5       
            RRC  A          
            MOV  R5, A       
            MOV  A, R7       
            RETR            ; ***** CALL TIMER END 
