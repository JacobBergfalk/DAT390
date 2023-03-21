/*
 * 	startup.c
 *
 */
 
 #include <math.h>
 /*
#define GPIO_E          0x40021000
#define GPIO_E_MODER    ((volatile unsigned int *)  GPIO_E)
#define GPIO_E_ODRLOW   ((volatile unsigned char *)  (GPIO_E + 0x14))
*/
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

#define SYSTICK     0xE000E010
#define STK_CTRL    ((volatile unsigned int *) SYSTICK)
#define STK_LOAD    ((volatile unsigned int *) SYSTICK + 4)
#define STK_VAL     ((volatile unsigned int *) SYSTICK + 8)

typedef int bool;
#define true 1
#define false 0

//#define SIMULATOR
#define MAX_POINTS 30


typedef struct {
    char x,y;
} POINT, *PPOINT;

typedef struct {
    int numpoints;          // antal pixlar
    int sizex;              //storleken
    int sizey;
    POINT px[ MAX_POINTS];  // 30 punkter
} GEOMETRY, *PGEOMETRY;

typedef struct tObj {
    PGEOMETRY geo;
    int dirx, diry;                     //riktning
    unsigned int posx, posy;                     // position
    void (*draw) (struct tObj *);
    void (*clear) (struct tObj *);
    void (*move) (struct tObj *);
    void (*set_speed) (struct tObj *, int, int);
    
} OBJECT, *POBJECT;

static void draw_object(POBJECT o);
static void clear_object( POBJECT o);
static void move_object( POBJECT o);
static void set_object_speed( POBJECT o, int speedx, int speedy);
static bool collision(POBJECT ball, POBJECT paddle);
unsigned char keyb(void);

GEOMETRY ball_geometry = {
    12, 
    4,4,
    { {0,1}, 
    {0,2}, 
    {1,0}, 
    {1,1}, 
    {1,2}, 
    {1,3}, 
    {2,0}, 
    {2,1}, 
    {2,2}, 
    {2,3}, 
    {3,1}, 
    {3,2} }
};

GEOMETRY spider_geometry = {
    22, 
    6, 8,
    {   {0,2},
        {0,3},
        {0,7},
        {1,1},
        {1,2},
        {1,4},
        {1,6},
        {2,0},
        {2,2},
        {2,3},
        {2,5},
        {3,0},
        {3,2},
        {3,3},
        {3,5},
        {4,1},
        {4,2},
        {4,4},
        {4,6},
        {5,2},
        {5,3},
        {5,7} }
};

static OBJECT spider_object = {
    &spider_geometry,
    0,0,
    60,30,
    draw_object,
    clear_object,
    move_object,
    set_object_speed
};

static OBJECT ball_object = { 
    &ball_geometry,
    2, 1,                           // x värde dubbelt så långt som y värdet
    1, 1,
    draw_object,
    clear_object,
    move_object,
    set_object_speed
};

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
    //*((unsigned long *) GPIO_E_MODER) = 0x00005555;
    *GPIO_D_MODER = 0x55005555;
    *GPIO_D_PUPDR = 0x00AA0000;
    
    *GPIO_D_OSPEEDR = 0x55555555;
}

void main(void)
{
    unsigned char c;
    
    POBJECT victim = &ball_object;
    POBJECT spoder = &spider_object;
    
    init_app();
    graphic_initiallize();
    graphic_clear_screen();
    victim->set_speed(victim, 4, 1);
    
    while(1) {
        victim->move(victim);
        spoder->move(spoder);
        
        c = keyb();
        
        switch(c) {
            case 6: spoder->set_speed( spoder, 2, 0); break;
            case 4: spoder->set_speed( spoder, -2, 0); break;
            case 2: spoder->set_speed( spoder, 0, -2); break;
            case 8: spoder->set_speed( spoder, 0, 2); break;
            default: spoder->set_speed( spoder, 0, 0); break;
        }
        
        if(object_overlap(victim, spoder)) {
            break;
        }
        //delay_milli(20);
    }
    
    
}
// Funktioner för POBJECT structs
static void draw_object(POBJECT o) {
    
    for(int p = 0; p < o->geo->numpoints; p++) {
        graphic_pixel_set(o->geo->px[p].x + o->posx, o->geo->px[p].y + o->posy);
    }
}

static void clear_object( POBJECT o) {
    for(int p = 0; p < o->geo->numpoints; p++) {
        graphic_pixel_clear(o->geo->px[p].x + o->posx, o->geo->px[p].y + o->posy);
    }
}

static void move_object( POBJECT o) {
    
    int new_posx, new_posy;
    
    o->clear(o);
        
    new_posx = o->posx + o->dirx;
    new_posy = o->posy + o->diry;
    
    // check roof and floor
    if( new_posy + o->geo->sizey > 64) {
        o->diry *= -1;
        unsigned int height = o->geo->sizey;
        new_posy = 64;
        new_posy = new_posy - height;
        
    } else if(new_posy< 1) {
        o->diry *= -1; 
        new_posy = 1;
    }
    
    // check left wall
    if(new_posx < 1) {
        o->dirx *= -1;
        new_posx = 1;
    }  else if(new_posx > 128) {
        o->dirx *= -1;
        new_posx = 128;
    } 
    //efter uträkningar, uppdatera pos för x och y till nya värderna.
    
    o->posx = new_posx;
    o->posy = new_posy;
    o->draw(o);
    
}

static void set_object_speed( POBJECT o, int speedx, int speedy) {
    o->dirx = speedx;
    o->diry = speedy; 
}


// overlap
bool object_overlap(POBJECT victim, POBJECT spoder) {
            // avgör om bollen är utanför spindeln y led, mellan båda x koordenaterna
    // kolla om boll är längre ner än paddel
    if(spoder->posy + spoder->geo->sizey < victim->posy ) {
        return false;
    }
     // kolla om boll är längre upp än paddel
    if(spoder->posy > victim->posy + victim->geo->sizey) {
        return false;
    }
    
                // avgör om bollen är utanför spidelns x led. mellan båda y koordinaterna
    
    if(spoder->posx > victim->posx + victim->geo->sizex) {
        return false;
    }
    
    if(spoder->posx + spoder->geo->sizex < victim->posx ) {
        return false;
    }
    // om victim är innanför spidelns area så kommer return true
    return true;
    
}


// för keyboard
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


// delay 
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