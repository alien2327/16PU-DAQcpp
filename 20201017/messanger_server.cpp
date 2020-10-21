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
    cout << "Waititng for client" << endl;

    //指定したサービスプロバイダへのTCPソケットを作成する。
    int listen_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_socket == INVALID_SOCKET)
    {
        cout << "couldn't open a socket.\n";
        return 1;
    }

    //IPv4のソケットアドレス情報を設定する
    struct sockaddr_in sockadd, from;
    memset(&sockadd, 0, sizeof(sockadd));
    sockadd.sin_family = AF_INET;
    sockadd.sin_port = htons(59422);
    sockadd.sin_addr.s_addr = INADDR_ANY;

    bool yes = 1;
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));

    //ソケットに名前をつける
    if (bind(listen_socket, (struct sockaddr *)(&sockadd), sizeof(sockadd)) == SOCKET_ERROR)
    {
        cout << "bind failure.\n";
        close(listen_socket);
        return 1;
    }

    //接続待ち状態にする
    if (listen(listen_socket, 0) == SOCKET_ERROR)
    {
        cout << "listen failure.\n";
        close(listen_socket);
        return 1;
    }

    //クライアントからの接続を受け入れる
    memset(&from, 0, sizeof(from));
    socklen_t from_len = sizeof(from);
    int sock;
    sock = accept(listen_socket, (struct sockaddr *)(&from), &from_len);
    if (sock == INVALID_SOCKET)
    {
        cout << "accept failure.\n";
        close(listen_socket);
        return 1;
    }

    cout << inet_ntoa(from.sin_addr) << " is connected.\n";
    cout << "If you want to quit, press 'end'" << endl;

    close(listen_socket);

    //メッセージを送受信
    string buffer;
    while (true)
    {
        cout << "Waiting for client...\n";
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
            cout << "Client is terminated\n";
            break;
        }
        cout << "Someone\t> " << buf << endl;

        cout << "You\t> ";
        cin >> noskipws >> buffer;
        send(sock, buffer.c_str(), buffer.size(), 0);
        if (buffer == "end")
        {
            break;
        }
    }
    //ソケットの送受信を切断する
    close(sock);
    cout << "End\n";

    return 0;
}