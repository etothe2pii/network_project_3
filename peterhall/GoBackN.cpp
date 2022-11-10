#include "includes.h"
struct  pkt *sndpkt[10];
struct pkt sndpktB;


int rtt;

int base;
int nextseqnum;

int expectedseqnum;





// ***************************************************************************
// * ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
// *
// * These are the functions you need to fill in.
// ***************************************************************************


int computeChecksum(struct pkt packet){
    int checksum = 0;
    checksum += packet.seqnum;
    checksum += packet.acknum;
    for(int i = 0; i < 20; i++){
        checksum += packet.payload[i];
    }

    return checksum;
}

// ***************************************************************************
// * The following routine will be called once (only) before any other
// * entity A routines are called. You can use it to do any initialization
// ***************************************************************************
void A_init() {

    rtt = 10;

    base = 1;
    nextseqnum = 1;
}

// ***************************************************************************
// * The following routine will be called once (only) before any other
// * entity B routines are called. You can use it to do any initialization
// ***************************************************************************
void B_init() {
    expectedseqnum = 1;
    sndpktB.seqnum = 0;
    sndpktB.acknum = 0;
    sndpktB.checksum = computeChecksum(sndpktB);
}

// ***************************************************************************
// * Called from layer 5, passed the data to be sent to other side 
// ***************************************************************************
bool rdt_sendA(struct msg message) {
    INFO << "RDT_SEND_A: Layer 4 on side A has received a message from the application that should be sent to side B: "
              << message << ENDL;

    if(nextseqnum < base + 10){
        struct pkt *packet = new struct pkt;
        packet -> seqnum = nextseqnum;
        packet -> acknum = 0;
        packet -> checksum = 0;

        for(int i = 0; i < 20; i++){
            packet -> payload[i] = message.data[i];
        }

        packet -> checksum = computeChecksum(*packet);

        simulation->udt_send(A,*packet);

        sndpkt[nextseqnum%10] = packet;
        
        if(nextseqnum == base){
            simulation->start_timer(A, rtt);
        }

        nextseqnum++;

        return(true);
    }

    INFO << "RDT_SEND_A: Layer 4 on side A has rejected a message from the application that should be sent to side B: "
              << message << ENDL;

    return(false);

    
}


// ***************************************************************************
// * Called from layer 3, when a packet arrives for layer 4 on side A
// ***************************************************************************
void rdt_rcvA(struct pkt packet) {
    if(packet.checksum == computeChecksum(packet)){
        base = packet.acknum + 1;
        if(base == nextseqnum){
            simulation->stop_timer(A);
        } else{
            simulation->start_timer(A, rtt);
        }

        INFO << "rdt_rcv_A: Ack packet recieved: "
              << packet << ENDL;
        
    } else{
    INFO << "rdt_rcv_A: Corrupted ack packet recieved: "
              << packet << ENDL;
    }
}


// ***************************************************************************
// * Called from layer 5, passed the data to be sent to other side
// ***************************************************************************
bool rdt_sendB(struct msg message) {
    INFO<< "RDT_SEND_B: Layer 4 on side B has received a message from the application that should be sent to side A: "
              << message << ENDL;

    bool accepted = false;

    return (accepted);
}


// ***************************************************************************
// // called from layer 3, when a packet arrives for layer 4 on side B 
// ***************************************************************************
void rdt_rcvB(struct pkt packet) {
    INFO << "RTD_RCV_B: Layer 4 on side B has received a packet from layer 3 sent over the network from side A:"
         << packet << ENDL;

    if(packet.checksum == computeChecksum(packet) && expectedseqnum == packet.seqnum){
        msg message;

        for(int i = 0; i < 20; i++){
            message.data[i] = packet.payload[i];
        }

        struct pkt packet2;
        packet2.acknum = packet.seqnum;
        packet2.checksum = computeChecksum(packet2);
    
        simulation->deliver_data(B, message);
        simulation->udt_send(B, packet2);
        INFO << "RTD_RCV_B: Layer 4 on side B sending ACK to side A:"
         << packet2 << ENDL;

        sndpktB.seqnum = packet2.seqnum;
        sndpktB.acknum = packet2.acknum;
        sndpktB.checksum = packet2.checksum;

        expectedseqnum ++;
    }
    else{
        simulation->udt_send(B, sndpktB);
        INFO << "RTD_RCV_B: Layer 4 on side B REsending ACK to side A:"
         << sndpktB <<  " Expectd seqnumber was: " << expectedseqnum << ENDL;
    }

    
}

// ***************************************************************************
// * Called when A's timer goes off 
// ***************************************************************************
void A_timeout() {
    INFO << "A_TIMEOUT: Side A's timer has gone off." << ENDL;
    rtt +=5;
    simulation -> start_timer(A, rtt);
    for(int i = base; i < nextseqnum + 10; i ++){
        if(sndpkt[i%10] != nullptr){
            INFO << "A_TIMEOUT: Resending packet: " << *sndpkt[1%10] << ENDL;
            simulation->udt_send(A, *sndpkt[i%10]);
        }
    }

}

// ***************************************************************************
// * Called when B's timer goes off 
// ***************************************************************************
void B_timeout() {
    INFO << "B_TIMEOUT: Side B's timer has gone off." << ENDL;
}
