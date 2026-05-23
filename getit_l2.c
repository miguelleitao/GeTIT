#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <mosquitto.h>
#include "getit.h"

#include "config.h"

int debug = 10;

struct mosquitto *mosq = NULL;

char hostname[256];

#ifndef NumAluno
#define NumAluno 1234567
#endif



/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	(void)obj;
	//printf("on_connect\n");
    int rc;
    /* Print out the connection result. mosquitto_connack_string() produces an
     * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
     * clients is mosquitto_reason_string().
     */
    if ( debug>2 ) printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
    if ( reason_code != 0 ) {
        /* If the connection fails for any reason, we don't want to keep on
         * retrying in this example, so disconnect. Without this, the client
         * will attempt to reconnect. */
        mosquitto_disconnect(mosq);
    }

    /* Making subscriptions in the on_connect() callback means that if the
     * connection drops and is automatically resumed by the client, then the
     * subscriptions will be recreated when the client reconnects.
     */

	//PGSCE-PRSIEM
	char topic[280];
	snprintf(topic, sizeof topic, "PRSIEM/%d/%s/cmd/#", NumAluno, hostname);
	rc = mosquitto_subscribe(mosq, NULL, topic, 0);
	if ( rc != MOSQ_ERR_SUCCESS ) {
		fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
		// disconnect if we were unable to subscribe
		mosquitto_disconnect(mosq);
	}
	else
	    if ( debug>4 ) printf("Subscrition requested.\n");
	snprintf(topic, sizeof topic, "PRSIEM/%d/all/cmd/#", NumAluno);
	rc = mosquitto_subscribe(mosq, NULL, topic, 0);
	if ( rc != MOSQ_ERR_SUCCESS ) {
		fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
		// disconnect if we were unable to subscribe
		mosquitto_disconnect(mosq);
	}
	else
	    if ( debug>4 ) printf("Subscrition requested.\n");
}


void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{
	(void)mosq;
	(void)userdata;
	
    printf("Recebeu msg:%s %s (%d), retained:%d\n", msg->topic,
            (const char *)msg->payload, msg->payloadlen, msg->retain);
    /*
     * Process retained and new messages
     */

}

/* Callback called when the broker sends a SUBACK in response to a SUBSCRIBE. */
void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	(void)obj;
	(void)mid;
	if (debug) printf("Subscribed.\n");
    int i;
    bool have_subscription = false;

    for( i=0; i<qos_count; i++) {
        if (debug>2) printf("on_subscribe: %d:granted qos = %d\n", i, granted_qos[i]);
        if (granted_qos[i] <= 2) {
            have_subscription = true;
        }
    }
    if ( have_subscription == false ) {
        /* The broker rejected all of our subscriptions, we know we only sent
         * the one SUBSCRIBE, so there is no point remaining connected. */
        fprintf(stderr, "Error: All subscriptions rejected.\n");
        mosquitto_disconnect(mosq);
    }
}

int mqttConnect() {
	mosquitto_lib_init();

    /* Create a new MQTT client instance.
     * id = NULL -> ask the broker to generate a client id for us
     * clean session = true -> the broker should remove old sessions when we connect
     * obj = NULL -> we aren't passing any of our private data for callbacks.
     * 
     * MQTT client is required. Abort if client is not available.
     */
    mosq = mosquitto_new(NULL, true, NULL);
    if ( mosq == NULL ) {
        fprintf(stderr, "Error: Could not allocate Mosquitto MQTT client.\n");
        return 1;
    }

    /* Configure callbacks. This should be done before connecting ideally. */
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_subscribe_callback_set(mosq, on_subscribe);
    mosquitto_message_callback_set(mosq, on_message);
    
    mosquitto_username_pw_set(mosq, MQTT_USERNAME, MQTT_PASSWORD);

    /* Connect to mqttBroker on port 1883, with a keepalive of 60 seconds.
     * This call makes the socket connection only, it does not complete the MQTT
     * CONNECT/CONNACK flow.
     * mosquitto_loop_forever() will processing net traffic.
     */
    if ( debug>5 ) fprintf(stderr, "Connecting to MQTT broker %s:%d\n", mqttBroker, mqttPort);
    int rc = mosquitto_connect(mosq, mqttBroker, mqttPort, 60);
    if(rc != MOSQ_ERR_SUCCESS){
        mosquitto_destroy(mosq);
        fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
        return 1;
    }
    if (debug) fprintf(stderr, "Connected to MQTT broker on %s:%d.\n", mqttBroker, mqttPort);
    return 0;
}


int mqttPublish(const char *topic, const char* msg, int qos, int retain) {
    /* Publish the message
     * mosq - our client instance
     * topic = the topic on which this message will be published
     * msg - the actual payload (0 terminated).
     * qos - publish with MQTT QoS
     * retain = false - do not use the retained message feature for this message
     */
    if (debug>3) printf("mqttPub '%s', '%s'\n", topic, msg);
    int rc = mosquitto_publish(mosq, NULL, topic, strlen(msg), msg, qos, retain);
    if (rc != MOSQ_ERR_SUCCESS) {
        fprintf(stderr, "Error publishing to MQTT broker: %s\n", mosquitto_strerror(rc));
        return false;
    }
    if (debug>3) printf("  mqttPub success '%s', '%s'\n", topic, msg);
    return true;
}

int getit_publishState(getit_state s) {
	char topic[280];
	sprintf(topic, "PRSIEM/%d/%s/state", NumAluno, hostname);
	char msg[280];
	sprintf(msg, 
	  "{\"ts\": %ld,\"pos\": [%f, %f, %f],\"quat\": [%f, %f, %f, %f]}",
       s.timestamp,
       s.position[0], s.position[1], s.position[2],
       s.orientation[0], s.orientation[1], s.orientation[2], s.orientation[3]); 

	int res = mqttPublish(topic, msg, 0, 0);
	return res;
}

int main(int argc, char** argv) {
	if ( argc>1 && argv[1][0]=='-' ) {
		if ( argv[1][1]=='b' ) {
			debug = 0;
			int pid = fork();
			if ( pid ) return 0;
		}
	}
	if (gethostname(hostname, sizeof(hostname)) != 0) {
        perror("gethostname");
        strcpy(hostname, "Unknown");
	}
    int fd;
    while ((fd = shm_open(SHM_NAME, O_RDONLY, 0666)) < 0) {
		perror("waiting for shm...");
		sleep(1);
	}
    if ( debug>3 ) printf("got shm\n");
    shared_state_t *shared = mmap(NULL, sizeof(shared_state_t),
                                  PROT_READ,
                                  MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap"); return 1;
    }
    if ( debug>3 ) printf("got mmap\n");
    
	sem_t *sem;
	while ((sem = sem_open(SEM_NAME, 0)) == SEM_FAILED) {
		perror("waiting for semaphore...");
		sleep(1);
	}
    if ( debug>3 ) printf("got sem\n");
    
    mqttConnect();
    
	/* Run the network loop in a background thread, this call returns quickly. */
	int rc = mosquitto_loop_start(mosq);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}
    
    
    while (1) {
        // bloqueia até haver novo estado
        sem_wait(sem);

        getit_state s = shared->state;

		if (debug) getit_printState(s);
		getit_publishState(s);
    }

    return 0;
}

