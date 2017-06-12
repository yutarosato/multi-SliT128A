#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "my_socket.h"
#include "my_signal.h"
#include "set_timer.h"
#include "readn.h"

volatile sig_atomic_t has_interrupt = 0;
volatile sig_atomic_t has_term      = 0;
volatile sig_atomic_t has_alarm     = 0;

#define GLOBAL_HEADER_SIZE  8
#define UNIT_HEADER_SIZE    6
#define UNIT_DATA_SIZE      6
#define N_CHIP              4
#define N_UNIT              4
#define BUFSIZE     1024*1024

int usage()
{
    char msg[] = "Usage: readslit [-t timeout_sec] ip_address port filename";
    fprintf(stderr, "%s\n", msg);

    return 0;
}

void sig_alarm(int signo)
{
    has_alarm = 1;

    return;
}

void sig_int(int signo)
{
    has_interrupt = 1;

    return;
}


void sig_term(int signo)
{
    has_term = 1;

    return;
}

int get_data_len(unsigned char *buf, int len)
{
  ///*
  // mark "ee"
  unsigned char global_mark = buf[0];
  global_mark = ( global_mark >> 4 );
  printf("************************************************Mark : %x\n",(int)global_mark);
  // event number
  unsigned short* tmp_event_number = (unsigned short*)&buf[4];
  int event_number = ntohs(*tmp_event_number);
  printf("event number : %d\n", event_number);
  //*/
  // ndata 
  unsigned long tmp_total_ndata = *(unsigned long*)&buf[1];
  tmp_total_ndata = ((tmp_total_ndata & 0xffffff01)  );
  unsigned long total_ndata = ntohl(tmp_total_ndata);
  total_ndata = (total_ndata >> 8);
  printf("total ndata : %ld\n", total_ndata);

  return (int)total_ndata*UNIT_DATA_SIZE + N_CHIP*N_UNIT*UNIT_HEADER_SIZE;
}

int main(int argc, char *argv[])
{
    unsigned char buf[BUFSIZE];
    
    int c;
    int timeout_sec = 0;
    int sockfd;
    char *filename;
    char *ip_address;
    int port;
    int n;
    FILE *fp;

    while ( (c = getopt(argc, argv, "t:")) != -1) {
        switch (c) {
            case 't':
                timeout_sec = strtol(optarg, NULL, 0);
                break;
            case '?':
                usage();
                exit(1);
            default:
                break;
        }
    }
    argc -= optind;
    argv += optind;

    if (argc != 3) {
        usage();
        exit(1);
    }

    ip_address = argv[0];
    port       = strtol(argv[1], NULL, 0);
    filename   = argv[2];

    my_signal(SIGALRM, sig_alarm);
    my_signal(SIGTERM, sig_term);
    my_signal(SIGINT,  sig_int);

    sockfd = tcp_socket();
    if (sockfd < 0) {
        err(1, "tcp_socket");
    }
    
    if (connect_tcp(sockfd, ip_address, port) < 0) {
        err(1, "tcp_connect");
    }

    fp = fopen(filename, "w");
    if (fp == NULL) {
        err(1, "fopen for %s", filename);
    }

    if (timeout_sec > 0) {
        set_timer(timeout_sec, 0, timeout_sec, 0);
    }

    int cnt = 0;
    for ( ; ; ) {
        if (has_term || has_interrupt || has_alarm) {
            fclose(fp);
            close(sockfd);
            exit(0);
        }

        // read header part
	fprintf(stderr,"START READN : %d\n", cnt++);
        n = readn(sockfd, &buf[0], GLOBAL_HEADER_SIZE);
        if (n != GLOBAL_HEADER_SIZE) {
            errx(1, "readn for header fail.  %d bytes request, %d bytes read", GLOBAL_HEADER_SIZE, n);
        }
        int data_len = get_data_len(&buf[0], GLOBAL_HEADER_SIZE);

        // read data part
        n = readn(sockfd, &buf[GLOBAL_HEADER_SIZE], data_len);
        if (n != data_len) {
            errx(1, "readn for data fail.  %d bytes request, %d bytes read", data_len, n);
        }

        // then write to file
        n = fwrite(&buf[0], 1, GLOBAL_HEADER_SIZE + data_len, fp);
        if (n == 0) {
            if (ferror(fp)) {
                err(1, "fwrite");
            }
        }
    }

    return 0;
}
