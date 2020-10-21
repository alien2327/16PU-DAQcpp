//SiTCP setting
#define ID_ADDRESS "10.72.108.42"
#define RBCP_PORT 0x1234
#define RBCP_BUF_SIZE 1024
#define TCP_PORT 0x18
#define UDP_BUF_SIZE 2048

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
    unsigned char data_0;
    unsigned char data_1;
    unsigned char data_2;
};

#define RBCP_VER 0xFF //0b1111 + 0b1111
#define RBCP_WR_CMD 0x80 //0b1000 + 0b0000
#define RBCP_WR_CMD_ERR 0x81 //0b1000 + 0b0001
#define RBCP_WR_ACK 0x88 //0b1000 + 0b1000
#define RBCP_WR_ACK_ERR 0x89 //0b1000 + 0b1001
#define RBCP_RD_CMD 0xC0 //0b1100 + 0b0000
#define RBCP_RD_CMD_ERR 0xC1 //0b1100 + 0b0001
#define RBCP_RD_ACK 0xC8 //0b1100 + 0b1000
#define RBCP_RD_ACK_ERR 0xC9 //0b1100 + 0b1001

#define MODE_PROCESS 0
#define MODE_WAVE 2
#define MODE_STEADY 3

//Readout setting
#define FILE_HEADER "TRANSVERS"
#define DATA_HEADER "wave"
#define FILE_FOOTER "DATA processed with the 16-pu-monitor circuit"
#define DATA_FOOTER "data"

#define MAX_LINE_LENGTH 1024
#define READING_BUF_SIZE 16384
#define MAX_PARAM_LENGTH 24
#define BUF_SIZE 40

#define Channel 16

#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string>
#include <string.h>

using namespace std;

int CommandList(char* pszVerb, char* pszArg1, char* pszArg2, char *ipAddr, unsigned int rbcpPort, unsigned int tcpPort, int addr, struct RBCP_HEADER* header, int *dispMode, int *datanumber);
int GetData(char* ipAddr, unsigned int port, struct RBCP_HEADER* sendHeader, char* recvData, int dispMode);
int ConnectionCheck(char* ipAddr, unsigned int port, struct RBCP_HEADER* sendHeader);
int SetMode(char* ipAddr, unsigned int port, struct RBCP_HEADER* sendHeader, char* pszArg2);
int Trigger(char* ipAddr, unsigned int port, struct RBCP_HEADER* sendHeader);
int ShowIp(char *ipAddr, unsigned int rbcpPort, unsigned int tcpPort, int address);
int SendCommand(char* ipAddr, unsigned int port, struct RBCP_HEADER* sendHeader);
int myGetArg(char* inBuf, int i, char* argBuf);
int myScanf(char* inBuf, char* argBuf1, char* argBuf2,  char* argBuf3);
int Help();

int main(int argc, char* argv[]) {
    cout << "╔═══════════════════════════════════════════════════════════╗" << endl;
    cout << "║    16Pick Up Beam Monitor DAQ connecting Program.         ║" << endl;
    cout << "║    This Program is built for collecting data for 16PU.    ║" << endl;
    cout << "║    If you have any connection error, please check out     ║" << endl;
    cout << "║    the ip address setting of script and firmware.         ║" << endl;
    cout << "║                                                           ║" << endl;
    cout << "║    Special Help : Toyama, Nakamura, Okada                 ║" << endl;
    cout << "║    Senior : Nakanishi, Uno, Tajima                        ║" << endl;
    cout << "║    Made by YOHAN LEE                                      ║" << endl;
    cout << "║                                                           ║" << endl;
    cout << "║                               Last update 2020/10/21      ║" << endl;
    cout << "╚═══════════════════════════════════════════════════════════╝" << endl;

    char *SiTCPAddr;
    char tempKeyBuf[MAX_LINE_LENGTH];
    char szVerb[MAX_PARAM_LENGTH];
    char szArg1[MAX_PARAM_LENGTH];
    char szArg2[MAX_PARAM_LENGTH];
    int rtnValue;
    int address;
    int check;

    unsigned int RBCPPPort;
    unsigned int TCPPort = TCP_PORT;

    int count = 0;

    int mode = 3;
    int dataNumber;

    if (argc != 4) {
        cout << "This application controls bus of a StTCP chip for getting data from FPGA" << endl;
        cout << "Usage: " << argv[0] << " <IP address> <Port #> <address(13, 15) #>" << endl;
        return -1;
    } else {
        SiTCPAddr = argv[1];
        RBCPPPort = atoi(argv[2]);
        address = atoi(argv[3]);

    }

    while(true) {
        RBCP_HEADER SiTCPHeader;
        SiTCPHeader.version = RBCP_VER;
        if (address != 0 && count == 0) {
            check = ConnectionCheck(SiTCPAddr, RBCPPPort, &SiTCPHeader);
            if (check < 0){
                address = 0;
            }
            count++;
        }
        printf("16PU-address#%d@DAQsystem$ ", address);
        fgets(tempKeyBuf, MAX_LINE_LENGTH, stdin);
        if ((rtnValue = myScanf(tempKeyBuf,szVerb, szArg1, szArg2)) < 0) {
            printf("Erro myScanf(): %i\n", rtnValue);
            return -1;
        }
        if (CommandList(szVerb, szArg1, szArg2, SiTCPAddr, RBCPPPort, TCPPort, address, &SiTCPHeader, &mode, &dataNumber) < 0) {
            break;
        }
    }

    return 0;
}

int CommandList(char* pszVerb, char* pszArg1, char* pszArg2, char *ipAddr, unsigned int rbcpPort, unsigned int tcpPort, int addr, struct RBCP_HEADER* header, int *dispMode, int *datanumber) {

    int sock_udp;
    header->id = 1;

    if (strcmp(pszVerb, "set") == 0) {
        if (strcmp(pszArg1, "mode") == 0) {
            if (strcmp(pszArg2, "") != 0) {
                return SetMode(ipAddr, rbcpPort, header, pszArg2);
            } else {
                puts("Parameter error. See help.");
                return 0;
            }
        } else if (strcmp(pszArg1, "delay") == 0) {
            if (strcmp(pszArg2, "") != 0) {
                header->flag = RBCP_WR_CMD;
                header->length = 1;
                header->address_0 = 0;
                header->address_1 = 0;
                header->address_2 = 0;
                header->address_3 = 38;
                header->data_0 = 0x3F & atoi(pszArg2);
                header->data_1 = 0;
                header->data_2 = 0;  
                return SendCommand(ipAddr, rbcpPort, header);
            } else {
                puts("Parameter error. See help.");
                return 0;
            }
        } else if (strcmp(pszArg1, "number") == 0) {
            if (strcmp(pszArg2, "") != 0) {
                header->flag = RBCP_WR_CMD;
                header->length = 1;
                header->address_0 = 0;
                header->address_1 = 0;
                header->address_2 = 0;
                header->address_3 = 33;
                header->data_0 = (char)((0x00FF0000 & atoi(pszArg2))>>16);
                header->data_1 = (char)((0x0000FF00 & atoi(pszArg2))>>8);
                header->data_2 = (char)(0x000000FF & atoi(pszArg2));            
                return SendCommand(ipAddr, rbcpPort, header);
            } else {
                puts("Parameter error. See help.");
                return 0;
            }
        } else if (strcmp(pszArg1, "gain") == 0) {
            if (strcmp(pszArg2, "") != 0) {
                for (int ch = 0; ch < Channel; ch++) {
                    cout << "Ch " << ch << " : ";
                    header->flag = RBCP_WR_CMD;
                    header->length = 1;
                    header->address_0 = 0xFF;
                    header->address_1 = 0xFF;
                    header->address_2 = 0xFF;
                    header->address_3 = (unsigned char)(ch * 2 + 1);
                    header->data_0 = (char)((0x0000FF00 & (32768 * atoi(pszArg2)))>>8);
                    header->data_1 = (char)(0x000000FF & (32768 * atoi(pszArg2)));
                    header->data_2 = 0;
                    SendCommand(ipAddr, rbcpPort, header);
                }
                return 0;
            } else {
                puts("Parameter error. See help.");
                return 0;
            }
        } else if (strcmp(pszArg1, "") == 0) {
            puts("Parameter error. See help.");
            return 0;
        }
    } else if (strcmp(pszVerb, "data") == 0) {
        return 0;
    } else if (strcmp(pszVerb, "trigger") == 0) {
        return Trigger(ipAddr, rbcpPort, header);
    } else if (strcmp(pszVerb, "reset") == 0) {
        header->flag = RBCP_WR_CMD;
        header->length = 1;
        header->address_0 = 0xFF;
        header->address_1 = 0xFF;
        header->address_2 = 0xFF;
        header->address_3 = 0x10;
        header->data_0 = 0x80;
        header->data_1 = 0;
        header->data_2 = 0;
        return SendCommand(ipAddr, rbcpPort, header);
    } else if (strcmp(pszVerb, "ipconfig") == 0) {
        return ShowIp(ipAddr, rbcpPort, tcpPort, addr);
    } else if (strcmp(pszVerb, "help") == 0) {
        return Help();  
    } else if (strcmp(pszVerb, "quit") == 0) {
        return -1;
    }
    puts("No such command");
    return 0;
}

int GetData() {
    char recvData[READING_BUF_SIZE];
    return 0;
}

int ConnectionCheck(char* ipAddr, unsigned int port, struct RBCP_HEADER* sendHeader) {
    struct sockaddr_in sitcpAddr;
    int sock;

    struct timeval timeout;
    fd_set setSelect;

    char sndBuf[RBCP_BUF_SIZE];
    int packetLen;

    int i, j = 0;
    int rcvdBytes;
    char rcvdBuf[UDP_BUF_SIZE];
    int numReTrans =0;

    puts("\nChecking connection...");

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    sitcpAddr.sin_family      = AF_INET;
    sitcpAddr.sin_port        = htons(port);
    sitcpAddr.sin_addr.s_addr = inet_addr(ipAddr);

    sendHeader->flag = RBCP_RD_CMD;
    sendHeader->length = 40;

    memcpy(sndBuf, sendHeader, sizeof(struct RBCP_HEADER));
    packetLen = sizeof(struct RBCP_HEADER);

    sendto(sock, sndBuf, packetLen, 0, (struct sockaddr *)&sitcpAddr, sizeof(sitcpAddr));
    puts("The packet have been sent!");
    puts("Wait to receive the ACK packet...");

    while(numReTrans < 5) {
        FD_ZERO(&setSelect);
        FD_SET(sock, &setSelect);
        timeout.tv_sec  = 1;
        timeout.tv_usec = 0;
        if (select(sock+1, &setSelect, NULL, NULL, &timeout) == 0) {
            puts("***** Timeout ! *****");
            sendHeader->id++;
            memcpy(sndBuf,sendHeader, sizeof(struct RBCP_HEADER));
            sendto(sock, sndBuf, packetLen, 0, (struct sockaddr *)&sitcpAddr, sizeof(sitcpAddr));
            numReTrans++;
            FD_ZERO(&setSelect);
            FD_SET(sock, &setSelect);
            if (numReTrans == 5) {
                cout << "Invalid ip address or no signal from FPGA." << endl;
                cout << "Change to address 0(TEST MODE)" << endl;
                return -1;
            }
        } else {
            if(FD_ISSET(sock,&setSelect)){
                rcvdBytes = recv(sock, rcvdBuf, UDP_BUF_SIZE, 0);
            }

            if (rcvdBytes<sizeof(struct RBCP_HEADER)) {
                puts("ERROR: ACK packet is too short");
                close(sock);
                return -1;
            }

            if ((0x0f & rcvdBuf[1]) != 0x8) {
                puts("ERROR: Detected bus error");
                close(sock);
                return -1;
            }

            rcvdBuf[rcvdBytes]=0;
            int offset = 8;
            if ((int)rcvdBuf[offset+0] == 0) puts("Current FPGA MODE : process");
            else if((int)rcvdBuf[offset+0] == 2) puts("Current FPGA MODE : wave");
            else if((int)rcvdBuf[offset+0] == 3) puts("Current FPGA MODE : steady");

            int data_length = 65536*((int)rcvdBuf[offset+33]) + 256*((int)rcvdBuf[offset+34]) + ((int)rcvdBuf[offset+35]);
            cout << "DATA # " << data_length << endl;

            if (rcvdBuf[offset+0] == 2 || rcvdBuf[offset+0] == 1) {
                if (rcvdBuf[offset+37] == 0) {cout << "sample memory is neither full nor empty." << endl;}
                else if(rcvdBuf[offset+37] == 1) {cout << "sample memory is full." << endl;}
                else if(rcvdBuf[offset+37] == 2) {cout << "sample memory is empty." << endl;}
            }

            numReTrans = 6;

            close(sock);
            return 0;
        }
    }

    close(sock);
    return 0;
}

int SetMode(char* ipAddr, unsigned int port, struct RBCP_HEADER* sendHeader, char* pszArg2) {

    struct sockaddr_in sitcpAddr;
    int dispMode;
    int sock;
    char sndBuf[RBCP_BUF_SIZE];
    int packetLen;
    int rcvdBytes;
    char rcvdBuf[UDP_BUF_SIZE];

    if (strcmp(pszArg2, "process") == 0) {
        puts("Setting mode to process...");
        dispMode = 0;
        }
    else if (strcmp(pszArg2, "wave") == 0) {
        puts("Setting mode to wave...");
        dispMode = 2;
        }
    else if (strcmp(pszArg2, "ready") == 0) {
        puts("Setting mode to ready...");
        dispMode = 3;
        }

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    sitcpAddr.sin_family      = AF_INET;
    sitcpAddr.sin_port        = htons(port);
    sitcpAddr.sin_addr.s_addr = inet_addr(ipAddr);

    sendHeader->flag = RBCP_WR_CMD;
    sendHeader->length = 1;
    sendHeader->data_0 = 0xFF & dispMode;;
    sendHeader->data_1 = 0;
    sendHeader->data_2 = 0;

    memcpy(sndBuf, sendHeader, sizeof(struct RBCP_HEADER));
    packetLen = sizeof(struct RBCP_HEADER);

    sendto(sock, sndBuf, packetLen, 0, (struct sockaddr *)&sitcpAddr, sizeof(sitcpAddr));
    puts("The packet have been sent!");
    puts("Wait to receive the ACK packet...");

    struct timeval timeout;
    fd_set setSelect;

    FD_ZERO(&setSelect);
    FD_SET(sock, &setSelect);

    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;

    if (select(sock+1, &setSelect, NULL, NULL, &timeout) == 0) {
        puts("***** Timeout ! *****");
        return 0;
    }

    rcvdBytes = recv(sock, rcvdBuf, UDP_BUF_SIZE, 0);

    sendHeader->id++;

    return 0;
}

int Trigger(char* ipAddr, unsigned int port, struct RBCP_HEADER* sendHeader) {
    struct sockaddr_in sitcpAddr;
    int sock;
    char sndBuf[RBCP_BUF_SIZE];
    int packetLen;
    int rcvdBytes;
    char rcvdBuf[UDP_BUF_SIZE];

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    sitcpAddr.sin_family      = AF_INET;
    sitcpAddr.sin_port        = htons(port);
    sitcpAddr.sin_addr.s_addr = inet_addr(ipAddr);

    sendHeader->flag = RBCP_WR_CMD;
    sendHeader->length = 1;
    sendHeader->address_3 = 37;
    sendHeader->data_0 = 1;
    sendHeader->data_1 = 0;
    sendHeader->data_2 = 0;

    memcpy(sndBuf, sendHeader, sizeof(struct RBCP_HEADER));
    packetLen = sizeof(struct RBCP_HEADER);
    
    sendto(sock, sndBuf, packetLen, 0, (struct sockaddr *)&sitcpAddr, sizeof(sitcpAddr));

    struct timeval timeout;
    fd_set setSelect;

    FD_ZERO(&setSelect);
    FD_SET(sock, &setSelect);

    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;

    if (select(sock+1, &setSelect, NULL, NULL, &timeout) == 0) {
        puts("***** Timeout ! *****");
        return 0;
    }

    rcvdBytes = recv(sock, rcvdBuf, UDP_BUF_SIZE, 0);

    sendHeader->id++;
    sendHeader->data_0 = 0;

    memcpy(sndBuf, sendHeader, sizeof(struct RBCP_HEADER));
    packetLen = sizeof(struct RBCP_HEADER);

    sendto(sock, sndBuf, packetLen, 0, (struct sockaddr *)&sitcpAddr, sizeof(sitcpAddr));
    
    rcvdBytes = recv(sock, rcvdBuf, UDP_BUF_SIZE, 0);

    sendHeader->id++;

    puts("Test trigger executed");

    return 0;
}

int ShowIp(char *ipAddr, unsigned int rbcpPort, unsigned int tcpPort, int address) {
    struct in_addr addr;
    struct hostent *host;
    int i = 0;
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        puts("error");
        return 0;
    }
    host = gethostbyname(hostname);
    if( host != NULL ) {
        while ( host->h_addr_list[i] != 0 ) {
            addr.s_addr = *(u_long *) host->h_addr_list[i++];
        }
    }
    cout << "\nDAQ system IP Configuration :" << endl;
    cout << "\tHost Name............: " << host->h_name << endl;
    cout << "\tIPv4 Address(HOST)...: " << inet_ntoa(addr) << endl;
    cout << "\tIPv4 Address(FPGA)...: " << ipAddr << endl;
    cout << "\tRBCP Port............: " << rbcpPort << endl;
    cout << "\tTCP Port.............: " << tcpPort << endl;
    cout << "\tMonitor Address......: #" << address << endl;
    cout << "\t  (The monitor address #0 means test mode.)\n" << endl;

    return 0;
}

int SendCommand(char* ipAddr, unsigned int port, struct RBCP_HEADER* sendHeader) {
    struct sockaddr_in sitcpAddr;

    int sock;
    char sndBuf[RBCP_BUF_SIZE];
    int packetLen;
    int rcvdBytes;
    char rcvdBuf[UDP_BUF_SIZE];

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    sitcpAddr.sin_family      = AF_INET;
    sitcpAddr.sin_port        = htons(port);
    sitcpAddr.sin_addr.s_addr = inet_addr(ipAddr);

    memcpy(sndBuf, sendHeader, sizeof(struct RBCP_HEADER));
    packetLen = sizeof(struct RBCP_HEADER);

    sendto(sock, sndBuf, packetLen, 0, (struct sockaddr *)&sitcpAddr, sizeof(sitcpAddr));

    struct timeval timeout;
    fd_set setSelect;

    FD_ZERO(&setSelect);
    FD_SET(sock, &setSelect);

    timeout.tv_sec  = 1;
    timeout.tv_usec = 0;

    if (select(sock+1, &setSelect, NULL, NULL, &timeout) == 0) {
        puts("***** Timeout ! *****");
        return 0;
    }

    rcvdBytes = recv(sock, rcvdBuf, UDP_BUF_SIZE, 0);

    puts("Done");

    sendHeader->id++;

    return 0;
}

int Help() {
    puts("\nCommand list:");
	puts("\tset\t\t: Change setting of FPGA. Usage, set <command> <parameter>");
    puts("\t\tmode\t: Change mode of FPGA(process/wave/ready)");
    puts("\t\tdelay\t: Change clock delay");
    puts("\t\tnumber\t: Set data number");
    puts("\t\tgain\t: Change gain value");
    puts("\tdata\t\t: Read out wave/process binary data. Currently not working.");
	puts("\ttrigger\t\t: Send test trigger signal");
	puts("\treset\t\t: Reset SiTCP");
    puts("\tipconfig\t: Show current ip address and port");
    puts("\thelp\t\t: Show available command");
	puts("\tquit\t\t: quit from this program\n");
	return 0;
}

int myScanf(char* inBuf, char* argBuf1, char* argBuf2,  char* argBuf3) {
    int i=0;

    argBuf1[0]='\0';
    argBuf2[0]='\0';
    argBuf3[0]='\0';

    if ((i = myGetArg(inBuf, i, argBuf1)) > 0) {
        if ((i = myGetArg(inBuf, i, argBuf2)) > 0) {
            return myGetArg(inBuf, i, argBuf3);
        } else {
            return i;
        }
    } else {
        return i;
    }
    return i;
}

int myGetArg(char* inBuf, int i, char* argBuf){
    int j;
    while (i < MAX_LINE_LENGTH) {
        if (isblank(inBuf[i])) {
            i++;
        } else if (iscntrl(inBuf[i])) {
            return 0;
        } else {
            break;
        }
    }

    for(j = 0; j < MAX_PARAM_LENGTH; j++){
        if (i < MAX_LINE_LENGTH) {
            if (isblank(inBuf[i])) {
                argBuf[j] = '\0';

            } else if (iscntrl(inBuf[i])) {
                argBuf[j] = '\0';
                return 0;
            } else {
                argBuf[j] = inBuf[i];
                i++;
            }
        } else {
            return -1;
        }
    }
    return i;
}
