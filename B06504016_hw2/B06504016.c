/*Chia-Hung Lin */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <time.h>

/* ******************************************************************
 ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
   This code should be used for PA2, unidirectional or bidirectional
   data transfer protocols (from A to B. Bidirectional transfer of data
   is for extra credit and is not required).  Network properties:
   - one way network delay averages five time units (longer if there
     are other messages in the channel for GBN), but can be larger
   - packets can be corrupted (either the header or the data portion)
     or lost, according to user-defined probabilities
   - packets will be delivered in the order in which they were sent
     (although some can be lost).
**********************************************************************/

#define BIDIRECTIONAL 0    /* change to 1 if you're doing extra credit */
                           /* and write a routine called B_output */

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg {
  char data[20];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt {
   int seqnum;
   int acknum;
   int checksum;
   char payload[20];
};

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

#define OK 1
#define ERROR 0
#define A_WINDOWSIZE 8
#define A_BUFFERSIZE 1000
typedef int State;
typedef struct msg msg;
typedef struct pkt pkt;

void calchecksum(pkt* packet){
    packet->checksum = packet->seqnum + packet->acknum;
    int i;
    for(i = 0;i < 20;i += 4){
        int x1 = (int)packet->payload[i];
        int x2 = (int)packet->payload[i + 1];
        int x3 = (int)packet->payload[i + 2];
        int x4 = (int)packet->payload[i + 3];
        x2 *= 256;
        x3 *= 65536;
        x4 *= 16777216;
        packet->checksum += (x1 + x2 + x3 + x4);
     }
     packet->checksum = ~ packet->checksum;
}//calchecksum

int test(pkt* packet){
    int check = packet->checksum;
    check += (packet->seqnum + packet->acknum);
    int i;
    for(i = 0;i < 20;i += 4){
        int y1 = (int)packet->payload[i];
        int y2 = (int)packet->payload[i + 1];
        int y3 = (int)packet->payload[i + 2];
        int y4 = (int)packet->payload[i + 3];
        y2 *= 256;
        y3 *= 65536;
        y4 *= 16777216;
        check += (y1 + y2 + y3 + y4);
    }
    return check;
}//test
extern float time;
int packetnum = 0;
float alpha = 0.125;
float sendtime[1000] = {0};
float receivetime[1000] = {0};
float sampleRTT[1000] = {0};
float estimatedRTT[1000] = {0};
float recordtime[1000];
int base;         /* the first unacknowledged */
int A_nextseqnum;
int A_seqnum;
int A_acknum;
int A_buffer_top;
float retranstime = 30.0;
pkt A_buffer[A_BUFFERSIZE];     /* packet buffer */
pkt B_packet;                   /* packet buffer */
int B_first_or_not;
int B_seqnum_looking_for;
int B_acknum;
char ACK[20] = "ACK";
char NAK[20] = "NAK";


/* called from layer 5, passed the data to be sent to other side */
State A_output(msg message){
    int i;
    if(((base + A_WINDOWSIZE) % A_BUFFERSIZE) == (A_nextseqnum)){
        if(((A_buffer_top) % A_BUFFERSIZE) == base){
            printf("BUFFER OVERFLOW!");
            return ERROR;
        }
        else{
            strncpy(A_buffer[A_buffer_top].payload,message.data,20);
            A_buffer_top = (A_buffer_top + 1) % A_BUFFERSIZE;
            return OK;
        }
    }
    else{
        strncpy(A_buffer[A_nextseqnum].payload,message.data,20);
        A_buffer[A_nextseqnum].seqnum = A_seqnum + (A_nextseqnum - base);
        A_buffer[A_nextseqnum].acknum = A_acknum + (A_nextseqnum - base);
        calchecksum(&A_buffer[A_nextseqnum]);
        tolayer3(0,A_buffer[A_nextseqnum]);         /* tolayer3(calling_entity,packet) */
        if(A_nextseqnum == base) starttimer(0,retranstime);

        /* starttimer(calling_entity,increment) */

        /***********************************************************/
        printf("A: sending:\n");
        printf("   seq:%d, ack:%d,\n",A_buffer[A_nextseqnum].seqnum,A_buffer[A_nextseqnum].acknum);
        printf("   check:%X\n",A_buffer[A_nextseqnum].checksum);
        printf("   message: ");
        for(i = 0;i < 20;i ++) printf("%c",A_buffer[A_nextseqnum].payload[i]);
        printf("\n");
        /***********************************************************/
        A_nextseqnum = (A_nextseqnum + 1) % A_BUFFERSIZE;
        A_buffer_top = (A_buffer_top + 1) % A_BUFFERSIZE;

    }//if
    return OK;
}//A_output

State B_output(msg message){  /* need be completed only for extra credit */

    return OK;

}
/* called from layer 3, when a packet arrives for layer 4 */
State A_input(pkt packet){
    /* first condition: wrong acknum*/
    printf("receive acknum: %d, looking for: %d\n",packet.seqnum,A_acknum);
    if(A_acknum != packet.seqnum) return ERROR;
    /* second condition: corrupted */
    if(test(&packet) == -1){ /* incorrupted */
        stoptimer(0);   /* stoptimer(calling_entity) */
        receivetime[packetnum] = time;
        if(packetnum == 0){
        sampleRTT[packetnum] = receivetime[packetnum] - 18.705740;
        estimatedRTT[0] = receivetime[0] - 18.705740;
        }
        if(packetnum > 0){
        sampleRTT[packetnum] = receivetime[packetnum] - sendtime[packetnum];
        estimatedRTT[packetnum] = alpha * sampleRTT[packetnum] + (1 - alpha) * estimatedRTT[packetnum-1];
        }
        recordtime[packetnum] = time;
        packetnum = packetnum + 1;
        if(!strcmp(packet.payload,ACK)){
            printf("ACK!\n");
            A_acknum++;
            A_seqnum++;
            base = (base + 1) % A_BUFFERSIZE;
            if(A_buffer_top != A_nextseqnum){
                A_buffer[A_nextseqnum].seqnum = A_seqnum + (A_nextseqnum - base);
                A_buffer[A_nextseqnum].acknum = A_acknum + (A_nextseqnum - base);
                calchecksum(&A_buffer[A_nextseqnum]);
                tolayer3(0,A_buffer[A_nextseqnum]);
                A_nextseqnum = (A_nextseqnum + 1) % A_BUFFERSIZE;
            }
            starttimer(0,retranstime);
            sendtime[packetnum] = time;
            return OK;
        }//if
        else{
            printf("NAK!\n");
            tolayer3(0,A_buffer[base]);   /** retransmitted **/
            starttimer(0,retranstime);
            return OK;
        }//else
    }//if
    else{
        printf("corrupted!\n");
        return ERROR;
    }//else
    return OK;
}

/* called when A's timer goes off */
State A_timerinterrupt(){
    int i,j;
    for(j = base;j < A_nextseqnum;j++){
        tolayer3(0,A_buffer[j]);/** retransmitted **/

        /******************************/
        printf("sending:\n");
        printf("        seq:%d, ack:%d check:%X\n",A_buffer[j].seqnum,A_buffer[j].acknum,A_buffer[j].checksum);
        printf("        message:");
        for(i = 0;i < 20;i ++) printf("%c",A_buffer[j].payload[i]);
        printf("\n\n");
        /******************************/
    }//for
    starttimer(0,retranstime);
    return OK;

}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
State A_init(){
    base = 0;
    A_nextseqnum = 0;
    A_buffer_top = 0;
    A_seqnum = 0;
    A_acknum = 100;
    return OK;
}
/* Note that with simplex transfer from A-to-B, there is no B_output() */
/* called from layer 3, when a packet arrives for layer 4 at B*/
State B_input(pkt packet){
    int i;
    /******************************/
    printf("receiving:\n");
    printf("          seq:%d, ack:%d check:%X\n",packet.seqnum,packet.acknum,packet.checksum);
    printf("          message:");
    for(i = 0;i < 20;i ++) printf("%c",packet.payload[i]);
    printf("\n\n");
    /******************************/
    if(test(&packet) == -1){ /* incorrupted */
        if(B_first_or_not){
            B_seqnum_looking_for = packet.seqnum;
            B_acknum = packet.acknum;
            B_first_or_not = 0;
        }
        printf("receive:%d,looking for:%d\n",packet.seqnum,B_seqnum_looking_for);
        if(packet.seqnum != B_seqnum_looking_for){
            if(packet.seqnum > B_seqnum_looking_for){
                tolayer3(1,B_packet);
                /******************************/
                printf("B:sending:\n");
                printf("  seq:%d, ack:%d check:%X\n",B_packet.seqnum,B_packet.acknum,B_packet.checksum);
                printf("  message:");
                for(i = 0;i < 20;i ++) printf("%c",B_packet.payload[i]);
                printf("\n\n");
                /******************************/
                return OK;
            }
            else{
                pkt lost_or_corrputed_ACK;
                lost_or_corrputed_ACK.seqnum = packet.acknum;
                lost_or_corrputed_ACK.acknum = packet.seqnum + 1;
                strncpy(lost_or_corrputed_ACK.payload,ACK,20);
                calchecksum(&lost_or_corrputed_ACK);
                tolayer3(1,lost_or_corrputed_ACK);
                /******************************/
                printf("B:sending:\n");
                printf("  seq:%d, ack:%d ",lost_or_corrputed_ACK.seqnum,lost_or_corrputed_ACK.acknum);
                printf("check:%X\n",lost_or_corrputed_ACK.checksum);
                printf("  message:");
                for(i = 0;i < 20;i ++) printf("%c",lost_or_corrputed_ACK.payload[i]);
                printf("\n\n");
                /******************************/
                return OK;
            }
        }
        else{
            B_packet.seqnum = B_acknum++;
            B_packet.acknum = ++B_seqnum_looking_for;
            strncpy(B_packet.payload,ACK,20);
            msg receive;
            strncpy(receive.data,packet.payload,20);
            tolayer5(1,receive);
        }
    }
    else    strncpy(B_packet.payload,NAK,20);
    calchecksum(&B_packet);
    /******************************/
    printf("B:sending:\n");
    printf("  seq:%d, ack:%d check:%X\n",B_packet.seqnum,B_packet.acknum,B_packet.checksum);
    printf("  message:");
    for(i = 0;i < 20;i ++) printf("%c",B_packet.payload[i]);
    printf("\n\n");
    /******************************/
    tolayer3(1,B_packet);

    return OK;

}

/* called when B's timer goes off */
State B_timerinterrupt(){

    return OK;

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
State B_init(){
    B_first_or_not = 1;
    B_packet.seqnum = -1;
    B_packet.acknum = -1;
    strncpy(B_packet.payload,NAK,20);
    return OK;
}
/*****************************************************************
***************** NETWORK EMULATION CODE STARTS BELOW ***********
The code below emulates the layer 3 and below network environment:
  - emulates the tranmission and delivery (possibly with bit-level corruption
    and packet loss) of packets across the layer 3/4 interface
  - handles the starting/stopping of a timer, and generates timer
    interrupts (resulting in calling students timer handler).
  - generates message to be sent (passed from later 5 to 4)
THERE IS NOT REASON THAT ANY STUDENT SHOULD HAVE TO READ OR UNDERSTAND
THE CODE BELOW.  YOU SHOLD NOT TOUCH, OR REFERENCE (in your code) ANY
OF THE DATA STRUCTURES BELOW.  If you're interested in how I designed
the emulator, you're welcome to look at the code - but again, you should have
to, and you defeinitely should not have to modify
******************************************************************/

struct event{
   float evtime;           /* event time */
   int evtype;             /* event type code */
   int eventity;           /* entity where event occurs */
   struct pkt *pktptr;     /* ptr to packet (if any) assoc w/ this event */
   struct event *prev;
   struct event *next;
};
struct event *evlist = NULL;   /* the event list */
/*** possible events: ***/
#define  TIMER_INTERRUPT 0
#define  FROM_LAYER5     1
#define  FROM_LAYER3     2
#define  OFF             0
#define  ON              1
#define   A    0
#define   B    1
int TRACE = 1;             /* for my debugging */
int nsim = 0;              /* number of messages from 5 to 4 so far */
int nsimmax = 0;           /* number of msgs to generate, then stop */
float time = 0.000;
float lossprob;            /* probability that a packet is dropped  */
float corruptprob;         /* probability that one bit is packet is flipped */
float lambda;              /* arrival rate of messages from layer 5 */
int   ntolayer3;           /* number sent into layer 3 */
int   nlost;               /* number lost in media */
int ncorrupt;              /* number corrupted by media*/

int main(int argc, char **argv){
    struct event *eventptr;
    struct msg  msg2give;
    struct pkt  pkt2give;

    int i,j;
    char c;

    init(argc, argv);
    A_init();
    B_init();

    while(1){
        eventptr = evlist;  /* get next event to simulate */
        if(eventptr==NULL)
            goto terminate;
        evlist = evlist->next;  /* remove this event from event list */
        if(evlist!=NULL)
            evlist->prev=NULL;
        if(TRACE>=2){
            printf("\nEVENT time: %f,",eventptr->evtime);
            printf("  type: %d",eventptr->evtype);
            if(eventptr->evtype==0)
                printf(", timerinterrupt  ");
            else if (eventptr->evtype==1)
                printf(", fromlayer5 ");
            else
                printf(", fromlayer3 ");
            printf(" entity: %d\n",eventptr->eventity);
        }//if
        time = eventptr->evtime;        /* update time to next event time */
        if(nsim==nsimmax)
            break;                        /* all done with simulation */
        if(eventptr->evtype == FROM_LAYER5 ){
            generate_next_arrival();   /* set up future arrival */
            /* fill in msg to give with string of same letter */
            j = nsim % 26;
            for (i=0; i<20; i++)
                msg2give.data[i] = 97 + j;
            if(TRACE>2) {
                printf("          MAINLOOP: data given to student: ");
                for (i=0; i<20; i++) printf("%c", msg2give.data[i]);
                printf("\n");
            }
            nsim++;
            if(eventptr->eventity == A) A_output(msg2give);
            else B_output(msg2give);
            }
            else if(eventptr->evtype == FROM_LAYER3){
                pkt2give.seqnum = eventptr->pktptr->seqnum;
                pkt2give.acknum = eventptr->pktptr->acknum;
                pkt2give.checksum = eventptr->pktptr->checksum;
            for(i = 0; i < 20; i++)
                pkt2give.payload[i] = eventptr->pktptr->payload[i];
	    if (eventptr->eventity ==A)      /* deliver packet by calling */
   	       A_input(pkt2give);            /* appropriate entity */
            else
   	       B_input(pkt2give);
	    free(eventptr->pktptr);          /* free the memory for packet */
            }
          else if (eventptr->evtype ==  TIMER_INTERRUPT) {
            if (eventptr->eventity == A)
	       A_timerinterrupt();
             else
	       B_timerinterrupt();
             }
          else  {
	     printf("INTERNAL PANIC: unknown event type \n");
             }
        free(eventptr);
        }

terminate:
    printf(" Simulator terminated at time %f\n after sending %d msgs from layer5\n",time,nsim);
    int k = 0;
    for(k=0;k<packetnum;k++){
        printf("sample : %f ", sampleRTT[k]);
        printf("estimated : %f, ", estimatedRTT[k]);
        printf("time : %f \n", recordtime[k]);
    }
}



void init(int argc, char **argv) /* initialize the simulator */
{
    int i;
    float sum, avg;
    float jimsrand();

    if (argc != 6)
    {
        printf("usage: %s  num_sim  prob_loss  prob_corrupt  interval  debug_level\n", argv[0]);
        exit(1);
    }

    nsimmax = atoi(argv[1]);
    lossprob = atof(argv[2]);
    corruptprob = atof(argv[3]);
    lambda = atof(argv[4]);
    TRACE = atoi(argv[5]);
    printf("-----  Stop and Wait Network Simulator Version 1.1 -------- \n\n");
    printf("the number of messages to simulate: %d\n", nsimmax);
    printf("packet loss probability: %f\n", lossprob);
    printf("packet corruption probability: %f\n", corruptprob);
    printf("average time between messages from sender's layer5: %f\n", lambda);
    printf("TRACE: %d\n", TRACE);

    srand(9999); /* init random number generator */
    sum = 0.0;   /* test random number generator for students */
    for (i = 0; i < 1000; i++)
        sum = sum + jimsrand(); /* jimsrand() should be uniform in [0,1] */
    avg = sum / 1000.0;
    if (avg < 0.25 || avg > 0.75)
    {
        printf("It is likely that random number generation on your machine\n");
        printf("is different from what this emulator expects.  Please take\n");
        printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
        exit(1);
    }

    ntolayer3 = 0;
    nlost = 0;
    ncorrupt = 0;

    time = 0.0;              /* initialize time to 0.0 */
    generate_next_arrival(); /* initialize event list */
}
/****************************************************************************/
/* jimsrand(): return a float in range [0,1].  The routine below is used to */
/* isolate all random number generation in one location.  We assume that the*/
/* system-supplied rand() function return an int in therange [0,mmm]        */
/****************************************************************************/
float jimsrand(){
  double mmm = 2147483647;   /* largest int  - MACHINE DEPENDENT!!!!!!!!   */
  float x;                   /* individual students may need to change mmm */
  x = rand()/mmm;            /* x should be uniform in [0,1] */
  return(x);
}

/********************* EVENT HANDLINE ROUTINES *******/
/*  The next set of routines handle the event list   */
/*****************************************************/

void generate_next_arrival(){

    double x,log(),ceil();
    struct event *evptr;
    //char* malloc();
    float ttime;
    int tempint;

    if(TRACE>2)
        printf("          GENERATE NEXT ARRIVAL: creating new arrival\n");

    x = lambda*jimsrand()*2;  /* x is uniform on [0,2*lambda] */
                             /* having mean of lambda        */
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime =  time + x;
    evptr->evtype =  FROM_LAYER5;
    if(BIDIRECTIONAL && (jimsrand()>0.5))
        evptr->eventity = B;
    else
        evptr->eventity = A;
    insertevent(evptr);
}


insertevent(struct event *p){

   struct event *q,*qold;

   if (TRACE>2) {
      printf("            INSERTEVENT: time is %lf\n",time);
      printf("            INSERTEVENT: future time will be %lf\n",p->evtime);
      }
   q = evlist;     /* q points to header of list in which p struct inserted */
   if (q==NULL) {   /* list is empty */
        evlist=p;
        p->next=NULL;
        p->prev=NULL;
        }
     else {
        for (qold = q; q !=NULL && p->evtime > q->evtime; q=q->next)
              qold=q;
        if (q == NULL) {   /* end of list */
             qold->next = p;
             p->prev = qold;
             p->next = NULL;
             }
           else if (q == evlist) { /* front of list */
             p->next = evlist;
             p->prev = NULL;
             p->next->prev = p;
             evlist = p;
             }
           else {     /* middle of list */
             p->next = q;
             p->prev = q->prev;
             q->prev->next = p;
             q->prev = p;
             }
         }
}

void printevlist(){
    struct event *q;
    int i;
    printf("--------------\nEvent List Follows:\n");
    for(q = evlist; q!=NULL; q=q->next){
        printf("Event time: %f, type: %d entity: %d\n",q->evtime,q->evtype,q->eventity);
    }
    printf("--------------\n");
}



/********************** Student-callable ROUTINES ***********************/

/* called by students routine to cancel a previously-started timer */
void stoptimer(int AorB/* A or B is trying to stop timer */){

    struct event *q,*qold;

    if(TRACE>2)
        printf("          STOP TIMER: stopping timer at %f\n",time);
        /* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
    for(q=evlist; q!=NULL ; q = q->next)
        if((q->evtype==TIMER_INTERRUPT  && q->eventity==AorB)){
            /* remove this event */
            if(q->next==NULL && q->prev==NULL)
                evlist=NULL;         /* remove first and only event on list */
            else if (q->next==NULL) /* end of list - there is one in front */
                q->prev->next = NULL;
            else if (q==evlist){ /* front of list - there must be event after */
                q->next->prev=NULL;
                evlist = q->next;
            }
            else{     /* middle of list */
                q->next->prev = q->prev;
                q->prev->next =  q->next;
            }
        free(q);
        return;
    }
    printf("Warning: unable to cancel your timer. It wasn't running.\n");
}
starttimer(AorB,increment)
int AorB;  /* A or B is trying to stop timer */
float increment;
{
printf("In starttimer:increment:%f\n",increment);
 struct event *q;
 struct event *evptr;
 //char *malloc();

 if (TRACE>2)
    printf("          START TIMER: starting timer at %f\n",time);
 /* be nice: check to see if timer is already started, if so, then  warn */
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next)  */
   for (q=evlist; q!=NULL ; q = q->next)
    if ( (q->evtype==TIMER_INTERRUPT  && q->eventity==AorB) ) {
      printf("Warning: attempt to start a timer that is already started\n");
      return;
      }

/* create future event for when timer goes off */
   evptr = (struct event *)malloc(sizeof(struct event));
   evptr->evtime =  time + increment;
   evptr->evtype =  TIMER_INTERRUPT;
   evptr->eventity = AorB;
   insertevent(evptr);
}
/************************** TOLAYER3 ***************/
tolayer3(AorB,packet)
int AorB;  /* A or B is trying to stop timer */
struct pkt packet;
{
    struct pkt *mypktptr;
    struct event *evptr,*q;
    //char *malloc();
    float lastime, x, jimsrand();
    int i;
    ntolayer3++;
    /* simulate losses: */
 if (jimsrand() < lossprob)  {
      nlost++;
      if (TRACE>0)
	printf("          TOLAYER3: packet being lost\n");
      return;
    }

/* make a copy of the packet student just gave me since he/she may decide */
/* to do something with the packet after we return back to him/her */
 mypktptr = (struct pkt *)malloc(sizeof(struct pkt));
 mypktptr->seqnum = packet.seqnum;
 mypktptr->acknum = packet.acknum;
 mypktptr->checksum = packet.checksum;
 for (i=0; i<20; i++)
    mypktptr->payload[i] = packet.payload[i];
 if (TRACE>2)  {
   printf("          TOLAYER3: seq: %d, ack %d, check: %d ", mypktptr->seqnum,
	  mypktptr->acknum,  mypktptr->checksum);
    for (i=0; i<20; i++)
        printf("%c",mypktptr->payload[i]);
    printf("\n");
   }

/* create future event for arrival of packet at the other side */
  evptr = (struct event *)malloc(sizeof(struct event));
  evptr->evtype =  FROM_LAYER3;   /* packet will pop out from layer3 */
  evptr->eventity = (AorB+1) % 2; /* event occurs at other entity */
  evptr->pktptr = mypktptr;       /* save ptr to my copy of packet */
/* finally, compute the arrival time of packet at the other end.
   medium can not reorder, so make sure packet arrives between 1 and 10
   time units after the latest arrival time of packets
   currently in the medium on their way to the destination */
 lastime = time;
/* for (q=evlist; q!=NULL && q->next!=NULL; q = q->next) */
 for (q = evlist; q != NULL ; q = q->next)
    if ( (q->evtype == FROM_LAYER3  && q->eventity == evptr->eventity) )
      lastime = q->evtime;
 evptr->evtime =  lastime + 1 + 9*jimsrand();



 /* simulate corruption: */
 if (jimsrand() < corruptprob)  {
    ncorrupt++;
    if ( (x = jimsrand()) < .75)
       mypktptr->payload[0]='Z';   /* corrupt payload */
      else if (x < .875)
       mypktptr->seqnum = 999999;
      else
       mypktptr->acknum = 999999;
    if (TRACE>0)
	printf("          TOLAYER3: packet being corrupted\n");
    }

  if (TRACE>2)
     printf("          TOLAYER3: scheduling arrival on other side\n");
  insertevent(evptr);
}

tolayer5(int AorB,char datasent[20]){
  int i;
  if (TRACE>2) {
     printf("          TOLAYER5: data received: ");
     for (i=0; i<20; i++)
        printf("%c",datasent[i]);
     printf("\n");
   }

}
