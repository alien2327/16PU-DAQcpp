//SiTCP setting
#define ID_ADDRESS "10.72.108.42"
#define RBCP_PORT 0x1234
#define RBCP_BUF_SIZE 1024
#define TCP_PORT 0x18
#define UDP_BUF_SIZE 1024

//RBCP Status
struct RBCP_HEADER{
    unsigned char version;
    unsigned char flag;
    unsigned char id;
    unsigned char length;
    unsigned char address;
    unsigned char data_0;
    unsigned char data_1;
    unsigned char data_2;
};

// RBCP MODE Integar
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
#include <cstdlib>
#include <string.h>

using namespace std;

int CommandList(char* pszVerb, char* pszArg1, char* pszArg2, char *ipAddr, unsigned int rbcpPort, unsigned int tcpPort, int addr, struct RBCP_HEADER* header, int *dispMode, int *datanumber);
int GetData(char* pszArg2, char* ipAddr, unsigned int port, struct RBCP_HEADER* sendHeader);
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
    cout << "║                               Last update 2020/10/27      ║" << endl;
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
        if (strcmp(pszArg1, "delay") == 0) {
            if (strcmp(pszArg2, "") != 0) {
                header->flag = RBCP_WR_CMD;
                header->length = 1;
                header->address = htonl(38);
                header->data_0 = 0x3F & atoi(pszArg2);
                header->data_1 = 0;
                header->data_2 = 0;  
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
                    header->address = htonl(1 + (ch * 2));
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
        if (strcmp(pszArg1, "process") == 0) {
            if (strcmp(pszArg2, "") != 0) {
                header->flag = RBCP_WR_CMD;
                header->length = 1;
                header->address = htonl(33);
                header->data_0 = (char)((0x00FF0000 & atoi(pszArg2))>>16);
                header->data_1 = (char)((0x0000FF00 & atoi(pszArg2))>>8);
                header->data_2 = (char)(0x000000FF & atoi(pszArg2));            
                SendCommand(ipAddr, rbcpPort, header);
                SetMode(ipAddr, rbcpPort, header, pszArg1);
            } else {
                puts("Parameter error. See help.");
                return 0;
            }
        } else if (strcmp(pszArg1, "wave") == 0) {
            if (strcmp(pszArg2, "") != 0) {
                header->flag = RBCP_WR_CMD;
                header->length = 1;
                header->address = htonl(33);
                header->data_0 = (char)((0x00FF0000 & atoi(pszArg2))>>16);
                header->data_1 = (char)((0x0000FF00 & atoi(pszArg2))>>8);
                header->data_2 = (char)(0x000000FF & atoi(pszArg2));            
                SendCommand(ipAddr, rbcpPort, header);
                SetMode(ipAddr, rbcpPort, header, pszArg1);
            } else {
                puts("Parameter error. See help.");
                return 0;
            }
        } else if (strcmp(pszArg1, "") == 0) {
            puts("Parameter error. See help.");
            return 0;
        }
        return 0;
    } else if (strcmp(pszVerb, "trigger") == 0) {
        return Trigger(ipAddr, rbcpPort, header);
    } else if (strcmp(pszVerb, "reset") == 0) {
        puts("Set reset mode");
        header->flag = RBCP_WR_CMD;
        header->length = 1;
        header->address = htonl(0xFFFFFF10);
        header->data_0 = 0x80;
        header->data_1 = 0;
        header->data_2 = 0;
        SendCommand(ipAddr, rbcpPort, header);
        puts("Set normal mode");
        header->flag = RBCP_WR_CMD;
        header->length = 1;
        header->address = htonl(0xFFFFFF10);
        header->data_0 = 0;
        header->data_1 = 0;
        header->data_2 = 0;
        return SendCommand(ipAddr, rbcpPort, header);
    } else if (strcmp(pszVerb, "ipconfig") == 0) {
        return ShowIp(ipAddr, rbcpPort, tcpPort, addr);
    } else if (strcmp(pszVerb, "check") == 0) {
        return ConnectionCheck(ipAddr, rbcpPort, header);  
    } else if (strcmp(pszVerb, "help") == 0) {
        return Help();  
    } else if (strcmp(pszVerb, "quit") == 0) {
        return -1;
    }
    puts("No such command");
    return 0;
}

int GetData(char* pszArg2, char* ipAddr, unsigned int port, struct RBCP_HEADER* sendHeader) {
    char recvData[READING_BUF_SIZE];

    struct sockaddr_in sitcpAddr;
    int sock, res;
    int optVal;

    struct timeval timeout;
    fd_set setSelect;

    char sndBuf[RBCP_BUF_SIZE];
    int packetLen;

    int i, j = 0;
    int rcvdBytes;

    sitcpAddr.sin_family      = AF_INET;
    sitcpAddr.sin_port        = htons(port);
    sitcpAddr.sin_addr.s_addr = inet_addr(ipAddr);

    //Memory Read size
    //最小　圧縮　最大
    //4096 87380 174760 (4KB 8KB 16KB)
    sock = socket(AF_INET, SOCK_STREAM, 0);
    res = bind(sock, (struct sockaddr *) &sitcpAddr, sizeof(sitcpAddr));
    if (res == -1) {
        puts("Data read out error: socket binding failure");
        close(sock);
        return -1;
    }
    /*
    if (getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &optVal, sizeof(int)) == -1) {
        puts("Data read out error: getting option value failure");
        close(sock);
        return -1;
    }

    optVal *= 2;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &optVal, sizeof(int)) == -1) {
        puts("Data read out error: setting option value failure");
        close(sock);
        return -1;
    }
    cout << "Option setting status: " << optVal << endl;
    */
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

    sendHeader->flag = RBCP_RD_CMD;
    sendHeader->id = 1;
    sendHeader->length = 40;
    sendHeader->address = htonl(0x00000000);
    sendHeader->data_0 = 0;
    sendHeader->data_1 = 0;
    sendHeader->data_2 = 0;

    puts("\nChecking connection...");

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    sitcpAddr.sin_family      = AF_INET;
    sitcpAddr.sin_port        = htons(port);
    sitcpAddr.sin_addr.s_addr = inet_addr(ipAddr);

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
                return 2;
            }
        } else {
            if(FD_ISSET(sock,&setSelect)){
                rcvdBytes = recvfrom(sock, rcvdBuf, 2048, 0, NULL, NULL);
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
    sendHeader->address = htonl(0x00000000);
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
    sendHeader->address = htonl(0x00000037);
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
    puts("\t\tdelay\t: Change clock delay");
    puts("\t\tgain\t: Change gain value");
    puts("\tdata\t\t: Read out wave/process binary data. Usage, data <mode> <data amount>.");
	puts("\ttrigger\t\t: Send test trigger signal");
    puts("\tcheck\t\t: Check conncetion and current mode");
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

/*

***** RBCP ADDRESS SETTING OF FPGA *****
***** Information is contained RBCP.v
   always@(posedge CLK) begin
      if(RBCP_RE) begin
   	 case(RBCP_ADDR)
   	   0 : RBCP_RD <= {6'b0,mode};
   	   1 : RBCP_RD <= G[15:8];
   	   2 : RBCP_RD <= G[7:0];
   	   3 : RBCP_RD <= G[31:24];
   	   4 : RBCP_RD <= G[23:16];
   	   5 : RBCP_RD <= G[47:40];
   	   6 : RBCP_RD <= G[39:32];
   	   7 : RBCP_RD <= G[63:56];
   	   8 : RBCP_RD <= G[55:48];
   	   9 : RBCP_RD <= G[79:72];
   	   10: RBCP_RD <= G[71:64];
   	   11: RBCP_RD <= G[95:88];
   	   12: RBCP_RD <= G[87:80];
   	   13: RBCP_RD <= G[111:104];
   	   14: RBCP_RD <= G[103:96];
   	   15: RBCP_RD <= G[127:120];
   	   16: RBCP_RD <= G[119:112];
   	   17: RBCP_RD <= G[143:136];
   	   18: RBCP_RD <= G[135:128];
   	   19: RBCP_RD <= G[159:152];
   	   20: RBCP_RD <= G[151:144];
   	   21: RBCP_RD <= G[175:168];
   	   22: RBCP_RD <= G[167:160];
   	   23: RBCP_RD <= G[191:184];
   	   24: RBCP_RD <= G[183:176];
   	   25: RBCP_RD <= G[207:200];
   	   26: RBCP_RD <= G[199:192];
   	   27: RBCP_RD <= G[223:216];
   	   28: RBCP_RD <= G[215:208];
   	   29: RBCP_RD <= G[239:232];
   	   30: RBCP_RD <= G[231:224];
   	   31: RBCP_RD <= G[255:248];
   	   32: RBCP_RD <= G[247:240];
	   33: RBCP_RD <= data_number[23:16];
	   34: RBCP_RD <= data_number[15:8];
	   35: RBCP_RD <= data_number[7:0];
	   36: RBCP_RD <= {4'b0,ch_control};
	   37: RBCP_RD <= {7'b0,status};	
	   38: RBCP_RD <= {7'b0,test_trigger}; //by tajima
	   default : RBCP_RD <= 8'h3f;
	 endcase
      end
      else if(RBCP_WE) begin
	 case(RBCP_ADDR)
	   0 :mode       <= RBCP_WD[1:0];
	   1 :G[15:8]    <= RBCP_WD[7:0];
	   2 :G[7:0]     <= RBCP_WD[7:0];
	   3 :G[31:24]   <= RBCP_WD[7:0];
	   4 :G[23:16]   <= RBCP_WD[7:0];
	   5 :G[47:40]   <= RBCP_WD[7:0];
	   6 :G[39:32]   <= RBCP_WD[7:0];
	   7 :G[63:56]   <= RBCP_WD[7:0];
	   8 :G[55:48]   <= RBCP_WD[7:0];
	   9 :G[79:72]   <= RBCP_WD[7:0];
	   10:G[71:64]   <= RBCP_WD[7:0];
	   11:G[95:88]   <= RBCP_WD[7:0];
	   12:G[87:80]   <= RBCP_WD[7:0];
	   13:G[111:104] <= RBCP_WD[7:0];
	   14:G[103:96]  <= RBCP_WD[7:0];
	   15:G[127:120] <= RBCP_WD[7:0];
	   16:G[119:112] <= RBCP_WD[7:0];
	   17:G[143:136] <= RBCP_WD[7:0];
	   18:G[135:128] <= RBCP_WD[7:0];
	   19:G[159:152] <= RBCP_WD[7:0];
	   20:G[151:144] <= RBCP_WD[7:0];
	   21:G[175:168] <= RBCP_WD[7:0];
	   22:G[167:160] <= RBCP_WD[7:0];
	   23:G[191:184] <= RBCP_WD[7:0];
	   24:G[183:176] <= RBCP_WD[7:0];
	   25:G[207:200] <= RBCP_WD[7:0];
	   26:G[199:192] <= RBCP_WD[7:0];
	   27:G[223:216] <= RBCP_WD[7:0];
	   28:G[215:208] <= RBCP_WD[7:0];
	   29:G[239:232] <= RBCP_WD[7:0];
	   30:G[231:224] <= RBCP_WD[7:0];
	   31:G[255:248] <= RBCP_WD[7:0];
	   32:G[247:240] <= RBCP_WD[7:0];
	   33:data_number[23:16] <= RBCP_WD[7:0];
	   34:data_number[15:8]  <= RBCP_WD[7:0];
	   35:data_number[7:0]   <= RBCP_WD[7:0];
	   36:ch_control <= RBCP_WD[3:0];
	   37:test_trigger <= RBCP_WD[0]; //by tajima
	   //40:reset_clk64 <= RBCP_WD[0]; // by tajima
	 endcase
      end

*/