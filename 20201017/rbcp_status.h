//SiTCP setting
#define _ID_ADDRESS "10.72.108.42"
#define _RBCP_PORT 0x1234
#define _TCP_PORT 0x18
#define _UDP_BUF_SIZE 2048

//RBCP Status
struct RBCP_HEADER{
    unsigned char version;
    unsigned char flag;
    unsigned char id;
    unsigned char length;
    unsigned char address_0;
    unsigned char address_1;
    unsigned char address_2;
    unsigned char address_3;
    unsigned int data;
};

#define _RBCP_VER 0xFF //0b1111 + 0b1111
#define _RBCP_WR_CMD 0x80 //0b1000 + 0b0000
#define _RBCP_WR_CMD_ERR 0x81 //0b1000 + 0b0001
#define _RBCP_WR_ACK 0x88 //0b1000 + 0b1000
#define _RBCP_WR_ACK_ERR 0x89 //0b1000 + 0b1001
#define _RBCP_RD_CMD 0xC0 //0b1100 + 0b0000
#define _RBCP_RD_CMD_ERR 0xC1 //0b1100 + 0b0001
#define _RBCP_RD_ACK 0xC8 //0b1100 + 0b1000
#define _RBCP_RD_ACK_ERR 0xC9 //0b1100 + 0b1001

#define _MODE_PROCESS 0
#define _MODE_WAVE 2
#define _MODE_STEADY 3

//Readout setting
#define _FILE_HEADER "TRANSVERS"
#define _DATA_HEADER "wave"
#define _FILE_FOOTER "DATA processed with the 16-pu-monitor circuit"
#define _DATA_FOOTER "data"

#define _MAX_LINE_LENGTH 1024
#define _READING_BUF_SIZE 16384
#define _MAX_PARAM_LENGTH 24
#define _BUF_SIZE 40

#define _Channel 16