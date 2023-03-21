/*
 * 	startup.c
 *
 */
 
#define n_row 4
#define n_col 4


#define GPIO_D          0x40020C00
#define GPIO_D_MODER    ((volatile unsigned int *)  GPIO_D)
#define GPIO_D_OTYPER   ((volatile unsigned short *)  (GPIO_D + 4))
#define GPIO_D_PUPDR    ((volatile unsigned int *)  (GPIO_D + 0xC))
#define GPIO_D_OSPEEDR  ((volatile unsigned int *)  (GPIO_D + 8))

//#define GPIO_ODR        ((volatile unsigned short *)  (GPIO_D + 0x14))
#define GPIO_ODR_LOW   ((volatile unsigned char *)  (GPIO_D + 0x14))
#define GPIO_ODR_HIGH    ((volatile unsigned char *)  (GPIO_D + 0x15))

#define GPIO_IDR_LOW   ((volatile unsigned char *)  (GPIO_D + 0x10))
#define GPIO_IDR_HIGH   ((volatile unsigned char *)  (GPIO_D + 0x11))


 
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}
//      intiera port gpio_port
//      skriv funktion out7seg()

void app_init(void){
    
    // starta klockor port D
    *((unsigned long *) 0x40023830) = 0x18;
    
    *((unsigned long *) GPIO_D_MODER) = 0x55005555;
    *((unsigned long *) GPIO_D_PUPDR) = 0x00AA0000;
    
    *GPIO_D_OSPEEDR = 0x55555555;

}

unsigned char keyb(void){
    int col, row;
    unsigned char key[] = {1, 2, 3, 0xA, 4, 5, 6, 0xB, 7, 8, 9, 0xC, 0xE, 0, 0xF, 0xD };
    for(row = 1; row <= 4; row++) {
        Activate_row(row);       
        col = Read_colomn();
        if( (col != 0 )) {
            Activate_row(0);
            return key [4*(row-1)+(col-1) ];
        }
    }
    Activate_row(0);
    return 0xFF;
}

void Activate_row(unsigned int r){
    
    switch(r) {
        
        case 1: *GPIO_ODR_HIGH = 0x10; break;
        case 2: *GPIO_ODR_HIGH = 0x20; break;
        case 3: *GPIO_ODR_HIGH = 0x40; break;
        case 4: *GPIO_ODR_HIGH = 0x80; break;
        default: *GPIO_ODR_HIGH = 0x00; break;
        
    }
}

int Read_colomn(void){
    
    unsigned char c;
    c = *GPIO_IDR_HIGH;
    
    if(c & 0x8) return 4;
    if(c & 0x4) return 3;
    if(c & 0x2) return 2;
    if(c & 0x1) return 1;
    return 0;
}


void out7seg(unsigned char c){
    
    switch(c) {
        case 0: *GPIO_ODR_LOW = 0x3F; break; // 00111111
        case 1: *GPIO_ODR_LOW = 0x06; break; // 00000110
        case 2: *GPIO_ODR_LOW = 0x5B; break; // 01011011
        case 3: *GPIO_ODR_LOW = 0x4F; break; // 01001111
        case 4: *GPIO_ODR_LOW = 0x66; break; // 01100110
        case 5: *GPIO_ODR_LOW = 0x6D; break; // 01101101
        case 6: *GPIO_ODR_LOW = 0x7D; break; // 01111101
        case 7: *GPIO_ODR_LOW = 0x07; break; // 00000111
        case 8: *GPIO_ODR_LOW = 0x7F; break; // 01111111
        case 9: *GPIO_ODR_LOW = 0x6F; break; // 01101111
        case 0xA: *GPIO_ODR_LOW = 0x77; break; // 01110111
        case 0xB: *GPIO_ODR_LOW = 0x7C; break; // 01111100
        case 0xC: *GPIO_ODR_LOW = 0x39; break; // 00111001
        case 0xD: *GPIO_ODR_LOW = 0x5E; break; // 01011110
        case 0xE: *GPIO_ODR_LOW = 0x79; break; // 01111001
        case 0xF: *GPIO_ODR_LOW = 0x71; break; // 01110001
        default: *GPIO_ODR_LOW = 0x00; break; // turn off display
    }
}


void main(void){
    app_init();
    
    while(1) {
        out7seg( keyb());
    }
}

