typedef char direction_t;

#define UP 'u'
#define DOWN 'd'
#define LEFT 'l'
#define RIGHT 'r'

#define FIFO_NAME "/tmp/fifo_snail"
int create_client_channel();
int create_server_channel();
void send_connection_message(int fd, char ch);
void send_movement_message(int fd, char ch, char  direction);
void read_message (int fd, char * message_type, char * c, direction_t *d );

