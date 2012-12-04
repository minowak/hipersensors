#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sensors.h"

char* getLoad(){
	int MAX_LEN = 20;
	FILE *fp;
	int status;
	char* res = malloc(MAX_LEN*sizeof(char));
		
	const char cmd[]="w | head -n 1 | sed \"s/.*load average: //\"" ;

	if ((fp = popen(cmd, "r")) == NULL){
		printf("popen error");
		return NULL;
		/* Handle error */;
	}
	fgets(res, MAX_LEN, fp);	
	
	if ((status = pclose(fp))== -1) {
		fprintf(stderr, "pclose error");
		return NULL ;
	} 	
	
	return res;	
	
}


char* getDate(){
	int MAX_LEN = 40;
	FILE *fp;
	int status;
	char* res = malloc(MAX_LEN*sizeof(char));
	
	const char cmd[]="date | sed \"s/ CET//\"";

	if ((fp = popen(cmd, "r")) == NULL){
		printf("popen error");
		return NULL;
		/* Handle error */;
	}
	fgets(res, MAX_LEN, fp);	
	
	if ((status = pclose(fp))== -1) {
		fprintf(stderr, "pclose error");
		return NULL ;
	} 	
	
	return res;	
}


char* getMem(){
	int MAX_LEN = 20;
	FILE *fp;
	int status;
	char* res = malloc(MAX_LEN*sizeof(char));
	
	const char cmd[]="free | awk '{print $3, $2}' | head -n 2 | tail -n 1";

	if ((fp = popen(cmd, "r")) == NULL){
		printf("popen error");
		return NULL;
		/* Handle error */;
	}
	fgets(res, MAX_LEN, fp);	
	
	if ((status = pclose(fp))== -1) {
		fprintf(stderr, "pclose error");
		return NULL ;
	} 	
	
	return res;	
	
}

struct measurement_object_t get_measurement()
{
	struct measurement_object_t result;
	char* load;	
	char* mem ;
	char* date;

	result.id = "123";
	result.sensor = "cpusensor";
	result.measure = "MHz";
	result.data_type = "int";
	
	result.data_feed = (struct data_feed_t *) malloc(sizeof(struct data_feed_t) * 2);
	result.df_len = 2;
		
	date = getDate();
	result.data_feed[0].date = malloc(sizeof(char)*40);	
	strcpy(result.data_feed[0].date, date);
	free(date);
	
	load = getLoad();
	result.data_feed[0].what= malloc(sizeof(char)*20);
	strcpy(result.data_feed[0].what, load);
	free(load);	

	date = getDate();
	result.data_feed[1].date = malloc(sizeof(char)*40);	
	strcpy(result.data_feed[1].date, date);
	free(date);
	
	mem = getLoad();
	result.data_feed[1].what= malloc(sizeof(char)*20);
	strcpy(result.data_feed[1].what, mem);
	free(mem);
			
	return result;
}

