#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include<ctype.h>
#include<stdarg.h>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<errno.h>
#include<netdb.h>
#include <time.h>
#include <assert.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif



#include <ccn/ccn.h>
#include <ccn/uri.h>
#include <ccn/keystore.h>
#include <ccn/signing.h>
#include <ccn/schedule.h>
#include <ccn/hashtb.h>


#include "utility.h"


char * getLocalTimeStamp(void)
{
	char *timestamp = (char *)malloc(sizeof(char) * 20);
	time_t ltime;
	ltime=time(NULL);
	struct tm *tm;
	tm=localtime(&ltime);
  
	sprintf(timestamp, "%04d%02d%02d%02d%02d%02d", tm->tm_year+1900, tm->tm_mon+1, 
		tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return timestamp;
}

char * getGmTimeStamp(void)
{
	char *timestamp = (char *)malloc(sizeof(char) * 20);
	time_t gtime;
	gtime=time(NULL);
	struct tm *tm;
	tm=gmtime(&gtime);
  
	sprintf(timestamp, "%04d%02d%02d%02d%02d%02d", tm->tm_year+1900, tm->tm_mon+1, 
		tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return timestamp;
}


int
get_width_of_number(long int number)
{
	int i=0;
	while(number>0)
	{
		i++;
		number/=10;
	}
	return i;
}

long int 
get_current_time_sec(void)
{
	struct timeval now;
	gettimeofday(&now,NULL);
	return now.tv_sec;
}


void
get_current_timestamp_micro(char * microSec)
{
	struct timeval now; 
	gettimeofday(&now, NULL);
	sprintf(microSec,"%ld%06ld",now.tv_sec,(long int)now.tv_usec);
}

char *
get_current_timestamp_micro_v2()
{
	struct timeval now; 
	gettimeofday(&now, NULL);
	//sprintf(microSec,"%ld%06ld",now.tv_sec,(long int)now.tv_usec);
	char *microSec=(char *)calloc(get_width_of_number(now.tv_sec)+7,sizeof(char));
	sprintf(microSec,"%ld%06ld",now.tv_sec,(long int)now.tv_usec);
	microSec[strlen(microSec)]='\0';	

	return microSec;
}

long int
get_time_diff(const char *time1, const char *time2)
{
	long int diff_secs;

	long int time1_in_sec,	time2_in_sec;

	char *time1_sec=(char *)malloc(strlen(time1)-6+1);
	memset(time1_sec,0,strlen(time1)-6+1);
	memcpy(time1_sec,time1,strlen(time1)-6);

	char *time2_sec=(char *)malloc(strlen(time2)-6+1);
	memset(time2_sec,0,strlen(time2)-6+1);
	memcpy(time2_sec,time2,strlen(time2)-6);

	time1_in_sec=strtol(time1_sec,NULL,10);
	time2_in_sec=strtol(time2_sec,NULL,10);

	diff_secs=time1_in_sec-time2_in_sec;

	free(time1_sec);
	free(time2_sec);

	return diff_secs;
}


void  
startLogging(char *loggingDir)
{
	struct passwd pd;
	struct passwd* pwdptr=&pd;
	struct passwd* tempPwdPtr;
	char *pwdbuffer;
	int  pwdlinelen = 200;
	char *logDir;
	char *logFileName;
	char *ret;
	char *logExt;
	char *defaultLogDir;	
	//int status;
	struct stat st;
	int isLogDirExists=0;
	char *time=getLocalTimeStamp();

	pwdbuffer=(char *)malloc(sizeof(char)*200);
	memset(pwdbuffer,0,200);		
	logDir=(char *)malloc(sizeof(char)*200);
	memset(logDir,0,200);
	logFileName=(char *)malloc(sizeof(char)*200);
	memset(logFileName,0,200);
	logExt=(char *)malloc(sizeof(char)*5);
	memset(logExt,0,5);
	defaultLogDir=(char *)malloc(sizeof(char)*10);
	memset(defaultLogDir,0,10);

	memcpy(logExt,".log",4);
	logExt[4]='\0';
	memcpy(defaultLogDir,"/nlsrLog",9);
	defaultLogDir[9]='\0';

	if(loggingDir!=NULL)
 	{
		if( stat( loggingDir, &st)==0)
		{
			if ( st.st_mode & S_IFDIR )
			{
				if( st.st_mode & S_IWUSR)
				{
					isLogDirExists=1;
					memcpy(logDir,loggingDir,strlen(loggingDir)+1);
				}
				else printf("User do not have write permission to %s \n",loggingDir);
			}
			else printf("Provided path for %s is not a directory!!\n",loggingDir);
    		}
  		else printf("Log directory: %s does not exists\n",loggingDir);
	} 
  
	if(isLogDirExists == 0)
  	{
		if ((getpwuid_r(getuid(),pwdptr,pwdbuffer,pwdlinelen,&tempPwdPtr))!=0)
     			perror("getpwuid_r() error.");
  		else
  		{
			memcpy(logDir,pd.pw_dir,strlen(pd.pw_dir)+1);	
			memcpy(logDir+strlen(logDir),defaultLogDir,strlen(defaultLogDir)+1);	
			if(stat(logDir,&st) != 0)
				mkdir(logDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);	
			//printf("Status: %d\n",status);
		}
	}	
 	memcpy(logFileName,logDir,strlen(logDir)+1);	
	if( logDir[strlen(logDir)-1]!='/')
	{
		memcpy(logFileName+strlen(logFileName),"/",1);
		memcpy(logFileName+strlen(logFileName),"\0",1);	
	}	
	memcpy(logFileName+strlen(logFileName),time,strlen(time)+1);	
	memcpy(logFileName+strlen(logFileName),logExt,strlen(logExt)+1);	
	ret=(char *)malloc(strlen(logFileName)+1);
	memset(ret,0,strlen(logFileName)+1);
       	memcpy(ret,logFileName,strlen(logFileName)+1); 

	setenv("NLSR_LOG_FILE",ret,1);

	free(time);	
	free(logDir);
	free(logFileName);	
	free(pwdbuffer);	
	free(logExt);
	free(defaultLogDir);
	free(ret);	
}


void 
writeLogg(const char *source_file, const char *function, const int line, const char *format, ...)
{
	char *file=getenv("NLSR_LOG_FILE");	
	if (file != NULL)
	{
		FILE *fp = fopen(file, "a");

		if (fp != NULL)
		{            
			struct timeval t;
			gettimeofday(&t, NULL);
			fprintf(fp,"%ld.%06u - %s, %s, %d :",(long)t.tv_sec , (unsigned)t.tv_usec , source_file , function , line);        
			va_list args;
			va_start(args, format);
			vfprintf(fp, format, args);
			fclose(fp);
			va_end(args);	
		}
    	}
}


struct sockaddr_in *
get_ip_from_hostname(char *hostname )
{
 

    struct addrinfo hints, *servinfo, *p;
    int res; 
    struct sockaddr_in * ip;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ( (res = getaddrinfo( hostname , "9696", &hints , &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
        return NULL;
    }
    int i=0;
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        ip = (struct sockaddr_in *) p->ai_addr;
	i++;
   
    }
    freeaddrinfo(servinfo);
    return ip;


}



int 
get_ip_from_hostname_02(char * hostname , char* ip)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;
    if ( (he = gethostbyname( hostname ) ) == NULL)
    {
        herror("gethostbyname");
        return 1;
    }
    addr_list = (struct in_addr **) he->h_addr_list;
    for(i = 0; addr_list[i] != NULL; i++)
    {
        strcpy(ip , inet_ntoa(*addr_list[i]) );
		ip[strlen(ip)]='\0';
        return 0;
    }
    return -1;
}


int 
add_ccn_uri_name(struct ccn_charbuf *res_name, struct ccn_charbuf *add){

	int i, res;
	struct ccn_indexbuf *idx=ccn_indexbuf_create();
	res=ccn_name_split(add,idx);	
	if ( res < 0 ){
		ccn_indexbuf_destroy(&idx);
		return -1;
	}

	const unsigned char *comp_ptr1;
	size_t comp_size;
	for(i=0;i<idx->n-1;i++){
		ccn_name_comp_get(add->buf,idx,i,&comp_ptr1, &comp_size);
		ccn_name_append_str(res_name,(char *)comp_ptr1);
	}
	ccn_indexbuf_destroy(&idx);

	return 0;

}


