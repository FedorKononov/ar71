#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVERPORT 6000

unsigned int CRC_TAB[] = {
    /* CRC tab */
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

unsigned char Check_crc(unsigned char *ptr, unsigned char len)
{
    unsigned int crc;
    unsigned char dat;

    crc = 0;

    while (len-- != 0)
    {
        dat = crc >> 8;

        crc <<= 8;
         
        crc ^= CRC_TAB[dat ^ *ptr];
        ptr++;
    }

    dat = crc;

    if ((*ptr == (unsigned char)(crc >> 8)) && (*(ptr+1) == dat))
        return 1;

    return 0;
}

void Pack_crc(unsigned char *ptr, unsigned char len)
{
    unsigned int crc;
    unsigned char dat;

    crc = 0;

    while (len-- != 0)
    {
        dat = crc >> 8;

        crc <<= 8;

        crc ^= CRC_TAB[dat ^ *ptr];
        ptr++;
    }

    *ptr = crc >> 8;
    ptr++;
    *ptr = crc;
}

void array_concat(const void *a, size_t an,
                   const void *b, size_t bn, size_t s, unsigned char *res)
{
    int i;

    char *p = malloc(s * (an + bn));
    memcpy(p, a, an * s);
    memcpy(p + an * s, b, bn * s);

    for (i = 0; i < (an + bn); i++){
        res[i] = p[i];
        //printf("%d : 0x%x\n", i, p[i]);
    }

    free(p);
}

int main(int argc, char const *argv[])
{
    int sockfd;
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;
    int numbytes;
    int broadcast = 1;
    unsigned char hdl_proto_packet[16];
    unsigned char data_packet[14];
    unsigned char full_command[30];

    //char broadcast = '1'; // if that doesn't work, try this

    if (argc != 3) {
        fprintf(stderr,"usage: broadcaster hostname message\n");
        exit(1);
    }

    if ((he = gethostbyname(argv[1])) == NULL) {  // get the host info
        perror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // this call is what allows broadcast packets to be sent:
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
        sizeof broadcast) == -1) {
        perror("setsockopt (SO_BROADCAST)");
        exit(1);
    }

    their_addr.sin_family = AF_INET;         // host byte order
    their_addr.sin_port = htons(SERVERPORT); // short, network byte order
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

    /**
     * filling packet
     */

    hdl_proto_packet[0]  = 0xC0;
    hdl_proto_packet[1]  = 0xA8;
    hdl_proto_packet[2]  = 0x0A;
    hdl_proto_packet[3]  = 0x64;
    hdl_proto_packet[4]  = 'H';
    hdl_proto_packet[5]  = 'D';
    hdl_proto_packet[6]  = 'L';
    hdl_proto_packet[7]  = 'M';
    hdl_proto_packet[8]  = 'I';
    hdl_proto_packet[9]  = 'R';
    hdl_proto_packet[10] = 'A';
    hdl_proto_packet[11] = 'C';
    hdl_proto_packet[12] = 'L';
    hdl_proto_packet[13] = 'E';
    hdl_proto_packet[14] = 0xAA; // leading code
    hdl_proto_packet[15] = 0xAA; // leading code

    data_packet[0]  = 0x0E; // data length
    data_packet[1]  = 0x01; // subnet id
    data_packet[2]  = 0x5B; // device id
    data_packet[3]  = 0x02; // original device type: higher then 8
    data_packet[4]  = 0x59; // original device type: lower then 8
    data_packet[5]  = 0x00; // operate code: higher then 8
    data_packet[6]  = 0x32; // operate code: lower then 8
    data_packet[7]  = 0xff; // subnet ID of targeted device
    data_packet[8]  = 0xff; // device ID of targeted device
    data_packet[9]  = 0x05; // additional, channel No
    data_packet[10] = 0xf8; // success
    data_packet[11] = atoi(argv[2]); // additional, intensity
    data_packet[12] = 0x00; // CRC, higher then 8
    data_packet[13] = 0x00; // CRC, lower then 8

    // generate 2 byte crc
    Pack_crc(data_packet, sizeof(data_packet) - 2);

    array_concat(hdl_proto_packet, sizeof(hdl_proto_packet), data_packet, sizeof(data_packet), sizeof(unsigned char), full_command);
   
    if ((numbytes=sendto(sockfd, full_command, sizeof(full_command), 0,
             (struct sockaddr *)&their_addr, sizeof their_addr)) == -1) {
        perror("sendto");
        exit(1);
    }

    printf("sent %d bytes to %s\n", numbytes,
        inet_ntoa(their_addr.sin_addr));

    close(sockfd);

    return 0;
}