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
