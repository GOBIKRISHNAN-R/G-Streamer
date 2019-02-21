    #include <stdio.h>
    #include <unistd.h>
    #include <signal.h>
    #include <stdlib.h>
    #include <sys/wait.h>
    #define max 10

    void handler(int signo);
    void (*hand_pt)(int);

    hand_pt = &handler;
    int ppid, pid[max], i, j, k, n;

    // So I created a pointer to a funcion (hand_pt)
    // and this pointer is going to point to the funcion (handler)

void handler(int signo)
    {
        printf("It worked");
    }

int main(int argc, char **argv)
    {
        signal(SIGUSR1, hand_pt);
        //this process will wait for the signal SIGUSR1
        //and will be handled by hand_pt -> handler 
        pid[0]=fork();
        if(pid[0]==0)
        {
            printf("Sending signal:\n\n");
            ppid=getppid();
            kill(ppid, SIGUSR1);
            exit(0);
        }
        return(0);
    }
