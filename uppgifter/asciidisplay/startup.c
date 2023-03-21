/*
 * 	startup.c
 *
 */
 
#define GPIO_E      0x40021000

#define GPIO_E_MODER        ((volatile unsigned int *)  GPIO_E)
#define GPIO_E_OTYPER       ((volatile unsigned int *)  GPIO_E + 4)
#define GPIO_E_OSPEED       ((volatile unsigned int *)  GPIO_E + 8)
#define GPIO_E_PUPDR        ((volatile unsigned int *)  GPIO_E + 0xC)

#define GPIO_E_IDRLOW       ((volatile unsigned char *)  (GPIO_E + 0x10))
#define GPIO_E_IDRHIGH      ((volatile unsigned char *)  (GPIO_E + 0x11))

#define GPIO_E_ODRLOW       ((volatile unsigned char *)  (GPIO_E + 0x14))
#define GPIO_E_ODRHIGH      ((volatile unsigned char *)  (GPIO_E + 0x15))


#define B_E                 0x40 /* Enable-signal */
#define B_SELECT            4 /* Välj ASCII-display */
#define B_RW                2 /* 0=Write, 1=Read */
#define B_RS                1 /* 0=Control, 1=Data */

#define SYSTICK     0xE000E010

#define STK_CTRL    ((volatile unsigned int *) SYSTICK)
#define STK_LOAD    ((volatile unsigned int *) SYSTICK + 4)
#define STK_VAL     ((volatile unsigned int *) SYSTICK + 8)
#define STK_CALIB   ((volatile unsigned int *) SYSTICK + 0xC) 

unsigned char ascii_read_status(void);

__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}


void ascii_init(void) {
    
    *GPIO_E_ODRHIGH = 0b0000111000;
    delay_mikro(40);
    *GPIO_E_ODRHIGH = 0b0000001110;
    delay_mikro(40);
    *GPIO_E_ODRHIGH = 0b0000000110;
    delay_mikro(40);
    
}

void app_init(void) {
    
    //för fördröjning
    // starta klockor port D och E 
    *((unsigned long *) 0x40023830) = 0x18;
    *((unsigned long *) GPIO_E_MODER) = 0x00005555;
}

void main(void){
    char *s;
    char test1[] = "Alfanumerisk ";
    char test2[] = "Display - test";
    
    app_init();
    ascii_init();
    ascii_gotoxy(1,1);
    s = test1;
    
    while( *s ) ascii_write_char( *s++);
    ascii_gotoxy(1,2);
    s = test2;
    while( *s) ascii_write_char( *s++);
    return 0;
}

void ascii_gotoxy(int x, int y) {
    *GPIO_E_ODRHIGH = x - 1;
    if(y == 2) {
        *GPIO_E_ODRHIGH = *GPIO_E_ODRHIGH + 0x40;
    }
    ascii_write_cmd(0x80 | *GPIO_E_ODRHIGH);
}

void ascii_write_char(unsigned char c)  {
    
    while(( ascii_read_status() & 0x80) == 0x80){}
    delay_mikro(8);
    ascii_write_data(c);
    delay_mikro(44);
}

void delay_250ns(void) {
    
    /* SystemCoreClock = 168000000 */
    *STK_CTRL = 0;
    *STK_LOAD = ( (168/4) -1 );
    *STK_VAL = 0;
    *STK_CTRL = 5;
    while( (*STK_CTRL & 0x10000 )== 0 );
    *STK_CTRL = 0;
}

void delay_milli(unsigned int ms) {
    
    #ifdef SIMULATOR
        ms = ms/1000;
        ms++;
    #endif
    
    ms *= 1000;
    
    while( ms > 0 )
    {
        
    delay_250ns();
    delay_250ns();
    delay_250ns();
    delay_250ns();
    ms--;
    }
}

void delay_mikro(unsigned int us) {
    
    
    while( us > 0 ) {
    
    delay_250ns();
    delay_250ns();
    delay_250ns();
    delay_250ns();
    us--;
    }
}

void ascii_ctrl_bit_set( char x ){ /* x: bitmask bits are 1 to set */

    char c;
    c = *GPIO_E_ODRLOW;
    *GPIO_E_ODRLOW = B_SELECT | x | c;
}

void ascii_ctrl_bit_clear( char x ){ /* x: bitmask bits are 1 to clear */

    char c;
    c = *GPIO_E_ODRLOW;
    c = c & ~x;
    *GPIO_E_ODRLOW = B_SELECT | c;
}

char ascii_read_controller( void ){
    
    char c;
    ascii_ctrl_bit_set( B_E );
    delay_250ns();
    delay_250ns();
    c = *GPIO_E_IDRHIGH;
    ascii_ctrl_bit_clear( B_E );
    return c;
}

void ascii_write_controller( char c ){
    
    ascii_ctrl_bit_set( B_E );
    *GPIO_E_ODRHIGH = c; 
    ascii_ctrl_bit_clear( B_E );
    delay_250ns();
}

void ascii_write_cmd(unsigned char command) {
    
    ascii_ctrl_bit_clear( B_RS );       // RS = 0 clEar
    ascii_ctrl_bit_clear( B_RW );       // RW = 0
    ascii_write_controller(command);
}

void ascii_write_data(unsigned char data) {
    
    ascii_ctrl_bit_set( B_RS );
    ascii_ctrl_bit_clear( B_RW ); 
    ascii_write_controller(data);
}

unsigned char ascii_read_status( void ){
    char c;
    *GPIO_E_MODER = 0x00005555;
    ascii_ctrl_bit_set( B_RW );
    ascii_ctrl_bit_clear( B_RS );
    c = ascii_read_controller();
    *GPIO_E_MODER = 0x55555555;
    return c;
}


char ascii_read_data( void ){
    
    char c;
    *GPIO_E_MODER = 0x00005555;
    ascii_ctrl_bit_set( B_RW );
    ascii_ctrl_bit_set( B_RS );
    c = ascii_read_controller();
    *GPIO_E_MODER = 0x55555555;
    return c;
    
}