/*
 * 	startup.c
 *
 */

#define PORT_D 0x40020C00
#define GPIO_MODER ((volatile unsigned int *)(PORT_D))
#define GPIO_OTYPER ((volatile unsigned short *)(PORT_D + 0x4))
#define GPIO_SPEEDR ((volatile unsigned int *)(PORT_D + 0x8))
#define GPIO_PUPDR ((volatile unsigned int *)(PORT_D + 0xC))
#define GPIO_IDR_LOW ((volatile unsigned char *)(PORT_D + 0x10))
#define GPIO_IDR_HIGH ((volatile unsigned char *)(PORT_D + 0x11))
#define GPIO_ODR_LOW ((volatile unsigned char *)(PORT_D + 0x14))
#define GPIO_ODR_HIGH ((volatile unsigned char *)(PORT_D + 0x15))

#define SYSTICK     0xE000E010
#define STK_CTRL    ((volatile unsigned int *) SYSTICK)
#define STK_LOAD    ((volatile unsigned int *) SYSTICK + 4)
#define STK_VAL     ((volatile unsigned int *) SYSTICK + 8)
#define SIMULATOR

void systick_irq_handler( void );
void delay_1mikro( void );
void delay( unsigned int count );

__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

static volatile int systick_flag;
static volatile int delay_count;

#ifdef SIMULATOR 
#define DELAY_COUNT 100
#else 
#define DELAY_COUNT 1000000
#endif

void init_app( void ) {
   
	/* starta klockor port D och E */
	* ( (unsigned long *) 0x40023830) = 0x18;
	/* starta klockor för SYSCFG */
	* ((unsigned long *)0x40023844) |= 0x4000; 	
	/* Relokera vektortabellen */
	* ((unsigned long *)0xE000ED08) = 0x2001C000;
	
	// port d output 
    *GPIO_MODER = 0x55555555;
	*GPIO_OTYPER = 0x0000;
	*GPIO_SPEEDR = 0x55555555;

	*((void(**)(void)) 0x2001C03C) = systick_irq_handler;
}

int main(void)
{
    init_app();
    
    *GPIO_ODR_LOW = 0;
    delay(DELAY_COUNT);
    *GPIO_ODR_LOW = 0xFF;
    
    while(1) {
        if (systick_flag) {
            break;
        }
    }
    *GPIO_ODR_LOW = 0;
    
    return 0;
}

void systick_irq_handler( void ){
    //*STK_CTRL = 0;
    delay_count--;
    if( delay_count > 0 ) delay_1mikro(); 
    else {
        *STK_CTRL = 0;
        systick_flag = 1;
    }
}

void delay_1mikro( void ){
  *STK_CTRL = 0;
  *STK_LOAD = ( 168 - 1 );
  *STK_VAL = 0;
  *STK_CTRL = 7;
}

void delay( unsigned int count ){
  if( count == 0 ) return;
  delay_count = count;
  systick_flag = 0;
  delay_1mikro();
}

