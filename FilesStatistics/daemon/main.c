#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>
#include <syslog.h>
#include <time.h>
#include <regex.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

#define DAEMON_NAME "file_mon"
#define PID_FILE "/var/run/file_mon.pid"

FILE *logfile;

struct event_item
{
    struct inotify_event *event;
    struct event_item *next;
    time_t time;
};

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

/*
  FILE *fp = fopen(PID_FILE,"w");
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
void analyze(struct event_item *event_item)
{
    struct event_item *next;
    struct inotify_event *event;
    char mark=' ';

    if(!event_item)
        return;

    next = event_item->next;
    event = event_item->event;

    /* directory? */
    if(event->mask & IN_ISDIR)
        mark='D';
    else
        mark='F';

    /* analaze type of event and log it to stats.dat */
    if(event->mask & IN_CREATE)
        fprintf (logfile,"%s %c C %lu\n", event->name, mark, event_item->time);
    if(event->mask & IN_ACCESS)
        fprintf (logfile,"%s %c A %lu\n", event->name, mark, event_item->time);
    if(event->mask & IN_OPEN)
        fprintf (logfile,"%s %c O %lu\n", event->name, mark, event_item->time);
    if(event->mask & IN_MODIFY)
        fprintf (logfile,"%s %c M %lu\n", event->name, mark, event_item->time);
    if(event->mask & IN_DELETE)
        fprintf (logfile,"%s %c D %lu\n", event->name, mark, event_item->time);

	free(event);
	free(event_item);
    analyze(next);
}

int char_to_flag(char c)
{
	switch(c)
	{
		case 'a':
		case 'A':
			return IN_ACCESS;
		case 'c':
		case 'C':
			return IN_CREATE;
		case 'o':
		case 'O':
			return IN_OPEN;
		case 'm':
		case 'M':
			return IN_MODIFY;
		case 'd':
		case 'D':
			return IN_DELETE;
		default:
			return 0;
	}

}

void load_config(int fd)
{
	char *buffer=0;
	char *pch=0;
	// TODO add it  to etc dir
	int config_fd = open("file_mon.conf",O_RDONLY);
	if( config_fd < 0 )
	{
		syslog(LOG_ERR,"cannot open config file");
		exit(1);
	}

	int size = lseek(config_fd,0, SEEK_END);
	lseek(config_fd,0,SEEK_SET);
	buffer = malloc(sizeof(char)*size);
	read(config_fd,buffer,size);

	pch = strtok(buffer,"\n");

	regex_t st_comment,st_pattern;
	const char *comment_pattern = "^#.*$";
  	const char *pattern = "^([CAOMD]+)[ \\t]+([a-zA-Z/\.-]+)$";


	if (regcomp(&st_pattern, pattern, REG_EXTENDED)) {
    	fprintf(stderr," bad pattern:");
    	exit(1);
  	}

	if (regcomp(&st_comment, comment_pattern, REG_EXTENDED)) {
    	fprintf(stderr," bad pattern:");
    	exit(1);
  	}

  	int line = 1;
	while(pch!=NULL)
	{

		int comm = regexec(&st_comment, pch, 0, NULL, 0);

		if(comm == REG_NOMATCH)//if not comment
		{
			regmatch_t matchptr[3];
 			int res = regexec(&st_pattern, pch, 3, matchptr, 0);
			if(res == REG_NOMATCH) printf("syntax error in line %d \n",line);
			else
			{
				int flags=0;
				//MASK
				char *s= pch+matchptr[1].rm_so;
				for(;s!=pch+matchptr[1].rm_eo;++s){
					flags|=char_to_flag(*s);
				}


				int path_len = (matchptr[2].rm_eo - matchptr[2].rm_so);
				char* path_str = malloc(sizeof(char)* (path_len+1) );
				memset(path_str, 0 , path_len+1);
				strncpy(path_str,pch+ matchptr[2].rm_so, path_len);

				struct stat file_info;
				if(lstat(path_str,&file_info)==0){
					if ( S_ISDIR(file_info.st_mode) || S_ISREG(file_info.st_mode)){
						if(inotify_add_watch(fd,path_str, flags) < 0){ syslog(LOG_ERR,"cannot add_watch on %s", path_str);}
					}else {
						printf("WARN: %s is not a file or directory skipping \n",path_str);
					}
				} else {
					printf("WARN: %s does not exist skipping\n", path_str);
				}
				free(path_str);
			}
		}

        line++;
		pch = strtok(NULL,"\n");
	}
    free(buffer);
}

int main(int argc, char** argv)
{
    int c,daemonize = 0;
    int fd,i;
    ssize_t size;
    struct inotify_event *event;
    char buffer[EVENT_BUF_LEN];
    struct event_item *head;
    struct event_item *last;
    struct event_item *tmp;
    time_t time_start;
    size_t event_size;

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
            print_help();
            exit(0);
            break;
        }
    }

    if(daemonize) daemonize_process();

    /* initialize log, you need to create file_mon catalogue first! */
    logfile = fopen("/var/lib/file_mon/stats.dat", "w");
    if(logfile < 0) {
        syslog(LOG_ERR,"cannot open /var/lib/file_mon/stats.dat for writing");
        exit(1);
    }

    fd = inotify_init();
    load_config(fd);

    head = NULL;
    last = NULL;

    time_start = time(NULL);
    int total_size = 0;

    while(true){
        size = read(fd,&buffer,EVENT_BUF_LEN);

        for(i = 0;i<size;){
            event = (struct inotify_event *) buffer + i;
            event_size = EVENT_SIZE+event->len;

            tmp = malloc(sizeof(struct event_item));
            tmp->time = time(NULL);
            tmp->next = NULL;
            tmp->event = malloc(event_size);
            memcpy(tmp->event,event,event_size);

            if(last==NULL){
                head = tmp;
                last = tmp;
            }else{
                last->next = tmp;
                last = tmp;
            }
            i += EVENT_SIZE+tmp->event->len;
            total_size += 1;
        }

        if(head && (time(NULL) - time_start)  > 5){
            analyze(head);
            time_start = time(NULL);
            total_size = 0;
            head = NULL;
            last = NULL;
			fflush(logfile);
        }
    }
    return 0;
}
