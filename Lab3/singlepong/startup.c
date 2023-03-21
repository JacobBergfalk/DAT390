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
    POINT p0;
    POINT p1;
} LINE, *PLINE;

typedef struct {
    POINT p0;
    char x,y; // ej punkt utan 2 värden
} RECT, *PRECT;

typedef struct polygonpoint{
    char x,y;
    struct polygonpoint *next;
} POLYPOINT, *PPOLYPOINT;

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
static void move_ballobject( POBJECT o);
static void set_ballobject_speed( POBJECT o, int speedx, int speedy);
static void move_paddleobject(POBJECT p);
static void set_paddleobject_speed(POBJECT o, int speedx, int speedy);
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

GEOMETRY paddle_geometry = {
    27,
    3, 6,
    {
    { 0, 0 },
    { 0, 1 },
    { 0, 2 },
    { 0, 3 },
    { 0, 4 },
    { 0, 5 },
    { 0, 6 },
    { 0, 7 },
    { 0, 8 },
    { 1, 0 },
    { 1, 8 },
    { 2, 0 },
    { 2, 3 },
    { 2, 4 },
    { 2, 5 },
    { 2, 8 },
    { 3, 0 },
    { 3, 8 },
    { 4, 0 },
    { 4, 1 },
    { 4, 2 },
    { 4, 3 },
    { 4, 4 },
    { 4, 5 },
    { 4, 6 },
    { 4, 7 },
    { 4, 8 },
    }
};

static OBJECT paddle_object = {
    &paddle_geometry,
    0,0,
    122, 30,
    draw_object,
    clear_object,
    move_paddleobject,
    set_paddleobject_speed
};

static OBJECT ball_object = { 
    &ball_geometry,
    2, 1,                           // x värde dubbelt så långt som y värdet
    1, 1,
    draw_object,
    clear_object,
    move_ballobject,
    set_ballobject_speed
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
    int xdir = 3;
    
    POBJECT b = &ball_object;
    POBJECT p = &paddle_object;
    
    init_app();
    graphic_initiallize();
    graphic_clear_screen();
     b->set_speed(b, xdir, 3);
    
    while(1) {
        b->move(b);
        p->move(p);
        //delay_milli(10);
        c = keyb();
        
        if(c == 6) {
            break;
        }
        
        switch(c) {
            case 3: p->set_speed(p, 0, -3); break;
            case 9: p->set_speed(p, 0, 3); break;
        }
        
        if(collision(b, p)) {
            xdir *= -1;
            b->set_speed(b, xdir, 3);
        }
        
        if(b->posx + b->geo->sizex > 128) {
            break;
        }
    }
    
}

static void move_paddleobject(POBJECT p) {
    
    int new_posy;
    
    p->clear(p);
    
    new_posy = p->posy + p->diry;           // sätt new_pos till nuvarande pos och direction
    
    if(new_posy + p->geo->sizey > 64) { // rör inte på sig om den rör kanten
        new_posy = 64 - p->geo->sizey; // så att hela paddle är inom området
    } else if(new_posy < 1) {
        new_posy = 1;                   //  rör inte på sig om den rör kanten
    }
    
    p->posy = new_posy;
    p->draw(p);
    
}

static void set_paddleobject_speed(POBJECT p, int speedx, int speedy) {
    p->diry = speedy; 
}

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

static void move_ballobject( POBJECT o) {
    
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
    } 
    //efter uträkningar, uppdatera pos för x och y till nya värderna.
    
    o->posx = new_posx;
    o->posy = new_posy;
    o->draw(o);
    
}

static void set_ballobject_speed( POBJECT o, int speedx, int speedy) {
    o->dirx = speedx;
    o->diry = speedy; 
}

static bool collision(POBJECT ball, POBJECT paddle) { // chans för kollision
    
    if(ball->posx < 120) {
        return false;
    }
     // kolla om boll är längre ner än paddel
    if(paddle->posy + paddle->geo->sizey < ball->posy ) {
        return false;
    }
     // kolla om boll är längre upp än paddel
    if(paddle->posy > ball->posy + ball->geo->sizey) {
        return false;
    }
    // om inget ger falskt, return true
    return true;
}


/*
 * 
 * 
 * Först kolla om boll och paddel är nära varandra, med y led 
 * 
 *  if ( bollens högra del är på 120 > )
 * 
 * för kollision :
 * 
 *  För ej kollision : det lägssta värdet på paddeln vara högre än högsta värdet på bollen,
 *  För ej kollision : det högsta värdet på paddeln vara lägre än lägsta värdet på bollen
 * 
 * 
 * Andra, kolla om punkter på paddeln har kontakt med bollen

 * */



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



void draw_polygon(PPOLYPOINT start) {
    
    PPOLYPOINT p0, p1;
    
    p0 = start;
    p1 = start->next;
    
    while(p0->next != 0) {

        LINE line = {p0->x, p0->y, p1->x, p1->y};
        draw_line(&line);
        
        p0 = p1;
        p1 = p1->next;
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