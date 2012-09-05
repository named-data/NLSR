#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
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
	char *timestamp = (char *)malloc(sizeof(char) * 16);
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
	char *timestamp = (char *)malloc(sizeof(char) * 16);
	time_t gtime;
	gtime=time(NULL);
	struct tm *tm;
	tm=gmtime(&gtime);
  
	sprintf(timestamp, "%04d%02d%02d%02d%02d%02d", tm->tm_year+1900, tm->tm_mon+1, 
		tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return timestamp;
}


char * 
nth_named_component(const char *name_prefix, int n)
{

	int i;
	char * seps="/";
	char *rem=NULL;
	char *component;

	char *prefix=(char *)malloc(strlen(name_prefix)+1);
	memcpy(prefix,name_prefix,strlen(name_prefix)+1);

	component=strtok_r(prefix,seps,&rem);

	for(i=1;i<n;i++)
		component=strtok_r(NULL,seps,&rem);

	return component;

}

long int 
get_current_time_sec(void)
{
	struct timeval now;
	gettimeofday(&now,NULL);
	return now.tv_sec;
}


long int 
get_current_time_microsec(void)
{
	struct timeval now; 
	gettimeofday(&now, NULL);
	long int microSec=1000000*now.tv_sec+now.tv_usec;
	return microSec;

}

char *
get_current_timestamp_micro(void)
{
	struct timeval now; 
	gettimeofday(&now, NULL);

	char *microSec=(char *)malloc(20);
	sprintf(microSec,"%ld%06ld",now.tv_sec,(long int)now.tv_usec);
	microSec[strlen(microSec)]='\0';

	return microSec;

}

