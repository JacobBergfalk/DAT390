/*
 * 	startup.c
 *
 */
 
 #include <math.h>
 
#define GPIO_E      0x40021000
#define GPIO_E_MODER    ((volatile unsigned int *)  GPIO_E)
#define GPIO_E_ODRLOW   ((volatile unsigned char *)  (GPIO_E + 0x14))

#define SYSTICK     0xE000E010
#define STK_CTRL    ((volatile unsigned int *) SYSTICK)
#define STK_LOAD    ((volatile unsigned int *) SYSTICK + 4)
#define STK_VAL     ((volatile unsigned int *) SYSTICK + 8)
#define SIMULATOR 1;


typedef struct {
    char x,y;
} POINT, *PPOINT;

typedef struct {
    POINT p0;
    POINT p1;
} LINE, *PLINE;

typedef struct {
    POINT p0;
    char x,y; // ej punkt utan 2 värden
} RECT, *PRECT;

__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

__attribute__((naked))
void graphic_initiallize(void) {
    __asm__ volatile (" .HWORD 0xDFF0\n");
    __asm__ volatile (" BX LR\n");
}

__attribute__((naked))
void graphic_clear_screen(void) {
    __asm__ volatile (" .HWORD 0xDFF1\n");
    __asm__ volatile (" BX LR\n");
}

__attribute__((naked))
void graphic_pixel_set(int x, int y) {
    __asm__ volatile (" .HWORD 0xDFF2\n");
    __asm__ volatile (" BX LR\n");
}

__attribute__((naked))
void graphic_pixel_clear(int x, int y) {
    __asm__ volatile (" .HWORD 0xDFF3\n");
    __asm__ volatile (" BX LR\n");
}


void init_app( void ) {
    // starta klockor port D och E 
    *((unsigned long *) 0x40023830) = 0x18;
    *((unsigned long *) GPIO_E_MODER) = 0x00005555;
}

void main(void)
{
    RECT rectangles[] = {
        {10,10, 20,10},
        {25,25, 10,20},
        {40,30, 70,20},
        {60,35, 10,10},
        {70,10, 5,5},
    };
    
    graphic_initiallize();
    graphic_clear_screen();
    
    while(1) {
        for(int i = 0; i < sizeof(rectangles) / sizeof(RECT); i++) {
            draw_rectagle(&rectangles[i]);
            delay_milli(500);
        }
        graphic_clear_screen();
    }
}

void draw_rectagle(PRECT r) {
                // startpunkt                          slutpunkt
        // skriver en linje mellan två punkter, detta sker 4 gånger för att bilda en rektangel
    LINE line = {r->p0.x, r->p0.y,              r->p0.x + r->x, r->p0.y}; // (start, end) == (startx, starty,    endx, endy)
    draw_line ( &line ); 
    
    LINE line2 = {r->p0.x + r->x, r->p0.y,               r->p0.x + r->x, r->p0.y + r->y};
    draw_line ( &line2 );
    
    LINE line3 = { r->p0.x + r->x, r->p0.y + r->y,        r->p0.x, r->p0.y + r->y};
    draw_line ( &line3 );
    
    LINE line4 = { r->p0.x, r->p0.y + r->y,               r->p0.x, r->p0.y};
    draw_line ( &line4 );
}

int draw_line(PLINE l){

    int steep, deltax, deltay, error = 0, ystep, temp, y;
    
    
    if( abs(l->p1.y - l->p0.y) > abs(l->p1.x - l->p0.x) ) {
        steep = 1;
    } else { // <=
        steep = 0;
    }
        
    if( steep == 1) {
        
        temp = l->p0.x;
        l->p0.x = l->p0.y;
        l->p0.y = temp;
        
        temp = l->p1.x;
        l->p1.x = l->p1.y;
        l->p1.y = temp;
    }
 
    if ( l->p0.x > l->p1.x) {
        
        temp = l->p0.x;
        l->p0.x = l->p1.x;
        l->p1.x = temp;
        
        temp = l->p0.y;
        l->p0.y = l->p1.y;
        l->p1.y = temp;
    }
    
    deltax = l->p1.x - l->p0.x;
    deltay = abs(l->p1.y - l->p0.y);
    
    y = l->p0.y;
    
    if(l->p0.y < l->p1.y) {
        ystep = 1;
    } else {
        ystep = -1;
    }
    
    for(int x = l->p0.x; x <= l->p1.x; x++) {
        if( steep == 1) {
            graphic_pixel_set(y,x);
        }
        else  {
            graphic_pixel_set(x,y);
        }
        
        error += deltay;
        if( 2 * error >= deltax) {
            y += ystep;
            error -= deltax;
        }
    }
    
    return 0;
}

void delay_250ns(void) {
    
    /* SystemCoreClock = 168000000 */
    *STK_CTRL = 0;
    *STK_LOAD = ( (168/4) -1 );
    *STK_VAL = 0;
    *STK_CTRL = 5;
    while( (*STK_CTRL & 0x10000 ) == 0 );
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