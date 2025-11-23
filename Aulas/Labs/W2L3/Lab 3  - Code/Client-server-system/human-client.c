#include <ncurses.h>
#include "comm-fifo.h"
#include <ctype.h>
#include <zmq.h>


void initialize_screen(){
	initscr();			/* Start curses mode 		*/
	cbreak();				/* Line buffering disabled	*/
	keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
	noecho();			/* Don't echo() while we do getch */
}



int main(){
    // -----------------------------
    //  ZeroMQ INIT (PUSH socket)
    // -----------------------------
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_PUSH);

    // Cliente conecta ao servidor
    zmq_connect(socket, "tcp://localhost:5555");


    char ch;
    do{
        printf("what is your character(a..z)?: ");
        ch = getchar();
        ch = tolower(ch);  
    }while(!isalpha(ch));

    // ALTERAÇÂO PARA ZEROMQ
    // send_connection_message(fd, ch);
    zmq_connect(socket, "tcp://localhost:5555");

    initialize_screen();
    int n = 0;


    
    int key;
    direction_t direction;
    do
    {
    	key = getch();		
        n++;
        switch (key)
        {
        case KEY_LEFT:
            mvprintw(0,0,"%d Left arrow is pressed", n);
            direction = LEFT;
            break;
        case KEY_RIGHT:
            mvprintw(0,0,"%d Right arrow is pressed", n);
            direction = RIGHT;
            break;
        case KEY_DOWN:
            mvprintw(0,0,"%d Down arrow is pressed", n);
            direction = DOWN;
            break;
        case KEY_UP:
            mvprintw(0,0,"%d :Up arrow is pressed", n);
            direction = UP;
            break;
        default:
            key = 'x'; 
            break;
        }


        if (key != 'x'){
            // send_movement_message(fd, ch, direction);
            zmq_send(socket, msg, strlen(msg), 0);

        }
        refresh();			/* Print it on to the real screen */
    }while(key != 27);
    
    
  	endwin();			/* End curses mode		  */

	return 0;
}