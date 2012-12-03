#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "sensors.h"

sensor_pointer * sensors;
unsigned int sensor_count;
void ** handle;
short int * sensor_states;

char monitor_address[255];

struct sensor_info_t * sensors_info;

volatile unsigned int * sleep_times;

/* Prints help */
inline void print_usage(const char * pname) 
{
	printf("usage: %s sensor1 sensor2 ...\n", pname);
}

void send_post(const char * json)
{
	int sockfd = 0;
	char buffer[4096];
	struct sockaddr_in serv_addr;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Network error");
		return;
	}
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(monitor_address);
	serv_addr.sin_port = htons(80);
	
	sprintf(buffer,
         	"POST %s HTTP/1.0\r\n"
         	"Host: %s\r\n"     
         	"Content-type: application/x-www-form-urlencoded\r\n"
         	"Content-length: %d\r\n\r\n"
         	"%s\r\n", "sensor", monitor_address, (unsigned int)strlen(json), json); 
	
	if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("Network error");
		return;
	}
	
	if(send(sockfd, buffer, sizeof(buffer), 0) < 0)
	{
		perror("Network error");
		return;
	}
	
	close(sockfd);
}

void register_sensor(struct sensor_info_t sinfo)
{
	char * result = (char *) malloc(sizeof(char) * 4096);
	
	printf("Registering sensor\n");
	
	strcpy(result, "{");
	
	/* ID */
	strcat(result, "\"id\":\"");
	strcat(result, sinfo.id);
	strcat(result, "\",");
	/* NAME */
	strcat(result, "\"name\":\"");
	strcat(result, sinfo.name);
	strcat(result, "\",");
	/* HREF */
	strcat(result, "\"href\":\"");
	strcat(result, sinfo.href);
	strcat(result, "\",");
	/* MEASURE */
	strcat(result, "\"measure\":\"");
	strcat(result, sinfo.measure);
	strcat(result, "\",");
	/* DATA TYPE */
	strcat(result, "\"dataType\":\"");
	strcat(result, sinfo.data_type);
	strcat(result, "\",");
	/* FREQUENCY */
	strcat(result, "\"frequency\":\"");
	strcat(result, "0");
	strcat(result, "\",");
	/* RESOURCE */
	strcat(result, "\"resource\":\"");
	strcat(result, sinfo.resource);
	strcat(result, "\",");
	/* TYPE */
	strcat(result, "\"type\":\"");
	strcat(result, "null");
	strcat(result, "\"");
	
	strcat(result, "}");
	
	send_post(result);
}

/* prints prompt help */
inline void print_help()
{
	printf("list			prints list of sensors\n");
	printf("toggle 			changes sensor state to DISABLED or ENABLED\n");
	printf("period			sets time beetwen measures\n");
	printf("help			print this\n");
	printf("exit			exits\n");
	printf("quit			quits\n");
}

char * to_json(const struct measurement_object_t r)
{
	char * result = (char *) malloc(sizeof(char) * 4096);
	int i;
	
	strcpy(result, "{");
	strcat(result, "\"id\":");
	
	strcat(result, "\"");
	strcat(result, r.id);
	strcat(result, "\"");
	
	strcat(result, ", \"sensor\":");
	
	strcat(result, "\"");
	strcat(result, r.sensor);
	strcat(result, "\"");
	
	strcat(result, ", \"measure\":");
	
	strcat(result, "\"");
	strcat(result, r.measure);
	strcat(result, "\"");
	
	strcat(result, ", \"dataType\":");
	
	strcat(result, "\"");
	strcat(result, r.data_type);
	strcat(result, "\"");	
	
	strcat(result, ", \"data\":[");
	
	for(i = 0 ; i < r.df_len ; i++)
	{
		strcat(result, "{");
		
		strcat(result, "\"date\":");
	
		strcat(result, "\"");
		strcat(result, r.data_feed[i].date);
		strcat(result, "\"");
		
		strcat(result, ", \"what\":");
	
		strcat(result, "\"");
		strcat(result, r.data_feed[i].what);
		strcat(result, "\"");
		
		strcat(result, "}");
		
		if(i != r.df_len - 1)
			strcat(result, ",");
	}
	
	strcat(result, "]");
	strcat(result, "}");
	
	return result;
}

/* Thread for running sensors */
void * runner(void * unused)
{
	int i;
	int oldstate, oldtype;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldtype);
	while(1)
	{
		for(i = 0 ; i < sensor_count ; i++)
		{
			if(!sensor_states[i])
				continue;
				
			struct measurement_object_t result = (sensors[i])();
			
			result.id = sensors_info[i].id;
			result.sensor = sensors_info[i].href;
			
			/* log */
			FILE * fp = fopen("results", "a");
			fprintf(fp, "%s\n", to_json(result));
			fclose(fp);
			
			send_post(to_json(result));
			
			sleep(sleep_times[i]);
			pthread_testcancel();
		}
	}
}

/* Main */
int main(int argc, char ** argv)
{
	char * pname = argv[0];			/* Program name */
	int i;					/* For loops */
	pthread_t rthread;			/* Running thread */
	sensor_count = argc-1;			/* Sensor count */

	if(argc == 1)
	{
		print_usage(pname);
		return EXIT_SUCCESS;
	}
	
	/* Read settings */
	FILE * sfile = fopen("settings", "r");
	if(sfile == NULL)
	{
		printf("No settings file!\n");
		return EXIT_SUCCESS;
	}
	
	fscanf(sfile, "%s", monitor_address);
	
	fclose(sfile);

	printf("Sensors: %d\n", sensor_count);
	printf("Allocating sensor memory ...");
	sensors = (sensor_pointer *)malloc(sizeof(sensor_pointer) * (sensor_count));
	sensors_info = (struct sensor_info_t *)malloc(sizeof(struct sensor_info_t) * sensor_count);
	sensor_states = (short int *)malloc(sizeof(short int) * sensor_count);
	sleep_times = (unsigned int *)malloc(sizeof(unsigned int) * sensor_count);
	printf("OK\n");
	
	printf("Allocating handles memory ...");
	handle = (void **)malloc(sizeof(void *) * (sensor_count));
	printf("OK\n");

	/* Reading cmd args */
	for(i = 1 ; i < argc ; i++)
	{
		/* Loading sensors */
		char * lib_name = argv[i];
		printf("Loading sensor: %s(%d) ...", lib_name, i-1);
		handle[i-1] = dlopen(lib_name, RTLD_LAZY);
		if(handle[i-1] == NULL)
		{
			printf("FAILED!\n");
			return EXIT_FAILURE;
		}

		sensors[i-1] = dlsym(handle[i-1], "get_measurement");
		
		sensors_info[i-1].name = lib_name;
		sensors_info[i-1].id = "-1";
		sensors_info[i-1].href = "wtfgoeshere";
		sensors_info[i-1].resource = "wtfgoeshere";
		sensors_info[i-1].frequency = 0;
		
		sensor_states[i-1] = 1;
		sleep_times[i-1] = 5;
		
		if(sensors[i-1] == NULL)
		{
			printf("FAILED!\n");
			return EXIT_FAILURE;
		}

		printf("OK\n");
	}
	
	/* Testing */
	printf("Testing sensors:\n");
	for(i = 0 ; i < sensor_count ; i++)
	{
		printf("[%d]\n", i);
		if(sensors[i] == NULL)
		{
			printf("\nFAILED\n");
			return EXIT_FAILURE;
		}
		
		struct measurement_object_t result = (sensors[i])();
		
		/* Fill sensor info */
		sensors_info[i].measure = result.measure;
		sensors_info[i].data_type = result.data_type;
		
		printf("ID        : %s\n", result.id);
		printf("Sensor    : %s\n", result.sensor);
		printf("Measure   : %s\n", result.measure);
		printf("Data type : %s\n", result.data_type);
		printf("Data feed:\n");
		
		int j;
		for(j = 0 ; j < result.df_len ; j++)
		{
			printf(" * date : %s\n", result.data_feed[j].date);
			printf(" * what : %s\n", result.data_feed[j].what);
			printf("----\n");
		}
		
		printf("OK\n\n");
	} 
	
	/* Registering sensors */
	for(i = 0 ; i < sensor_count ; i++)
	{
		register_sensor(sensors_info[i]);
	}
	
	/* Starting thread */
	printf("Starting runner thread ...");
	if(pthread_create(&rthread, NULL, &runner, NULL) != 0)
	{
		printf("FAILED!\n");
		return EXIT_FAILURE;
	}
	printf("OK\n");
	
	/* Main loop - prompt */
	while(1) 
	{
		char cmd[255];
		printf(">> ");
		scanf("%s", cmd);
		
		if(strcmp(cmd, "exit") == 0)
			break;
		else
		if(strcmp(cmd, "quit") == 0)
			break;
		else
		if(strcmp(cmd, "list") == 0)
		{
			for(i = 0 ; i < sensor_count ; i++)
			{
				printf("Nr \t\tName\t\t\t\tPeriod\t\tState\n");
				printf("%d)\t\t%s\t\t%ds\t\t", i, sensors_info[i].name, sleep_times[i]);
				if(sensor_states[i] == 1)
					printf("ENABLED\n");
				else
					printf("DISABLED\n");
			}
		}
		else
		if(strcmp(cmd, "toggle") == 0)
		{
			int nr;
			printf(" -> number? : ");
			scanf("%s", cmd);
			nr = atoi(cmd);
			
			if(nr < sensor_count)
				sensor_states[nr] = !sensor_states[nr];
		}
		else
		if(strcmp(cmd, "period") == 0)
		{
			int nr;
			printf(" -> number? : ");
			scanf("%s", cmd);
			nr = atoi(cmd);
			
			if(nr < sensor_count)
			{
				int stime;
				printf(" -> time[s]? : ");
				scanf("%s", cmd);
				stime = atoi(cmd);
				
				sleep_times[nr] = stime;
			}
		}
	}
	
	printf("Cancelling runner thread ...");
	pthread_cancel(rthread);
	printf("OK\n");
	
	printf("Closing sensors\n");
	/* Closing sensor handlers */
	for(i = 0 ; i < sensor_count ; i++)
	{
		printf(" * closing sensor %d ...", i);
		dlclose(handle[i]);
		printf("OK\n");
	}
	
	free(sensors);
	free(sensors_info);
	free(sensor_states);
	free((void *)sleep_times);

	return EXIT_SUCCESS;
}
