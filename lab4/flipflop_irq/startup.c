/*
 * 	startup.c
 *
 */
 
#define PORT_D          0x40020C00
#define GPIO_MODER      ((volatile unsigned int *)(PORT_D))
#define GPIO_OTYPER     ((volatile unsigned short *)(PORT_D + 0x4))
#define GPIO_SPEEDR     ((volatile unsigned int *)(PORT_D + 0x8))
#define GPIO_PUPDR      ((volatile unsigned int *)(PORT_D + 0xC))
#define GPIO_IDR_LOW    ((volatile unsigned char *)(PORT_D + 0x10))
#define GPIO_IDR_HIGH   ((volatile unsigned char *)(PORT_D + 0x11))
#define GPIO_ODR_LOW    ((volatile unsigned char *)(PORT_D + 0x14))
#define GPIO_ODR_HIGH   ((volatile unsigned char *)(PORT_D + 0x15))

#define GPIO_E            0x40021000
#define GPIO_E_MODER      ((volatile unsigned int *)  GPIO_E)
#define GPIO_E_ODRLOW     ((volatile unsigned char *)  (GPIO_E + 0x14))
#define GPIO_D_IDR_LOW    ((volatile unsigned char *)(PORT_E + 0x10))
#define GPIO_D_IDR_HIGH   ((volatile unsigned char *)(PORT_E + 0x11))
#define GPIO_D_ODR_LOW    ((volatile unsigned char *)(PORT_E + 0x14))
#define GPIO_D_ODR_HIGH   ((volatile unsigned char *)(PORT_E + 0x15))

#define EXTI                0x40013C00                                  // def adress
#define EXTI_IMR            ((unsigned int *) EXTI )                    // Interupt mask reg, 
#define EXTI_EMR            ((unsigned int *) (EXTI + 0x4))             // eventmask reg
#define EXTI_RTSR           ((unsigned int *) (EXTI + 0x8))             // pos flank
#define EXTI_FTSR           ((unsigned int *) (EXTI + 0xC))             // neg flank
#define EXTI_PR             ((unsigned int *) (EXTI + 0x14))            // Återställnings reg

#define EXTI3_VTOR_IRQ      0x2001C064                                  // avbrottsvektor för EXTI3

#define SYSCFG_EXTICR1      0x40013808                                  // bit 12 - 15 för knapparna IRQ
#define NVIC_ISER0          0xE000E100                                  // nvic
#define NVIC_EXTI3_IRQ_BPOS (1<<9)                                      // flyttar en 1 till bit 9  (avbrottsvektorn bit 9)
#define EXTI3_IRQ_BPOS      (1<<3)                                      // flyttar en 1 till bit 3


static volatile int count;

__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

void init_app(void) {
    
    /* starta klockor port D och E */
	* ( (unsigned long *) 0x40023830) = 0x18;
	/* starta klockor för SYSCFG */
	* ((unsigned long *)0x40023844) |= 0x4000; 	
	/* Relokera vektortabellen */
	* ((unsigned long *)0xE000ED08) = 0x2001C000;
        
    // D output
    *GPIO_MODER = 0x55555555;
    *GPIO_E_MODER = 0x00005555;
    
    count = 1;
    
    // reset på flipflop
    *GPIO_E_ODR_LOW = 0x70; 
    *GPIO_E_ODR_LOW = ~0x70;
    
    *SYSCFG_EXTICR1 |= 0x4000; 

    // aktivera interrupt mask
    *EXTI_IMR |= EXTI3_IRQ_BPOS; 
     
    // aktivera rising edge
    *EXTI_RTSR |= EXTI3_IRQ_BPOS;
    // inaktivera falling edge
    *EXTI_FTSR |= EXTI3_IRQ_BPOS;
    
    
    *((void(**)(void)) 0x2001C03C) = irq_handler;

    // aktivera nvic
    *((unsigned int *) NVIC_ISER0) |= NVIC_EXTI3_IRQ_BPOS;  
    
}


void irq_handler(void) {
    
    char input = 0xF & *GPIO_E_IDR_LOW; // Få bitarna 0-3
    
    if( *EXTI_PR & EXTI3_IRQ_BPOS) {    // om trigg har uppträtt, 
        
         *EXTI_PR |= EXTI3_IRQ_BPOS; // återställer flaggan         ÅTERKOM
         
         if(input & 1) {                // Om IRQ0 är triggad
             *GPIO_E_ODR_LOW = 0x10;    // kvittera avbrott IRQ0
             count++;
         }
         if(input & 2) {                // Om IRQ1 är triggad
            *GPIO_E_ODR_LOW = 0x20;     // kvittera avbrott IRQ1
            count = 0;
             
         }
         if(input & 4) {                //Om IRQ2 är triggad
             *GPIO_E_ODR_LOW = 0x40;    // kvittera avbrott IRQ2
             
             if ( count > 0) count = 0; // 
             else count = 0xFF;         // 
         }
         
         // reset på flipflop
        *GPIO_E_ODR_LOW = 0x70; 
        *GPIO_E_ODR_LOW = ~0x70;
    }
    
    
}


void main(void)
{
    init_app();
    while(1) {
        *GPIO_ODR_LOW = count;
    }
}

