;********************************************************************************************************
;                                               uC/OS-II
;                                         The Real-Time Kernel
;
;                         (c) Copyright 2002, Jean J. Labrosse, Weston, FL
;                                          All Rights Reserved
;
;
;                                       PAGED S12X Specific code
;                                            (CODEWARRIOR)
;
; File         : OS_CPU_A.S
; By           : Eric Shufro
;
; Notes        : THIS FILE *MUST* BE LINKED INTO NON_BANKED MEMORY!
;********************************************************************************************************

NON_BANKED:       section  

;********************************************************************************************************
;                                           I/O PORT ADDRESSES
;********************************************************************************************************

PPAGE:            equ    $0015         ; Addres of PPAGE register (assuming MC9S12XEP100 part)
RPAGE:            equ    $0016         ; Addres of RPAGE register (assuming MC9S12XEP100 part)
EPAGE:            equ    $0017         ; Addres of EPAGE register (assuming MC9S12XEP100 part)
GPAGE:            equ    $0010         ; Addres of GPAGE register (assuming MC9S12XEP100 part)

;********************************************************************************************************
;                                          PUBLIC DECLARATIONS
;********************************************************************************************************
   
    xdef   OS_CPU_SR_Save
    xdef   OS_CPU_SR_Restore    
    xdef   OSStartHighRdy
    xdef   OSCtxSw
    xdef   OSIntCtxSw
    xdef   OSTickISR
    xdef   SCI0_ISR
    xdef   SCI1_ISR
    xdef   SCI2_ISR
    xdef   SCI3_ISR
    xdef   ADC0_ISR
    xdef   ADC1_ISR
    xdef   CAN0_RX_ISR
    xdef   CAN1_RX_ISR
    xdef   CAN2_RX_ISR
    xdef   CAN3_RX_ISR
    xdef   CAN4_RX_ISR
    xdef   PWM_ECT_ISR
    xdef   TIMER_CH4_ISR
    ;xdef   port_p_isr
    xdef   I2C0_ISR
    xdef   I2C1_ISR
    
;********************************************************************************************************
;                                         EXTERNAL DECLARATIONS
;********************************************************************************************************
   
    xref   OSIntExit
    xref   OSIntNesting  
    xref   OSPrioCur    
    xref   OSPrioHighRdy
    xref   OSRunning   
    xref   OSTaskSwHook 
    xref   OSTCBCur     
    xref   OSTCBHighRdy 
    xref   OSTickISR_Handler 
    xref   OSTimeTick
    xref   SCI0_ISR_Handler
    xref   SCI1_ISR_Handler
    xref   SCI2_ISR_Handler
    xref   SCI3_ISR_Handler 
    xref   ADC0_ISR_Handler
    xref   ADC1_ISR_Handler
    xref   CAN0_RX_ISR_Handler
    xref   CAN1_RX_ISR_Handler
    xref   CAN2_RX_ISR_Handler
    xref   CAN3_RX_ISR_Handler
    xref   CAN4_RX_ISR_Handler
    xref   PwmEctISR_Handler
    xref   TimerCh4ISR_Handler
    xref   TimerCh0ISR_Handler
    xref   soc_adc1_isr
    xref   xadc0_adc_isr
    ;xref   port_p_isr_handler
    xref   i2c0_isr_handler
    xref   i2c1_isr_handler
    
;********************************************************************************************************
;                                  SAVE THE CCR AND DISABLE INTERRUPTS
;                                                  &
;                                              RESTORE CCR
;
; Description : These function implements OS_CRITICAL_METHOD #3 
;
; Arguments   : The function prototypes for the two functions are:
;               1) OS_CPU_SR  OSCPUSaveSR(void)
;                             where OS_CPU_SR is the contents of the CCR register prior to disabling
;                             interrupts.
;               2) void       OSCPURestoreSR(OS_CPU_SR os_cpu_sr);
;                             'os_cpu_sr' the the value of the CCR to restore.
;
; Note(s)     : 1) It's assumed that the compiler uses the D register to pass a single 16-bit argument
;                  to and from an assembly language function.
;********************************************************************************************************

OS_CPU_SR_Save:
    tfr  ccrw, d                       ; It's assumed that the 16-bit return value is in register D
    sei                                ; Disable interrupts
    rtc                                ; Return to caller with D containing the previous CCR

OS_CPU_SR_Restore:
    tfr  d,ccrw                        ; D contains the CCR word value to restore, move to CCRW
    rtc

;********************************************************************************************************
;                               START HIGHEST PRIORITY TASK READY-TO-RUN
;
; Description : This function is called by OSStart() to start the highest priority task that was created
;               by your application before calling OSStart().
;
; Arguments   : none
;
; Note(s)     : 1) The stack frame is assumed to look as follows:
;   
;                  OSTCBHighRdy->OSTCBStkPtr +  0  -->  gPAGE
;                                            +  1       ePAGE
;                                            +  2       rPAGE
;                                            +  3       pPAGE
;                                            +  1       CCRW
;                                            +  2       B
;                                            +  3       A
;                                            +  4       X (H)
;                                            +  5       X (L)
;                                            +  6       Y (H)
;                                            +  7       Y (L)
;                                            +  8       PC(H)
;                                            +  9       PC(L)
;
;               2) OSStartHighRdy() MUST:
;                      a) Call OSTaskSwHook() then,
;                      b) Set OSRunning to TRUE,
;                      c) Switch to the highest priority task by loading the stack pointer of the
;                         highest priority task into the SP register and execute an RTI instruction.
;********************************************************************************************************

OSStartHighRdy:
    call   OSTaskSwHook                ; Invoke user defined context switch hook            

    ldab   #$01                        ; Indicate that we are multitasking
    stab   OSRunning

    ldx    OSTCBHighRdy                ; Point to TCB of highest priority task ready to run 
    lds    0,x                         ; Load SP into 68HC12
    
    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register                                

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register                                

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register                                

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register   
    
    rti                                ; Run task                                           

;********************************************************************************************************
;                                       TASK LEVEL CONTEXT SWITCH
;
; Description : This function is called when a task makes a higher priority task ready-to-run.
;
; Arguments   : none
;
; Note(s)     : 1) Upon entry, 
;                  OSTCBCur     points to the OS_TCB of the task to suspend
;                  OSTCBHighRdy points to the OS_TCB of the task to resume
;
;               2) The stack frame of the task to suspend looks as follows:
;
;                  SP            CCR
;                     +  2       B
;                     +  3       A
;                     +  4       X (H)
;                     +  5       X (L)
;                     +  6       Y (H)
;                     +  7       Y (L)
;                     +  8       PC(H)
;                     +  9       PC(L)
;
;               3) The stack frame of the task to resume looks as follows:
; 
;                  OSTCBHighRdy->OSTCBStkPtr +  0  -->  gPAGE
;                                            +  1       ePAGE
;                                            +  2       rPAGE
;                                            +  3       pPAGE
;                                            +  4       CCR
;                                            +  6       B
;                                            +  7       A
;                                            +  8       X (H)
;                                            +  9       X (L)
;                                            + 10       Y (H)
;                                            + 11       Y (L)
;                                            + 12       PC(H)
;                                            + 13       PC(L)
;********************************************************************************************************

OSCtxSw:
    ldaa   GPAGE                       ; Get current value of GPAGE register                                
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register                                
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register                                
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register                                
    psha                               ; Push PPAGE register onto current task's stack
    
    ldy    OSTCBCur                    ; OSTCBCur->OSTCBStkPtr = Stack Pointer     
    sts    0,y

    call   OSTaskSwHook                ; Call user task switch hook                       
    
    ldx    OSTCBHighRdy                ; OSTCBCur  = OSTCBHighRdy
    stx    OSTCBCur
    
    ldab   OSPrioHighRdy               ; OSPrioCur = OSPrioHighRdy                        
    stab   OSPrioCur
    
    lds    0,x                         ; Load SP into 68HC12                              
    
    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register                                
        
    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register                                

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register                                

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register 
    
    rti                                ; Run task                                         

;********************************************************************************************************
;                                    INTERRUPT LEVEL CONTEXT SWITCH
;
; Description : This function is called by OSIntExit() to perform a context switch to a task that has
;               been made ready-to-run by an ISR. The GPAGE, EPAGE, RPAGE and PPAGE CPU registers of the 
;               preempted task have already been stacked during the start of the ISR that is currently 
;               running.
;
; Arguments   : none
;********************************************************************************************************

OSIntCtxSw:
    call   OSTaskSwHook                ; Call user task switch hook                
    
    ldx    OSTCBHighRdy                ; OSTCBCur  = OSTCBHighRdy
    stx    OSTCBCur
    
    ldab   OSPrioHighRdy               ; OSPrioCur = OSPrioHighRdy                        
    stab   OSPrioCur
                                    
    lds    0,x                         ; Load the SP of the next task
                      
    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register                                
                         
    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register                                
                            
    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register                                
                               
    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register     
                                  
    rti                                ; Run task                                  

;********************************************************************************************************
;                                           SYSTEM TICK ISR
;
; Description : This function is the ISR used to notify uC/OS-II that a system tick has occurred.  You 
;               must setup the S12XE's interrupt vector table so that an OUTPUT COMPARE interrupt 
;               vectors to this function.
;
; Arguments   : none
;
; Notes       :  1) The 'tick ISR' assumes the we are using the Output Compare specified by OS_TICK_OC
;                   (see APP_CFG.H and this file) to generate a tick that occurs every OS_TICK_OC_CNTS 
;                   (see APP_CFG.H) which corresponds to the number of FRT (Free Running Timer) 
;                   counts to the next interrupt.
;
;                2) All USER interrupts should be modeled EXACTLY like this where the only
;                   line to be modified is the call to your ISR_Handler and perhaps the call to
;                   the label name OSTickISR1.
;********************************************************************************************************

OSTickISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register                                
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register                                
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register                                
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register                                
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {    
    cmpb   #$01                           
    bne    OSTickISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer     
    sts    0,y                         ; }                                          

OSTickISR1:
    call   OSTickISR_Handler
    call   OSTimeTick
;   cli                                ; Optionally enable interrupts to allow interrupt nesting
       
    call   OSIntExit                   ; Notify uC/OS-II about end of ISR
    
    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register                                
    
    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register                                

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register                                

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register                                
            
    rti                                ; Return from interrupt, no higher priority tasks ready.


;********************************************************************************************************
;********************************************************************************************************

PWM_ECT_ISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register                                
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register                                
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register                                
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register                                
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {    
    cmpb   #$01                           
    bne    PWM_ECT_ISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer     
    sts    0,y                         ; }                                          

PWM_ECT_ISR1:
    call   PwmEctISR_Handler
;   cli                                ; Optionally enable interrupts to allow interrupt nesting
       
    call   OSIntExit                   ; Notify uC/OS-II about end of ISR
    
    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register                                
    
    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register                                

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register                                

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register                                
            
    rti                                ; Return from interrupt, no higher priority tasks ready.

;********************************************************************************************************
;********************************************************************************************************

TIMER_CH4_ISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register                                
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register                                
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register                                
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register                                
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {    
    cmpb   #$01                           
    bne    TIMER_CH4_ISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer     
    sts    0,y                         ; }                                          

TIMER_CH4_ISR1:
    call   TimerCh4ISR_Handler
;   cli                                ; Optionally enable interrupts to allow interrupt nesting
       
    call   OSIntExit                   ; Notify uC/OS-II about end of ISR
    
    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register                                
    
    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register                                

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register                                

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register                                
            
    rti                                ; Return from interrupt, no higher priority tasks ready.

;********************************************************************************************************
;********************************************************************************************************
;                                           SCI0_ISR 
;
; Description : 
;
; Arguments   : none
;
; Notes       : 1) used as master-slaver communication link layer
;********************************************************************************************************

SCI0_ISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {
    cmpb   #$01
    bne    SCI0_ISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ; }


SCI0_ISR1:
    call   SCI0_ISR_Handler            ; Call Rx ISR handler. (Slave485.c)

;   cli                               ; Optionally enable interrupts to allow interrupt nesting

    call   OSIntExit                  ; Notify uC/OS-II about end of ISR, a context switch may occur from within OSIntExit().

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti  
 
 
 
  ;********************************************************************************************************
;                                           SCI1_ISR 
;
; Description : 
;
; Arguments   : none
;
; Notes       : 1) 
;********************************************************************************************************

SCI1_ISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {
    cmpb   #$01
    bne    SCI1_ISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ; }


SCI1_ISR1:
    call   SCI1_ISR_Handler  ; Call Rx ISR handler.   (See SD_SCI.c)

;   cli                               ; Optionally enable interrupts to allow interrupt nesting

    call   OSIntExit                  ; Notify uC/OS-II about end of ISR, a context switch may occur from within OSIntExit().

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti  
    
  ;********************************************************************************************************
;                                           SCI2_ISR 
;
; Description : 
;
; Arguments   : none
;
; Notes       : 1) 
;********************************************************************************************************

SCI2_ISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {
    cmpb   #$01
    bne    SCI2_ISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ; }


SCI2_ISR1:
    call   SCI2_ISR_Handler  ; Call Rx ISR handler.   (See SD_SCI.c)

;   cli                               ; Optionally enable interrupts to allow interrupt nesting

    call   OSIntExit                  ; Notify uC/OS-II about end of ISR, a context switch may occur from within OSIntExit().

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti  
    

    
 ;********************************************************************************************************
;                                           SCI3_ISR 
;
; Description : 
;
; Arguments   : none
;
; Notes       : 1) 
;********************************************************************************************************

SCI3_ISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {
    cmpb   #$01
    bne    SCI3_ISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ; }


SCI3_ISR1:
    call   SCI3_ISR_Handler  ; Call Rx ISR handler. (See HMI485.c)

;   cli                               ; Optionally enable interrupts to allow interrupt nesting

    call   OSIntExit                  ; Notify uC/OS-II about end of ISR, a context switch may occur from within OSIntExit().

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti  
 
 

;********************************************************************************************************
;                                           ADC0_ISR 
;
; Description : 
;
; Arguments   : none
;
; Notes       : 1) used as master-slaver communication link layer
;********************************************************************************************************

ADC0_ISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {
    cmpb   #$01
    bne    ADC0_ISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ; }


ADC0_ISR1:
    ;call   ADC0_ISR_Handler  ; Call ADC0 handler. (CAN4.c)
    call   xadc0_adc_isr
;   cli                               ; Optionally enable interrupts to allow interrupt nesting

    call   OSIntExit                  ; Notify uC/OS-II about end of ISR, a context switch may occur from within OSIntExit().

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti  
    
    
;********************************************************************************************************
;                                           ADC1_ISR 
;
; Description : 
;
; Arguments   : none
;
; Notes       : 1) used as master-slaver communication link layer
;********************************************************************************************************

ADC1_ISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {
    cmpb   #$01
    bne    ADC1_ISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ; }


ADC1_ISR1:
    ;call   ADC1_ISR_Handler  ; Call ADC0 handler. (CAN4.c)
    call   soc_adc1_isr
;   cli                               ; Optionally enable interrupts to allow interrupt nesting

    call   OSIntExit                  ; Notify uC/OS-II about end of ISR, a context switch may occur from within OSIntExit().

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti  
    

    
;********************************************************************************************************
;                                           CAN0_RX_ISR 
;
; Description : 
;
; Arguments   : none
;
; Notes       : 1) used as master-slaver communication link layer
;********************************************************************************************************

CAN0_RX_ISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {
    cmpb   #$01
    bne    CAN0_RX_ISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ; }


CAN0_RX_ISR1:
    call   CAN0_RX_ISR_Handler  ; Call ADC0 handler. (CAN4.c)

;   cli                               ; Optionally enable interrupts to allow interrupt nesting

    call   OSIntExit                  ; Notify uC/OS-II about end of ISR, a context switch may occur from within OSIntExit().

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti 
    
    
    
;********************************************************************************************************
;                                           CAN1_RX_ISR 
;
; Description : 
;
; Arguments   : none
;
; Notes       : 1) used as Car-Ems communication link layer
;********************************************************************************************************

CAN1_RX_ISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {
    cmpb   #$01
    bne    CAN1_RX_ISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ; }


CAN1_RX_ISR1:
    call   CAN1_RX_ISR_Handler  ; Call ADC0 handler. (CAN4.c)

;   cli                               ; Optionally enable interrupts to allow interrupt nesting

    call   OSIntExit                  ; Notify uC/OS-II about end of ISR, a context switch may occur from within OSIntExit().

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti    
      
;********************************************************************************************************
;                                           CAN2_RX_ISR 
;
; Description : 
;
; Arguments   : none
;
; Notes       : 1) used as master-slaver communication link layer
;********************************************************************************************************

CAN2_RX_ISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {
    cmpb   #$01
    bne    CAN2_RX_ISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ; }


CAN2_RX_ISR1:
    call   CAN2_RX_ISR_Handler  ; Call ADC0 handler. (CAN4.c)

;   cli                               ; Optionally enable interrupts to allow interrupt nesting

    call   OSIntExit                  ; Notify uC/OS-II about end of ISR, a context switch may occur from within OSIntExit().

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti    
          
    
;********************************************************************************************************
;                                           CAN3_RX_ISR 
;
; Description : 
;
; Arguments   : none
;
; Notes       : 1) used as master-slaver communication link layer
;********************************************************************************************************

CAN3_RX_ISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {
    cmpb   #$01
    bne    CAN3_RX_ISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ; }


CAN3_RX_ISR1:
    call   CAN3_RX_ISR_Handler  ; Call ADC0 handler. (CAN4.c)

;   cli                               ; Optionally enable interrupts to allow interrupt nesting

    call   OSIntExit                  ; Notify uC/OS-II about end of ISR, a context switch may occur from within OSIntExit().

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti    
          
       
;********************************************************************************************************
;                                           CAN4_RX_ISR 
;
; Description : 
;
; Arguments   : none
;
; Notes       : 1) used as master-slaver communication link layer
;********************************************************************************************************

CAN4_RX_ISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {
    cmpb   #$01
    bne    CAN4_RX_ISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ; }


CAN4_RX_ISR1:
    call   CAN4_RX_ISR_Handler  ; Call ADC0 handler. (CAN4.c)

;   cli                               ; Optionally enable interrupts to allow interrupt nesting

    call   OSIntExit                  ; Notify uC/OS-II about end of ISR, a context switch may occur from within OSIntExit().

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti    

;********************************************************************************************************
;                                           CAN3_RX_ISR 
;
; Description : 
;
; Arguments   : none
;
; Notes       : 1) used as master-slaver communication link layer
;********************************************************************************************************

;port_p_isr:
;    ldaa   GPAGE                       ; Get current value of GPAGE register
;    psha                               ; Push GPAGE register onto current task's stack
;
;    ldaa   EPAGE                       ; Get current value of EPAGE register
;    psha                               ; Push EPAGE register onto current task's stack
;
;    ldaa   RPAGE                       ; Get current value of RPAGE register
;    psha                               ; Push RPAGE register onto current task's stack
;
;    ldaa   PPAGE                       ; Get current value of PPAGE register
;    psha                               ; Push PPAGE register onto current task's stack
;
;    inc    OSIntNesting                ; Notify uC/OS-II about ISR
;
;    ldab   OSIntNesting                ; if (OSIntNesting == 1) {
;    cmpb   #$01
;    bne    port_p_isr1
;
;    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer
;    sts    0,y                         ; }
;
;
;port_p_isr1:
;    call   port_p_isr_handler  ; Call ADC0 handler. (CAN4.c)

;   cli                               ; Optionally enable interrupts to allow interrupt nesting

;    call   OSIntExit                  ; Notify uC/OS-II about end of ISR, a context switch may occur from within OSIntExit().
;
;    pula                               ; Get value of PPAGE register
;    staa   PPAGE                       ; Store into CPU's PPAGE register
;
;    pula                               ; Get value of RPAGE register
;    staa   RPAGE                       ; Store into CPU's RPAGE register
;
;    pula                               ; Get value of EPAGE register
;    staa   EPAGE                       ; Store into CPU's EPAGE register
;
;    pula                               ; Get value of GPAGE register
;    staa   GPAGE                       ; Store into CPU's GPAGE register
;
;    rti    



;********************************************************************************************************
;                                           I2C0_ISR 
;
; Description : 
;
; Arguments   : none
;
; Notes       : 1) used as master-slaver communication link layer
;********************************************************************************************************

I2C0_ISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {
    cmpb   #$01
    bne    I2C0_ISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ; }


I2C0_ISR1:
    call   i2c0_isr_handler


    call   OSIntExit                  ; Notify uC/OS-II about end of ISR, a context switch may occur from within OSIntExit().

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti    




;********************************************************************************************************
;                                           I2C0_ISR 
;
; Description : 
;
; Arguments   : none
;
; Notes       : 1) used as master-slaver communication link layer
;********************************************************************************************************

I2C1_ISR:
    ldaa   GPAGE                       ; Get current value of GPAGE register
    psha                               ; Push GPAGE register onto current task's stack

    ldaa   EPAGE                       ; Get current value of EPAGE register
    psha                               ; Push EPAGE register onto current task's stack

    ldaa   RPAGE                       ; Get current value of RPAGE register
    psha                               ; Push RPAGE register onto current task's stack

    ldaa   PPAGE                       ; Get current value of PPAGE register
    psha                               ; Push PPAGE register onto current task's stack

    inc    OSIntNesting                ; Notify uC/OS-II about ISR

    ldab   OSIntNesting                ; if (OSIntNesting == 1) {
    cmpb   #$01
    bne    I2C1_ISR1

    ldy    OSTCBCur                    ;     OSTCBCur->OSTCBStkPtr = Stack Pointer
    sts    0,y                         ; }


I2C1_ISR1:
    call   i2c1_isr_handler


    call   OSIntExit                  ; Notify uC/OS-II about end of ISR, a context switch may occur from within OSIntExit().

    pula                               ; Get value of PPAGE register
    staa   PPAGE                       ; Store into CPU's PPAGE register

    pula                               ; Get value of RPAGE register
    staa   RPAGE                       ; Store into CPU's RPAGE register

    pula                               ; Get value of EPAGE register
    staa   EPAGE                       ; Store into CPU's EPAGE register

    pula                               ; Get value of GPAGE register
    staa   GPAGE                       ; Store into CPU's GPAGE register

    rti    
    