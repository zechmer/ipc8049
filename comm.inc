; =============================================================================
; COMMUNICATION SUBROUTINES
; -----------------------------------------------------------------------------
; get 4bit data from ZX8302,  ret A.lsb
; send 4bit data to ZX8302,  ent A.msb=data
; get byte from ZX8302,  ret A
; send byte to ZX8302
; send A.lsb to ZX8302
; =============================================================================


; -----------------------------------------------------------------------------
; get 4bit data from ZX8302,  ret A.lsb
; -----------------------------------------------------------------------------
GET_4BIT:   MOV  R6, #0x00     
            MOV  R7, #0x04     
            IN   A, P2       
            ANL  A, #0x80      
            JNZ  #0x0753     ; loop until comdata=L
            MOVX @R0, A      ;assert WRN
            IN   A, P2       ;??
            MOVX @R0, A      ;assert WRN
            RLC  A          
            MOV  A, R6       
            RLC  A          
            MOV  R6, A       ;shift bits into R6
            DJNZ R7, #0x0753   
            RETR            ;R6=A = 4bit cmd
; -----------------------------------------------------------------------------
; send 4bit data to ZX8302,  ent A.msb=data
; -----------------------------------------------------------------------------
SEND_4BIT:  MOV  R7, #0x04     
            MOV  R6, A       
            IN   A, P2       
            ANL  A, #0x80     ; loop until comdata=H
            JNZ  #0x0765      
            MOV  A, R6       
            MOVX @R0, A      
            JB7  #0x0772      
            ANL  P2, #0x7F     
            JMP  $774       
            ORL  P2, #0x80     
            MOVX @R0, A      
            RL   A          
            MOV  R6, A       
            ORL  P2, #0x80     
            DJNZ R7, #0x0765   
            RETR            
; -----------------------------------------------------------------------------
; get byte from ZX8302,  ret A
; -----------------------------------------------------------------------------
GET_8BIT:   CALL GET_4BIT       ;get 4bit data from ZX8302,  ret A.lsb
            SWAP A          
            MOV  R5, A       
            CALL GET_4BIT       ;get 4bit data from ZX8302,  ret A.lsb
            ORL  A, R5       
            RETR            
; -----------------------------------------------------------------------------
; send byte to ZX8302
; -----------------------------------------------------------------------------
SEND_8BIT:  CALL SEND_4BIT       ;send 4bit data to ZX8302,  ent A.msb=data
            CALL SEND_4BIT       ;send 4bit data to ZX8302,  ent A.msb=data
            RETR            
; -----------------------------------------------------------------------------
; send A.lsb to ZX8302
; -----------------------------------------------------------------------------
SEND_LSB:   SWAP A          
            CALL SEND_4BIT       ;send 4bit data to ZX8302,  ent A.msb=data
            RETR