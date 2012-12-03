#ifndef __SENSORS__
#define __SENSORS__

struct data_feed_t
{
	char * date;
	char * what;
};

struct measurement_object_t
{
	char * id;
	char * sensor;
	char * measure;
	char * data_type;
	struct data_feed_t * data_feed;
	int df_len;
};

struct measurement_object_t get_measurement(void);
typedef struct measurement_object_t (*sensor_pointer)(void);

#endif
