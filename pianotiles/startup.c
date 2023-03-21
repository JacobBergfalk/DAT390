/*
 * 	startup.c
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// INPUT
#define GPIO_D          0x40020C00
#define GPIO_D_MODER    ((volatile unsigned int *)  GPIO_D)
#define GPIO_D_OTYPER   ((volatile unsigned short *)  (GPIO_D + 4))
#define GPIO_D_PUPDR    ((volatile unsigned int *)  (GPIO_D + 0xC))
#define GPIO_D_OSPEEDR  ((volatile unsigned int *)  (GPIO_D + 8))
#define GPIO_D_IDR_LOW   ((volatile unsigned char *)  (GPIO_D + 0x10))
#define GPIO_D_IDR_HIGH   ((volatile unsigned char *)  (GPIO_D + 0x11))
#define GPIO_D_ODR_LOW    ((volatile unsigned char *) (GPIO_D + 0x14))
#define GPIO_D_ODR_HIGH   ((volatile unsigned char *) (GPIO_D + 0x15))

//OUTPUT
#define GPIO_E            0x40021000
#define GPIO_E_MODER      ((volatile unsigned int *)  GPIO_E)
#define GPIO_E_ODRLOW     ((volatile unsigned char *)  (GPIO_E + 0x14))
#define GPIO_E_ODR_LOW    ((volatile unsigned char *) (GPIO_E + 0x14))
#define GPIO_E_ODR_HIGH   ((volatile unsigned char *) (GPIO_E + 0x15))

#define SYSTICK     0xE000E010
#define STK_CTRL    ((volatile unsigned int *) SYSTICK)
#define STK_LOAD    ((volatile unsigned int *) (SYSTICK + 4))
#define STK_VAL     ((volatile unsigned int *) (SYSTICK + 8))
 
uint32_t state = 777;

char myRand(void){
   state = state * 1664525 + 1013904223;
   return state >> 24;
}



typedef int bool;
#define true 1
#define false 0

 
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

typedef struct {
    char x,y;
} POINT, *PPOINT;

typedef struct {
    POINT p0;
    POINT p1;
} LINE, *PLINE;

typedef struct {
    POINT start;
    POINT end;
} BUTTON, *PBUTTON;

static BUTTON b1 = {38,54, 48,64};
static BUTTON b2 = {52,54, 62,64};
static BUTTON b3 = {66,54, 76,64};
static BUTTON b4 = {80,54, 90,64};


typedef struct tObj {
    int diry;
    unsigned int posx, posy;        // pos x i main
    unsigned int length_x, length_y;
    bool active;
    void (*draw) (struct tObj *);
    void (*clear) (struct tObj *);
    void (*move) (struct tObj *);
    void (*set_speed) (struct tObj *, int);
} TILE, *PTILE;

void draw_tile_object(PTILE t);
void clear_tile_object( PTILE t);
void move_tile_object( PTILE t);
void set_tile_speed( PTILE t, int speed);


static TILE tile ={
    2,               // dirx = speed
    39,0,             // posx, posy, ändra i main vid ny tile
    8, 12,
    false,
    draw_tile_object,
    clear_tile_object,
    move_tile_object,
    set_tile_speed
};

void app_init(void ) {
    *((unsigned long *) 0x40023830) = 0x18;
    *GPIO_E_MODER = 0x00005555;
    
    *GPIO_D_MODER = 0x55005555;
    *GPIO_D_PUPDR = 0x00AA0000;
    *GPIO_D_OSPEEDR = 0x55555555;
}

void game_init(void) {
    // längd på tile är 10, 5 är avståndet mellan tiles. 
    draw_button_square();
}

// FÖR BUTTON
void draw_button_square(void) {
    draw_square( &b1 );
    draw_square( &b2 );
    draw_square( &b3 );
    draw_square( &b4 );
}

void draw_square( PBUTTON p ) {

    for(int i = 0; i <= 10; i++) { 
        graphic_pixel_set(p->start.x + i, p->start.y); 
    }
    for(int i = 0; i <= 10; i++) {
        graphic_pixel_set(p->start.x, p->start.y + i);
    }
    for(int i = 0; i <= 10; i++) {
        graphic_pixel_set(p->start.x + 10, p->start.y + i);
    }
    for(int i = 0; i <= 10; i++) {
        graphic_pixel_set(p->start.x + i, p->start.y + 10);
    }
}
// FÖR TILE
void draw_tile_object(PTILE t) {
    // om x > 63 dont draw
    if(t->active == false) {
        return;
    }
    
    for(int i = 0; i <= t->length_x; i++) { 
        graphic_pixel_set(t->posx + i, t->posy);
    }
    for(int i = 0; i <= t->length_y; i++) { 
        graphic_pixel_set(t->posx, t->posy + i);
    }
    for(int i = 0; i <= t->length_x; i++) { 
        graphic_pixel_set(t->posx + i, t->posy + t->length_y);
    }
    for(int i = 0; i <= t->length_y; i++) { 
        graphic_pixel_set(t->posx + t->length_x, t->posy + i);
    }
}

void clear_tile_object( PTILE t) {                  // fixa bug
    
    if(t->active == false) {
        return;
    }
    
    for(int i = 0; i <= t->length_x; i++) { 
        graphic_pixel_clear(t->posx + i, t->posy);
    }
    for(int i = 0; i <= t->length_y; i++) { 
        graphic_pixel_clear(t->posx, t->posy + i);
    }
    for(int i = 0; i <= t->length_x; i++) { 
        graphic_pixel_clear(t->posx + i, t->posy + t->length_y);
    }
    for(int i = 0; i <= t->length_y; i++) { 
        graphic_pixel_clear(t->posx + t->length_x, t->posy + i);
    }
}

void move_tile_object(PTILE t) {
    
    int new_posy;
    
    t->clear(t);
    new_posy = t->posy + t->diry;

    if(new_posy + t->length_y > 64 && t->length_y < 1) {
        t->length_y -= 1;
    }
    
    t->posy = new_posy;
    t->draw(t);
}

void set_tile_speed(PTILE t, int speed) {
    t->diry = speed; 
}

// KEYBIND 
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
        
        case 1: *GPIO_D_ODR_HIGH = 0x10; break;
        case 2: *GPIO_D_ODR_HIGH = 0x20; break;
        case 3: *GPIO_D_ODR_HIGH = 0x40; break;
        case 4: *GPIO_D_ODR_HIGH = 0x80; break;
        default: *GPIO_D_ODR_HIGH = 0x00; break;
        
    }
}

int Read_colomn(void){
    
    unsigned char c;
    c = *GPIO_D_IDR_HIGH;
    
    if(c & 0x8) return 4;
    if(c & 0x4) return 3;
    if(c & 0x2) return 2;
    if(c & 0x1) return 1;
    return 0;
}

static bool collision (PTILE t, char c ) { // chans för kollision
    
    if(t->posy + t->length_y < 54) return false;
    char right_side = 38 + 14*(c - 1), left_side = right_side + 10;
    
    // för rätt knapp
    if(t->posx + t->length_x < right_side) {
        return false;
    }
    if(t->posx > left_side) {
        return false;
    }
    
    return true;
}

void fill_square(char c ) {
    if(c == 0) return;
    for(int x = 0; x < 9; x += 2) {
        for(int y = 0; y < 9; y += 2) {
            graphic_pixel_set(38 + 14*(c -1) + x, 54 + y);
        }
    }
}

void clear_square() {
    for(int c = 1; c < 5; c++ ){
        for(int x = 2; x < 9; x += 2) {
            for(int y = 2; y < 9; y += 2) {
                graphic_pixel_clear(38 + 14*(c -1) + x, 54 + y);
            }
        }
    }  
}


void main(void)
{
    app_init();
    graphic_initiallize();
    graphic_clear_screen();
    game_init();
    
    int random1 = myRand() % 4, random2 = myRand() % 4, random_y, last_random = 2;
    bool game_over = false;
    char c = 0;
    int speed = 2, counter = 0, score = 0;
    
    TILE t1 = tile;
    TILE t2 = tile;
    
    while(!game_over) {
        if(t1.active == false ) t1 = tile;              
        if (t2.active == false) t2 = tile;
        
        while(random1 == last_random) {
            random1 = myRand() % 4;
        }
        last_random = random1;
        
        while(random1 == random2 ) {
            random2 = myRand() % 4;
        }
        
        if(!t1.active) {
            t1.posx += random1 * 14;
            t1.active = true;
            t1.set_speed(&t1, speed);
        }
        if(speed > 3 && !t2.active) {
            t2.posy -= 27;
            t2.posx += random2 * 14;
            t2.active = true;
            t2.set_speed(&t2, speed);
        }
        
        while(t1.active) {
            t1.move(&t1);
            t2.move(&t2);
            draw_button_square();
            
            c = keyb();
            
            if ( c != 0xFF) {
                if(c == 0xA) c = 4;
                fill_square(c);
            } else {
                clear_square();
            }
            
            if( collision(&t1, c) ) {
                t1.clear(&t1);
                t1.active = false;
                counter++;
                score++;
                if(counter == 2) {
                    counter = 0;
                    speed++;
                }
            }
            
            if( collision(&t2, c) ) {
                t2.clear(&t2);
                t2.active = false;
                counter++;
                score++;
                if(counter == 5) {
                    counter = 0;
                    speed++;
                }
            }
            *GPIO_D_ODR_LOW = score;
            
            if((t1.posy > 63 && t1.posy < 70) || (t2.posy > 63 && t2.posy < 70)) {
                t1.active = false;
                t2.active = false;
                game_over = true;
            }
        }
    }
    graphic_clear_screen();
}