#include "usersFunctions.h"
#include <math.h>
#include <stdio.h>


#ifdef EPICS

#include <cadef.h>
#include <epicsEvent.h>

static double timeout = 3.0; 

/* connect to a PV */
int connect_channel(const char *name,  chid *ch_id) {
    int status = ECA_NORMAL; 
    status = ca_create_channel(name, NULL, NULL, CA_PRIORITY_DEFAULT, ch_id);
    if (status != ECA_NORMAL)
        return status; 

    status = ca_pend_io(timeout); 
    return status; 
}

/* disconnect to a PV */
int disconnect_channel(chid ch_id)
{
    ca_clear_channel(ch_id); 
    ca_flush_io(); 
}

int caget(chid ch_id,  double *value) {

    int status = ECA_NORMAL; 

    status = ca_get(DBR_DOUBLE, ch_id, value); 
    if (status  !=  ECA_NORMAL) {
        return status; 
    }

    status = ca_pend_io(timeout); 
    if (status  !=  ECA_NORMAL) {
        return status; 
    }

    return status; 
}

int caputq(chid ch_id,  double value) {
  // does not wait!
    int status = ECA_NORMAL; 

    status = ca_put(DBR_DOUBLE, ch_id, &value); 
    if (status  !=  ECA_NORMAL)
        return status; 

    status = ca_pend_io(timeout); 
    if (status  !=  ECA_NORMAL) {
        return status; 
    }

    return status; 
}

void put_callback(struct event_handler_args args)
{
    epicsEventId eid = (epicsEventId)args.usr; 
    epicsEventSignal(eid); 
}

int caput(chid ch_id,  double value) {

  // waits!
    int status = ECA_NORMAL; 
    epicsEventId eid = epicsEventCreate(epicsEventEmpty); 

    status = ca_put_callback(DBR_DOUBLE, 
            ch_id, 
            &value, 
            put_callback,
            eid); 
    status = ca_pend_io(timeout); 
    if (status  != ECA_NORMAL) 
        return status; 

    if (epicsEventWait(eid) != epicsEventWaitOK) 
        status = ECA_TIMEOUT; 

    return status; 
}

/* int main(int argc,  char *argv[]) { */
/*     double value = 256; */
/*     /\* channel name *\/ */
/*     const char *name = "ARIDI-PCT:CURRENT";  */
/*     /\* channel id *\/ */
/*     chid ch_id;  */
/*     /\* status code *\/ */
/*     int status;  */

/*     /\* init channel access context before any caget/put *\/ */
/*     ca_context_create(ca_enable_preemptive_callback); */
 
/*     /\* open the channel by name and return ch_id *\/ */
/*     status = connect_channel(name,  &ch_id); */
/*     if (status  ==  ECA_NORMAL) */
/*         printf("channel connected %s\n",  name);  */
/*     else { */
/*         printf(ca_message(status));  */
/*         return -1;  */
/*     } */
/*     /\* caput and wait until done *\/ */
/*     if ((status = caput(ch_id, value)) == ECA_NORMAL) */
/*         printf("caput: success\n"); */
/*     else */
/*         printf(ca_message(status));  */
    
/*     /\* caget *\/ */
/*     if (caget(ch_id, &value) == ECA_NORMAL) */
/*         printf("caget: %f\n",  value);  */
/*     else */
/*         printf(ca_message(status));  */
   
/*     /\* close channel connect *\/ */
/*     disconnect_channel(ch_id);  */

/*     /\* delete channel access context before program exits *\/ */
/*     ca_context_destroy();  */
/* } */



#endif








float pos;
float i0=0;

/* 
   contains the conversion channel-angle for a module channel
   conv_r=pitch/radius
*/


float angle(int ichan, float encoder, float totalOffset, float conv_r, float center, float offset, float tilt, int direction) {
 
  (void) offset; /* to avoid warning: unused parameter */
  (void) tilt; /* to avoid warning: unused parameter */
  float ang;

  ang=180./PI*(center*conv_r+atan((float)(ichan-center)*conv_r))+encoder+totalOffset; 

  return direction*ang;

}

/* reads the encoder and returns the position */

float get_position() {
#ifdef VERBOSE
  printf("Getting motor position \n");
#endif
  return pos;
}

/* moves the encoder to position p */

int go_to_position(float p) {
#ifdef VERBOSE
  printf("Setting  motor position \n");
#endif
  pos=p;
  return pos;
}

/* moves the encoder to position p without waiting */

int go_to_position_no_wait(float p) {
#ifdef VERBOSE
  printf("Setting  motor position no wait \n");
#endif
  pos=p;
  return pos;
}


/* reads I0 and returns the intensity */

float get_i0() {
#ifdef VERBOSE
  printf("Getting I0 readout \n");
#endif
  return i0++;
}
  
int disconnect_channels() { }
int connect_channels() {}
