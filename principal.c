#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

pthread_mutex_t mutex_contador;
int contador1 = 0;
int contador2 = 0;
int id = 0;

void* procesoSecundario(void* arg) {
    int pipe_fd[2];
    pipe(pipe_fd);

    pid_t pid = fork();
    if (pid == -1) {
        fprintf(stderr, "Error al crear el proceso hijo.\n");
        exit(1);
    }

    if (pid == 0) {
        // Proceso hijo
        close(pipe_fd[1]); // Cerrar el extremo de escritura del pipe en el hijo

        char buffer;
        read(pipe_fd[0], &buffer, sizeof(char));
        if (buffer == 'l') {
            pthread_mutex_lock(&mutex_contador); // Bloquear el mutex antes de actualizar el contador
            contador1++;
            pthread_mutex_unlock(&mutex_contador); // Desbloquear el mutex
        } else if (buffer == 'r') {
            pthread_mutex_lock(&mutex_contador); // Bloquear el mutex antes de actualizar el contador
            contador2++;
            pthread_mutex_unlock(&mutex_contador); // Desbloquear el mutex
        }

        close(pipe_fd[0]);
        exit(0);
    } else {
        // Proceso padre
        close(pipe_fd[0]); // Cerrar el extremo de lectura del pipe en el padre

        while (1) {
            printf("Contador 1: %d\n", contador1);
            printf("Contador 2: %d\n", contador2);

            if (contador1 >= id) {
                printf("¡El ganador es el contador 1!\n");
                break;
            } else if (contador2 >= id) {
                printf("¡El ganador es el contador 2!\n");
                break;
            }

            sleep(1);
        }

        close(pipe_fd[1]);
        wait(NULL);
    }

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Uso: %s <limite>\n", argv[0]);
        return 1;
    }

    id = atoi(argv[1]);

    pthread_mutex_init(&mutex_contador, NULL);

    pthread_t thread;
    pthread_create(&thread, NULL, procesoSecundario, NULL);

    while (1) {
        char caracter;
        printf("Ingrese un caracter (l/r): ");
        scanf(" %c", &caracter);

        if (caracter == 'l' || caracter == 'r') {
            int pipe_fd[2];
            pipe(pipe_fd);

            pid_t pid = fork();
            if (pid == -1) {
                fprintf(stderr, "Error al crear el proceso hijo.\n");
                return 1;
            }

            if (pid == 0) {
                // Proceso hijo
                close(pipe_fd[0]); // Cerrar el extremo de lectura del pipe en el hijo
                write(pipe_fd[1], &caracter, sizeof(char));
                close(pipe_fd[1]);
                exit(0);
            } else {
                // Proceso padre
                close(pipe_fd[1]); // Cerrar el extremo de escritura del pipe en el padre

