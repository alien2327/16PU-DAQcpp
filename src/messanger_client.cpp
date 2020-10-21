#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>
using namespace std;

int main()
{
    cout << "********** start chat program **********" << endl;
    cout << "If you want to quit, type 'end'" << endl;
    cout << "Waititng for server reponse" << endl;

    //指定したサービスプロバイダへのTCPソケットを作成する。
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        cout << "couldn't open a socket.\n";
        return 1;
    }

    //IPv4のソケットアドレス情報を設定する
    struct sockaddr_in sockadd;
    memset(&sockadd, 0, sizeof(sockadd));
    sockadd.sin_family = AF_INET;
    sockadd.sin_port = htons(59422);
    sockadd.sin_addr.s_addr = inet_addr("127.0.0.1");

    //指定したソケットへ接続する
    if (connect(sock, (struct sockaddr *)(&sockadd), sizeof(sockadd)) != 0)
    {
        cout << "connect failure.\n";
        close(sock);
        return 1;
    }

    //メッセージを送受信
    string buffer;
    while (true)
    {
        cout << "You\t> ";
        cin >> noskipws >> buffer;
        send(sock, buffer.c_str(), buffer.size(), 0);
        if (buffer == "end")
        {
            break;
        }
        cout << "Waiting for reply...\n";
        char buf[1024];
        memset(buf, 0, sizeof(buf));
        int rec = recv(sock, buf, sizeof(buf)-1, 0);
        if (rec == SOCKET_ERROR)
        {
            cout << "error\n";
            break;
        }
        buf[rec] = '\0';
        if (strcmp(buf, "end") == 0)
        {
            cout << "server teminated.\n";
            break;
        }

        cout << "Someone\t> " << buf << endl;
    }
    //ソケットの送受信を切断する
    close(sock);
    cout << "End\n";

    return 0;
}
