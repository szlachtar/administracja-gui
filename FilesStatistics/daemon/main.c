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
#include <assert.h>

#define _XOPEN_SOURCE 500
#include <ftw.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

#define DAEMON_NAME "file_mon"
#define PID_FILE "/var/run/file_mon.pid"
#define PATH_SIZE 1024

FILE *logfile;

struct event_item
{
    struct inotify_event *event;
    struct event_item *next;
    time_t time;
};

struct wd_path_item_t
{
	int wd; 
	char *file_path;
	struct wd_path_item_t* next;
};  
typedef struct wd_path_item_t   wd_path_item;
//head of the list 
wd_path_item* wd_path_list=NULL; 

wd_path_item* create_wd_path_item(int wd, const char* path)
{
	wd_path_item*  it=(wd_path_item* )malloc(sizeof(wd_path_item));
	it->wd = wd; 
	int len = (strlen(path)+2);
	it->file_path = malloc(len*sizeof(char));
	memset(it->file_path,0,len*sizeof(char));
	strncpy(it->file_path, path,len-1);
	it->next = NULL; 
	return it;
}

void add_to_wd_path_list(wd_path_item* it)
{
	it->next = wd_path_list;
	wd_path_list = it;  
} 

//naive search
const char* find_filepath_by_wd(int wd) 
{ 
	const char *res=NULL;
	wd_path_item* it= wd_path_list;
	while(it) {
		if( it->wd == wd ) res = it->file_path;
		//printf("%s \n",it->file_path);
		it =it->next;
		 
	} 
	return res; 
} 


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

	const char* file_path = find_filepath_by_wd(event->wd);
	
	// sometimes wd == 0 i dont know why
	//assert(file_path); 
	if(file_path != 0 ){
		
		char* p=NULL; 
		if(!(event->mask & IN_ISDIR)){
			int l = event->len+strlen(file_path)+2;
			p = (char*)malloc(l);
			memset(p,0,sizeof(char)*l);
			strncat(p, file_path,strlen(file_path));
			if( event->len){
				strncat(p, "/",1);  	
				strncat(p, event->name, event->len);
			}
			file_path = p;
		}
    /* analaze type of event and log it to stats.dat */
		if(event->mask & IN_CREATE)
			fprintf (logfile,"%s %c C %lu\n", file_path, mark, event_item->time);
		if(event->mask & IN_ACCESS)
			fprintf (logfile,"%s %c A %lu\n", file_path, mark, event_item->time);
		if(event->mask & IN_OPEN)
			fprintf (logfile,"%s %c O %lu\n", file_path, mark, event_item->time);
		if(event->mask & IN_MODIFY)
			fprintf (logfile,"%s %c M %lu\n", file_path, mark, event_item->time);
		if(event->mask & IN_DELETE)
			fprintf (logfile,"%s %c D %lu\n", file_path, mark, event_item->time);
			
		if(!(event->mask & IN_ISDIR)) free(p);
	}
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

int has_wildcard(const char* path)
{
	char* p = (char*)path; 
	while(*p!=0 && *p!='*')p++;  
	return *p!=0; 	 
} 


void add_file(int fd, int flags, const char* path_str)
{ 
	// prevents infinite loop
	if(strcmp(path_str,"/var/lib/file_mon/stats.dat") == 0) return;  
	
	struct stat file_info;
	if(lstat(path_str,&file_info)==0){
		if ( S_ISDIR(file_info.st_mode) || S_ISREG(file_info.st_mode)){
			//printf("%s matched \n", path_str);
			int wd  = inotify_add_watch(fd,path_str, flags);
			if( wd< 0){ syslog(LOG_ERR,"cannot add_watch on %s", path_str);}
			else { add_to_wd_path_list(create_wd_path_item(wd,path_str)); } 
		}else {
			//printf("WARN: %s is not a file or directory skipping \n",path_str);
		}
	} else {
		//printf("WARN: %s does not exist skipping\n", path_str);
	}
} 
// global 
// there is no other way to pass it to add_if_correct which is invoked by nftw function
static struct {
	int fd; 
	int flags;
	regex_t pattern;
	//char pattern[PATH_SIZE*2];
} params; 

int add_if_correct(const char* path, const struct stat* st, int typeflag) 
{ 
	assert(strlen(path)<PATH_SIZE); 
	int res = regexec(&params.pattern, path, 0, NULL, 0);
	// st struct could be passed to prevent stating file twice 
	if(res != REG_NOMATCH) add_file(params.fd,params.flags,path);
	 
	return 0;
} 

// extract dir to traverse ex. /etc/*/*.conf  -> /etc/
void wildcard_toplevel_dir(const char* w_path, char* out)
{
	int ce  =0; 
	char* p= (char*)w_path;
	while(*p && *p != '*' ){
		if(*p == '/') ce = (int)(p-w_path);
		p++; 
	}
	strncpy(out,w_path, ce+1);
}

void wildcard_pattern_to_regex(const char *wildcard, regex_t *rt) 
{ 
	char pattern[PATH_SIZE*3];
	char *pin = (char*) wildcard; 
	char *pout = (char*) pattern; 
	memset(pattern, 0, PATH_SIZE*3*sizeof(char));
	*pout = '^';pout++;
	
	
	while(*pin){
		//escape dot (ex. file extension)
		if(*pin == '.' ){
			*pout = '\\';
			pout++; 
			*pout = '.';
			
		} else if(*pin == '*') {
			//double asteristk -> greedy - simply escape it
			if( *(pin+1) == '*') {
				*pout = '.';
				pout++; 
				*pout = '*';
				pin++;
			//non greedy
			}else { 
				/* didnt worked so using [^>]*
				 * *pout = '.';
				pout++; 
				*pout = '*';
				pout++;
				*pout = '?';*/
				
				*pout = '['; pout++;
				*pout = '^'; pout++;
				*pout = '/'; pout++;
				*pout = ']'; pout++;
				*pout = '*';
			}  
			
		}else {
			*pout = *pin;
		} 
		
		pin++;
		pout++;
	} 
	*pout = '$';
		
	if (regcomp(rt, pattern, REG_EXTENDED)) {
    	fprintf(stderr," bad pattern:");
    	exit(1);
  	}
  	
 // 	printf("final regex %s\n",pattern); 
}

// match all directories and add it to inotify
// returns number of files
void wildcard_expand(int fd, int fl, const char* path) 
{
	assert(strlen(path)<PATH_SIZE); 
	char dirpath[PATH_SIZE];
	params.fd = fd; 
	params.flags = fl;
		
	memset(dirpath,0,PATH_SIZE*sizeof(char));	
	
	wildcard_pattern_to_regex(path,&params.pattern);
	wildcard_toplevel_dir(path,dirpath);
	
	ftw(dirpath,add_if_correct, 100);
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
  	const char *pattern = "^([CAOMD]+)[ \\t]+([a-zA-Z/\\.-\\*]+)$";


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

				if(has_wildcard(path_str)) wildcard_expand(fd,flags,path_str);
				else add_file(fd,flags,path_str);
				
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
