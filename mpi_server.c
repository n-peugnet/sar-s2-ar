#include "mpi_server.h"

#define SIZE 128

static server the_server;

void start_server(void (*callback)(int tag, int source)) {
    pthread_mutex_init(&the_server.mutex, NULL);
    the_server.callback = callback;
    if (pthread_create(&the_server.listener, NULL, callback)) {
        printf("fail");
    }
} /*initialiser le serveur*/
void destroy_server() {/*detruire le serveur*/
    free(&the_server);
}

pthread_mutex_t* getMutex() {/*renvoyer une reference sur le mutex*/
    return &the_server.mutex;
}
