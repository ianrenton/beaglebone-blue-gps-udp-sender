// Beaglebone Blue GPS UDP Sender
// by Ian Renton, March 2023
// https://github.com/ianrenton/beaglebone-blue-gps-udp-sender
//
// Reads lines from a serial port, and sends them via UDP to two ports. Maybe
// useful if you have several programs needing the data that can't both access
// the serial port, or you want to forward the data to another PC.
//
// If using this on a different device, you may need to customise the #define values
// near the top of the file to reflect the serial port of your GPS.

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

// Set the port on which GPS data arrives in NMEA-0183 format. For the Beaglebone Blue
// on Debian Linux, this is /dev/ttyS2.
#define GPS_PORT "/dev/ttyS2"
// This code sends the heading data to two UDP ports, on the host and with port numbers
// defined here.
#define UDP_SEND_SERVER "127.0.0.1"
#define UDP_SEND_PORT_1 2011
#define UDP_SEND_PORT_2 2012


// interrupt handler to catch ctrl-c
static int running = 0;
static void __signal_handler(__attribute__ ((unused)) int dummy) {
    running = 0;
    return;
}

int main()  {
    // Set up interrupt handler
    signal(SIGINT, __signal_handler);
    running = 1;

    // Open serial port, exit on failure
    int serial_port = open(GPS_PORT, O_RDONLY);
    if (serial_port < 0) {
        fprintf(stderr,"Open serial port failed\n");
        return -1;
    }

    // Configure serial port
    struct termios tty;
    if(tcgetattr(serial_port, &tty) != 0) {
        fprintf(stderr,"Get serial port config failed\n");
        return -1;
    }
    cfsetispeed(&tty, B9600);
    tty.c_cflag |= CREAD | CLOCAL;
    tty.c_lflag &= ~ECHO;
    tty.c_lflag &= ~ECHOE;
    tty.c_lflag &= ~ECHONL;
    tty.c_lflag &= ~ISIG;
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        fprintf(stderr,"Set serial port config failed\n");
        return -1;
    }

    // Create UDP sockets, exit on failure
    int socket1;
    struct sockaddr_in server1;
    if ((socket1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) >= 0) {
        server1.sin_family      = AF_INET;
        server1.sin_port        = htons(UDP_SEND_PORT_1);
        server1.sin_addr.s_addr = inet_addr(UDP_SEND_SERVER);
    } else {
        fprintf(stderr,"create socket 1 failed\n");
        return -1;
    }
    int socket2;
    struct sockaddr_in server2;
    if ((socket2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) >= 0) {
        server2.sin_family      = AF_INET;
        server2.sin_port        = htons(UDP_SEND_PORT_2);
        server2.sin_addr.s_addr = inet_addr(UDP_SEND_SERVER);
    } else {
        fprintf(stderr,"create socket 2 failed\n");
        return -1;
    }
    printf("Sending GPS data to local UDP ports %d and %d.\n", UDP_SEND_PORT_1, UDP_SEND_PORT_2);

    // Main run loop
    while (running) {
        // Read line from the serial port
        char buffer[256] = {0};
        int i = 0;
        while (i < 255) {
            read(serial_port, buffer + i, 1);
            if (buffer[i] == '\n') break;
            i++;
        }

        // Only proceed if there was content on the line
        if (i > 0) {
            // Null terminate the string and send it via UDP
            buffer[i+1] = '\0';
            sendto(socket1, buffer, i+1, 0, (struct sockaddr *)&server1, sizeof(server1));
            sendto(socket2, buffer, i+1, 0, (struct sockaddr *)&server2, sizeof(server2));
        }
    }

    // Close serial port and sockets
    close(serial_port);
    close(socket1);
    close(socket2);
    return 0;
}
