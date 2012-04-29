#include <stdlib.h>

#include <sys/inotify.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <syslog.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )


#define DAEMON_NAME "file_mon"
#define PID_FILE "/var/run/file_mon.pid"

FILE *logfile;


void daemonize_process()
{
	pid_t pid, sid;
	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}

	if (pid > 0) {
		printf("Filemon started as a daemon pid: %d \n",pid);
		exit(EXIT_SUCCESS);
	}

	umask(0);
	sid = setsid();

	if(sid < 0){
		exit(EXIT_FAILURE);
	}

	if ((chdir("/")) < 0) {
		exit(EXIT_FAILURE);
	}


	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

/*	FILE *fp = fopen(PID_FILE,"w");
	if (!fp)
	{
		// TODO Handle error
	}

	fprintf(fp,"%d\n",getpid());
	fclose(fp);*/
}

void print_help()
{
	puts("Syntax: filemon [options]");
	puts("Options:");
	puts("  -h display this help");
	puts("  -D run program in daemon mode");

}


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

int main(int argc, char** argv)
{
	int c,daemonize = 0;
    while( (c = getopt(argc, argv, "nD|help")) != -1) {
        switch(c){
            case 'h':
                print_help();
                exit(0);
                break;
            case 'D':
                daemonize = 1;
                break;
            default:
                //PrintUsage(argc, argv);
                exit(0);
                break;
        }
    }

	if(daemonize) daemonize_process();


    int fd,i;
    ssize_t size;
    struct inotify_event *event;
    char buffer[EVENT_BUF_LEN];

    /* initialize log, you need to create file_mon catalogue first! */
    logfile = fopen("/var/lib/file_mon/stats.dat", "w");
    if(logfile < 0) {
        syslog(LOG_ERR,"cannot open /var/lib/file_mon/stats.dat for writing");
        exit(1);
    }


    fd = inotify_init();

    /* monitor home dir for open, create, modify and delete */
    if(inotify_add_watch(fd,"/tmp", IN_ACCESS | IN_CREATE | IN_OPEN | IN_MODIFY | IN_DELETE) < 0){
        syslog(LOG_ERR,"cannot add_watch");
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
