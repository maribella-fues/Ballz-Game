// Name: Maribella Fues
// Class: Fundamentals of Computing
// Assignment: project.c
// Purpose: run a ball game in a separate graphics window

# include <stdio.h>
# include <math.h>
# include <stdlib.h>
# include <time.h>
# include <unistd.h>
# include "gfx.h"

typedef struct {
    int wd; // width of the graphics window
    int ht; // height of the graphics window
    int buffer; // height of empty space at top of window to display score
    int ns; // total number of squares
    int nb; // total number of balls
    int sqsz; // max possible size of each square
    int score; // player score
    int start; // indicates if it is the start of the game
    int won; // indicates if the player beat the game (makes sure program doesn't go outside array size)
} Board;

typedef struct {
    int x; // x coord. of the center of the square
    int y; // y coord. of the center of the square
    int sz; // side length of the square
    int hits; // number of hits needed to delete the square
} Square;

typedef struct {
    float x; // x coord. of the center of the ball
    float y; // y coord. of the center of the ball
    int r; // radius of the ball
    float dx; // change in x coord. each time ball moves
    float dy; // change in y coord. each time ball moves
} Ball;

void instructions(Board *pb);
void drawSquare(int x, int y, int s);
void start(Board *pb, int r, Square s[], Ball b[]);
int play(Board *pb, Square s[], Ball b[]);
int moveBalls(Board *pb, int nb, Square s[], Ball b[]);
void bounce(int n, Board *pb, Ball b[]);
void collide(int n, Board *pb, Square s[], Ball b[]);
void lower(Board *pb, Square s[], Ball b[]);
int checkEnd(Board *pb, Square s[]);
void dispSquares(Board *pb, Square s[]);

int main() {
   time_t t;
   srand((unsigned) time(&t));

   Board board = {502, 530, 25, 0, 1, 50, 0, 1, 0}; // width, height, buffer, num squares, num balls, square size, score, start of game, won game
   Board *pb = &board; // pointer to the board to access and edit the attributes from any function that the pointer is passed to

   gfx_open(board.wd, board.ht, "Ballz Game");

   int c, r = 5;
   Square s[500];
   Ball b[501];
   int quit = 0; //indicates whether to continue playing the game or not

   instructions(pb);
   // waits for user to start the game
   while(1) {
        c = gfx_wait();
        if(c == 'p') break;
        if(c == 'q') { quit = 1; break; }
   }
   // runs the game until the user ends it or loses
   while(!quit) {
        start(pb, r, s, b);
        while(1) {
            c = gfx_wait(); // checks for user event 
            if(c == 'q') { quit = 1; break; } // breaks the loop to end the program if the user selects 'q'
            if(c == 1) if(play(pb, s, b)) break; // starts the round once the user clicks their mouse
        }

        // only runs if the user lost the game, rather than quit
        if(!quit) {
            gfx_clear();
            if(pb->won) gfx_text(pb->wd/3+50, pb->ht/2 - 10, "You Won!"); // ending if program maxed out the square/ball arrays
            else gfx_text(pb->wd/3+50, pb->ht/2 - 10, "Game Over"); // ending when player loses the game
            char str[20];
            sprintf(str, "Score: %d", pb->score);
            gfx_text(pb->wd/3+50, pb->ht/2 + 10, str); // displays final score
            gfx_text(pb->wd/4, pb->ht/2 + 30, "Press 'q' to quit or 'p' to play again.");
            // waits for the user to click either 'q' or 'p'
            while(1) {
                c = gfx_wait();
                if(c == 'q') { quit = 1; break; }
                if(c == 'p') break; // restarts the game from beginning of loop
            }
        }

   }
}

// displays the initial game instructions to the user on the graphics window
void instructions(Board *pb) {
    gfx_clear();
    int w = pb->wd;
    int ht = pb->ht/3;
    int c = 20;
    gfx_text(w/3-10, ht-10, "Welcome to the Ballz Game!");
    gfx_text(w/2-50, ht+c, "How to play:");
    gfx_text(w/11-20, ht+(2*c), "Position your mouse where you want to aim the balls and click once to shoot.");
    gfx_text(w/5-7, ht+(3*c), "Earn a point for each square that you make disappear.");
    gfx_text(w/5-15, ht+(4*c), "The number in each square indicates the amount of times");
    gfx_text(w/5-5, ht+(5*c), "the balls must hit the square to make it disappear.");
    gfx_text(w/7+20, ht+(6*c), "You start with one ball but as you earn more points,");
    gfx_text(w/5+20, ht+(7*c), "you add that many balls to your inventory.");
    gfx_text(w/7-10, ht+(8*c), "Wait until all the balls have returned before clicking again.");
    gfx_text(w/5+10, ht+(9*c), "Don't let any of the squares hit the bottom");
    gfx_text(w/3-5, ht+(10*c), "or else you end the game!");
    gfx_text(w/4-10, ht+(11*c), "Press 'q' after any turn to quit the game.");
    gfx_text(w/3+10, ht+(12*c)+10, "Press 'p' to play!");
}

// initializes the game before the user starts playing
void start(Board *pb, int r, Square s[], Ball b[]) {
    gfx_clear();
    // resets all the board attributes to the start in case the user restarted the game
    int sz = pb->sqsz;
    pb->ns = 0;
    pb->nb = 1;
    pb->score = 0;
    pb->start = 1;

    int mrgn = 3;
    int nrow = (pb->ht - pb->buffer)/sz - mrgn; // number of rows of squares
    int max = pb->wd/sz; // max num of squares in each row
    int min = 1; // min num of squares in each row

    // adds the starting squares to the array to be added to the board
    for(int i = 0; i < nrow; i++) {
        int ncol = rand() % (max-min + 1) + min; // randomizes the number of squares in each row
        int dx = max/ncol;
        int start = rand() % ((dx-1) + 1); // randomizes the index that the squares start at
        for(int j = start; j < max; j+=dx) {
            s[pb->ns].x = j*sz + (sz/2);
            s[pb->ns].y = i*sz + (sz/2) + pb->buffer + 10;
            s[pb->ns].sz = sz - 10;
            s[pb->ns].hits = 1; 
            pb->ns++; 
        }
    }

    dispSquares(pb, s);

    // creates the first ball placed at the bottom of the window
    b[0].x = pb->wd/2;
    b[0].y = pb->ht-10;
    b[0].r = r;
    gfx_color(255,255,255);
    gfx_circle(b[0].x, b[0].y, b[0].r);

    gfx_flush();
}

// plays each round after the user clicks to aim and shoot the balls
int play(Board *pb, Square s[], Ball b[]) {
    //calculates the dx and dy values based on the mouse's location when the user clicked
    int x = gfx_xpos() - b[0].x;
    int y = b[0].y - gfx_ypos();
    float a = atan2(-y,x);
    float dx = 5*cos(a);
    float dy = 5*sin(a);
    if(abs(dy) < 0.001) dy -= 0.001; // stops the balls from getting stuck
    
    // initializes all the balls at the bottom of the screen with the new dx and dy values
    for(int i = 0; i < pb->nb; i++) {
        b[i].x = pb->wd/2;
        b[i].y = pb->ht-10;
        b[i].dx = dx;
        b[i].dy = dy;
        b[i].r = 5;
    }

    int done = 0; // indicator that all the balls have reached the bottom of the screen again and the round has ended
    while(!done) done = moveBalls(pb, pb->nb, s, b);
    lower(pb, s, b); 

    pb->nb = pb->score + 1; // adds new balls equal to the player's current score (plus the initial ball)

    if(checkEnd(pb, s)) return 1; // checks if the loser has lost the game and indicates if so

    return 0;
}

// moves the balls across the screen using dx and dy
int moveBalls(Board *pb, int nb, Square s[], Ball b[]) {
    // variables used when the balls are first launched so there is space between each ball
    static int i = 1; // makes it so that a ball isn't launched until after the one before it has moved
    static int count = 0; // makes it so that there is a small distance between each ball

    gfx_clear();
    dispSquares(pb, s);
    
    // moves each ball (either all the balls or the balls up to index i)
    for(int j = 0; j < i; j++) {
        if(b[j].dx != 0) {
            if(abs(b[j].dy) < 0.001) b[j].dy -= 0.001; // keeps the balls from getting stuck
            b[j].x += b[j].dx;
            b[j].y += b[j].dy;

            gfx_color(255,255,255);
            gfx_circle(round(b[j].x), round(b[j].y), b[j].r);
            bounce(j, pb, b); // checks if ball hits the walls
            collide(j, pb, s, b); // checks if ball hit a square
        } 
    }
    count++;
    if(count>5 && i < nb) {count=0; i++; } // continues incrementing i until it equals the total number of balls
    // doesn't increment i to include the next ball until the ball before it has moved five times
 
    gfx_flush();
    usleep(10000);

    for(int m = 0; m < nb; m++)
        if(b[m].dx != 0) return 0; // returns 0 to indicate that not all of the balls habe reached the bottom yet so the round is not over 

    // resets the static variables for the next round and returns 1 to indicate the round is over
    count = 0;
    i = 1;
    return 1;
}

// checks if the balls have hit the edges of the board - if so redirects their path
void bounce(int n, Board *pb, Ball b[]) {
    int y = b[n].y, x = b[n].x, r = b[n].r;
    int wd = pb->wd, ht = pb->ht;

    if(y-r <= pb->buffer) { b[n].dy = -b[n].dy; b[n].y = pb->buffer+r; }
    if(x+r > wd) { b[n].dx = -b[n].dx; b[n].x = wd-r; }
    if(x-r < 0) { b[n].dx = -b[n].dx; b[n].x = r; }

    // indicates the ball has hit the bottom so it is done and resets it to the starting position
    if(y+r >= ht) { b[n].dx = 0; b[n].dy = 0; b[n].x = wd/2; b[n].y = ht-10; }
    
}

// checks if the ball has collided with any of the squares
void collide(int n, Board *pb, Square s[], Ball b[]) {
    int count = 0; // counts the number of squares hit to add to the score
    for(int i = 0; i < pb->ns; i++) {
        // only checks squares that haven't been deleted yet
        if(s[i].sz != 0) {
            int x = b[n].x, y = b[n].y, r = b[n].r, mrgn = s[i].sz/2;
            // checks if ball has hit the square at all
            if((y-r <= s[i].y+mrgn && y+r >= s[i].y-mrgn) && (x-r <= s[i].x+mrgn && x+r >= s[i].x-mrgn)) {
                // checks if the ball hit the bottom of the square - redirects dy
                if(abs((y-r)-(s[i].y+mrgn)) < abs((x-r)-(s[i].x+mrgn)) && abs((y-r)-(s[i].y+mrgn)) < abs((x+r)-(s[i].x-mrgn))) b[n].dy = -b[n].dy;
                // checks if the ball hit the top of the square - redirects dy
                else if(abs((y+r)-(s[i].y-mrgn)) < abs((x-r)-(s[i].x+mrgn)) && abs((y+r)-(s[i].y-mrgn)) < abs((x+r)-(s[i].x-mrgn))) b[n].dy = -b[n].dy;
                // or else the ball hit the side of the square - redirects dx
                else b[n].dx = -b[n].dx;
                
                s[i].hits--;
                // checks if the square has no more hits left - if so, deletes the square and increments the count
                if(s[i].hits == 0) { 
                    s[i].sz = 0;
                    count++; }
            }
        }
    }
    // adds the number of squares hit to the total score
    pb->score += count;
}

// lowers all the existing squares on the board and adds a new row of squares
void lower(Board *pb, Square s[], Ball b[]) {
    static int hits = 2; // used to change the number of hits so each new row requires a greater number of hits to be deleted
    if(pb->start) hits = 2; // resets hits to 2 if the player restarted the game
    pb->start = 0; // indicates that hits have been reset
    
    //displays previous board
    gfx_clear();
    dispSquares(pb, s);
    gfx_circle(b[0].x, b[0].y, b[0].r);
    usleep(10000);
    
    //lowers all the existing squares on the board
    for(int i = 0; i < pb->ns; i++)
        if(s[i].sz > 0) s[i].y += s[i].sz + 10;

    int max = pb->wd/pb->sqsz;
    int min = 1;
    int ncol = rand() % (max-min + 1) + min; // randomizes the number of squares in the new row
    int dx = max/ncol;
    int start = rand() % ((dx-1) + 1); // randomizes what index the squares start at
    // adds new squares to the top row of the board
    for(int j = start; j < max; j+=dx) {
            if(pb->ns == 500) { pb->won = 1; break; } // makes sure doesn't go over the square array size
            s[pb->ns].x = j*pb->sqsz + (pb->sqsz/2);
            s[pb->ns].y = pb->sqsz/2 + pb->buffer + 10;
            s[pb->ns].sz = pb->sqsz - 10;
            s[pb->ns].hits = hits;
            pb->ns++;
     }

    hits++; // increments hits so the next new row of squares has a higher hit level

    // displays the new lowered board
    gfx_clear();
    dispSquares(pb, s);
    gfx_circle(b[0].x, b[0].y, b[0].r);
    usleep(10000);
        
}

// check if any of the squares have hit the bottom of the board to end the game
int checkEnd(Board *pb, Square s[]) {
    for(int i = 0; i < pb->ns; i++)
        if(s[i].sz > 0)
            if((s[i].y + s[i].sz/2) >= pb->ht) return 1; // indicates that the game is over

    if(pb->score == 500) return 2; // ends the program before it can go outside the square/ball array size
    return 0;
}

// displays the squares on the board and the score at the top
void dispSquares(Board *pb, Square s[]) {    
    char str[20];
    // displays every sqaure in the array
    for(int i = 0; i < pb->ns; i++)
        // only displays squares that haven't been deleted
        if(s[i].sz > 0) {
            int changer = 0;
            int changeb = 20*(s[i].hits-1);
            int changeg = 255 - changeb;
            if(changeb >= 255) {  
                changeg = 0;
                changer = 20*(s[i].hits-14); 
                changeb = 255 - changer;
                if(changeb <= 0) { changeb = 0; changer = 255; }
            }
            gfx_color(changer, changeg, changeb); // displays the square using the color shade corresponding to the number of hits it has left
            sprintf(str, "%d", s[i].hits); // displays the number of hits in the center of the square
            drawSquare(s[i].x, s[i].y, s[i].sz);
            gfx_text(s[i].x, s[i].y, str); }
    gfx_flush;

    // displays the score at the top as well as the line indicating the top of the playing board
    sprintf(str, "Score: %d", pb->score);
    gfx_color(255,255,255);
    gfx_line(0, pb->buffer, pb->wd, pb->buffer);
    gfx_text(pb->wd/2-30, 20, str);

}

// draws a square using the x and y coord. of the center of the square and the side length
void drawSquare(int x, int y, int s)
{
    s = s/2;
    gfx_line(x-s,y-s,x+s,y-s);
    gfx_line(x-s,y-s,x-s,y+s);
    gfx_line(x+s,y-s,x+s,y+s);
    gfx_line(x-s,y+s,x+s,y+s);
}
