#include <stdlib.h>
#include <sys/inotify.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )


FILE *logfile;


/* analaze event and print info to stats.dat */
void analyze(struct inotify_event *event)
{
    char mark=' ';

    /* directory? */
    if(event->mask & IN_ISDIR)
        mark='D';
    else
        mark='F';

    /* analaze type of event and log it to stats.dat */
    if(event->mask & IN_CREATE)
        fprintf (logfile,"%s %c C %lu\n",event->name,mark,time(NULL));
    if(event->mask & IN_ACCESS)
        fprintf (logfile,"%s %c A %lu\n",event->name,mark,time(NULL));
    if(event->mask & IN_OPEN)
        fprintf (logfile,"%s %c O %lu\n",event->name,mark,time(NULL));
    if(event->mask & IN_MODIFY)
        fprintf (logfile,"%s %c M %lu\n",event->name,mark,time(NULL));
    if(event->mask & IN_DELETE)
        fprintf (logfile,"%s %c D %lu\n",event->name,mark,time(NULL));
}

int main()
{
    int fd,i;
    ssize_t size;
    struct inotify_event *event;
    char buffer[EVENT_BUF_LEN];

    /* initialize log, you need to create file_mon catalogue first! */
    logfile = fopen("/var/lib/file_mon/stats.dat", "w");
    if(logfile < 0) {
        perror("cannot open /var/lib/file_mon/stats.dat for writing");
        exit(1);
    }


    fd = inotify_init();

    /* monitor home dir for open, create, modify and delete */
    if(inotify_add_watch(fd,"/tmp", IN_ACCESS | IN_CREATE | IN_OPEN | IN_MODIFY | IN_DELETE) < 0){
        perror("cannot add_watch");
        exit(1);
    }

    while(true){
        size = read(fd,&buffer,EVENT_BUF_LEN);

        for(i = 0;i<size;){
            event = (struct inotify_event *) buffer + i;
            analyze(event);
            i += EVENT_SIZE+event->len;
        }

    }
    return 0;
}
