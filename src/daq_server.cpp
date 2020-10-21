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
    unsigned int *data;
};

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

int main() {

    cout << "********** Start Test server for FPGA **********" << endl;

    int sock;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    
    struct sockaddr_in test_sitcpaddr, client;

    memset(&test_sitcpaddr, 0, sizeof(test_sitcpaddr));
    test_sitcpaddr.sin_family = AF_INET;
    test_sitcpaddr.sin_addr.s_addr = INADDR_ANY;
    test_sitcpaddr.sin_port = htons(4660);

    if (bind(sock, (struct sockaddr *) &test_sitcpaddr, sizeof(test_sitcpaddr)) == -1) {
        cout << "Socket bind failure" << endl;
        close(sock);
        return -1;
    }

    memset(&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr("10.227.9.146");
    client.sin_port = htons(4660);

    int step = 0;
    while(step < 20) {
        char buf[UDP_BUF_SIZE];

        cout << "Waiting for client..." << endl;
        memset(buf, 0, sizeof(buf));
    
        int rec = recv(sock, buf, sizeof(buf)-1, 0);
        if (rec == -1) {
            cout << "Receive error" << endl;
            break;
        }
        buf[rec] = '\0';

        cout << "Received " << rec << " data" << endl;
        for (int i = 0; i < rec; i++) {
            cout << +buf[i] << " ";
        }
        cout << endl;

        step++;
    }

    close(sock);
    cout << "Close server" << endl;

    return 0;
}