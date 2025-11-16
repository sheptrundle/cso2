#include <stdio.h>
#include "netsim.h"

#define TIMEOUT_MS 1000

// globals
static unsigned char expected_seq = 1;
static unsigned char total_seqs = 0;
static char last_ack[5];
static int timeout_id = -1;

// helper to compute XOR checksum
unsigned char checksum(size_t len, unsigned char *data) {
    unsigned char cs = 0;
    for (size_t i = 1; i < len; i++)
        cs ^= data[i];
    return cs;
}


// helper to send a message with checksum auto-filled
void sendMsg(size_t len, unsigned char *data) {
    data[0] = checksum(len, data);
    send(len, data);
}

// timeout callback to resend last ACK/GET
void resend(void *arg) {
    (void)arg; // unused
    sendMsg(5, (unsigned char*)last_ack);
    timeout_id = setTimeout(resend, TIMEOUT_MS, NULL);
}

void recvd(size_t len, void* _data) {
    // FIX ME -- add proper handling of messages
    char *data = _data;
    unsigned char seq = data[1];
    unsigned char total = data[2];

    // set total count if first packet
    if (total_seqs == 0)
        total_seqs = total;

    // print in-order messages
    if (seq == expected_seq) {
        fwrite(data + 3, 1, len - 3, stdout);
        fflush(stdout);
        expected_seq++;
    }
    // prepare ACK message
    unsigned char ack[5];
    ack[1] = 'A'; ack[2] = 'C'; ack[3] = 'K';
    ack[4] = (expected_seq - 1);
    memcpy(last_ack, ack, 5);

    sendMsg(5, ack);

    // reset timeout to wait for next packet
    if (timeout_id > 0)
        clearTimeout(timeout_id);
    if (expected_seq <= total_seqs)
        timeout_id = setTimeout(resend, TIMEOUT_MS, NULL);
}

int main(int argc, char *argv[]) {
    // this code should work without modification
    if (argc != 2) {
        fprintf(stderr, "USAGE: %s n\n    where n is a number between 0 and 3\n", argv[0]);
        return 1;
    }
    char data[5];
    data[1] = 'G'; data[2] = 'E'; data[3] = 'T'; data[4] = argv[1][0];
    // end of working code

    // compute checksum (xor of all other bytes)
    data[0] = checksum(5, data);
    memcpy(last_ack, data, 5);
    send(5, data);
    
    // FIX ME -- add action if no reply
    timeout_id = setTimeout(resend, TIMEOUT_MS, NULL);
    waitForAllTimeoutsAndMessagesThenExit();
}
