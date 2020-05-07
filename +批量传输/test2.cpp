#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include<iostream>
#include<direct.h>
#include<io.h>
#include<string>
using namespace std;
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996) 

#define BUF_SIZE 1024
char buffer[BUF_SIZE];  //文件缓冲区

int main() {

	//先输入文件保存路径
	string in_filepath;
	printf("输入要保存文件的路径: ");
	getline(cin, in_filepath);
	const char *filepath;  //文件保存路径
	filepath = in_filepath.c_str();

	/*创建连接的SOCKET */
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0)
	{/*创建失败 */
		fprintf(stderr, "socker Error:%s\n", strerror(errno));
		exit(1);
	}

	/* 初始化客户端地址*/
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	//sockAddr.sin_addr.s_addr = inet_addr("192.168.0.106");
	sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);		//默认绑定本机所有ip
	sockAddr.sin_port = htons(2345);					//绑定一个端口，此处自己选择绑定了2345
	if (bind(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR)) < 0)
	{ /*棒定失败 */
		fprintf(stderr, "Bind Error:%s\n", strerror(errno));
		system("pause");
		exit(1);
	}

	/* 初始化服务器地址*/
	sockaddr_in ser_addr;
	ser_addr.sin_family = AF_INET;

	string server_ip;
	cout << "请输入服务器的ip地址：";
	cin >> server_ip;
	const char *server;
	server = server_ip.c_str();
	ser_addr.sin_addr.s_addr = inet_addr(server);
	int server_port;
	cout << "请输入服务器的端口号：";
	cin >> server_port;
	ser_addr.sin_port = htons(server_port);

	//ser_addr.sin_addr.s_addr = inet_addr("192.168.0.106");
	//ser_addr.sin_port = htons(1234);

	if (connect(sock, (SOCKADDR*)&ser_addr, sizeof(SOCKADDR)) < 0)	//请求连接
	{/*连接失败 */
		fprintf(stderr, "Connect Error:%s\n", strerror(errno));
		closesocket(sock);
		exit(1);
	}
	recv(sock, buffer, BUF_SIZE, 0);
	strcpy(buffer, "iGet");
	send(sock, buffer, sizeof(buffer), 0);
	for(int j=int(buffer[0]);j>0;j--)
	{
		char buffer[BUF_SIZE];  //文件缓冲区
		//接收发送过来的原文件路径
		recv(sock, buffer, BUF_SIZE, 0);
		cout << "收到：" << buffer << endl;

		int flag = 0;	//0----发送的是文件夹型名称（不带.）；1----发送的是文件名称(带.)
		for (int i = 0; buffer[i] != '\0'; i++)
		{
			if (buffer[i] == '.')
				flag = 1;
		}
		while (flag == 0)	//发送的是文件夹名称，判断是否有此文件夹，没有就创建一个
		{
			//与自己输入的路径拼接成本地文件夹路径
			char filefoldername[200] = { 0 };
			strcpy(filefoldername, filepath);
		
			strcat(filefoldername, "\\");
			strcat(filefoldername, buffer);

			if (0 != access(filefoldername, 0))
			{
				//不存在此文件夹，重新创建一个
				mkdir(filefoldername);
				cout << "创建文件夹：" << filefoldername << endl;
				strcpy(filefoldername,"\0");
			}

			//告知服务端，我们收到了此信息
			strcpy(buffer, "CilentGet");
			send(sock, buffer, sizeof(buffer), 0);

			//接收下一条信息，判断是文件夹名还是文件名
			recv(sock, buffer, BUF_SIZE, 0);
			//cout << "收到：" << buffer << endl;
			for (int i = 0; buffer[i] != '\0'; i++)
			{
				if (buffer[i] == '.')
					flag = 1;
			}
		}
		while (flag == 1)		//发送的是文件名
		{
			//与输入的路径拼接成本地文件路径
			char filename_path[200] = { 0 };
			strcpy(filename_path, filepath);
			strcat(filename_path, "\\");
			strcat(filename_path, buffer);

			FILE *fp = fopen(filename_path, "wb");  //以二进制方式打开（创建）文件

			//告知服务端我们收到此文件名
			strcpy(buffer, "CilentGet");
			send(sock, buffer, sizeof(buffer), 0);

			//循环接收数据，直到文件传输完毕，每收到一段文件内容遍回传一段"CilentRecivedDATA."信息。
			int nCount;
			while ((nCount = recv(sock, buffer, BUF_SIZE, 0)) > 0)
			{
				if (strcmp(buffer, "ThisFileIsEnd.") == 0)
				{
					cout << "收到：" << buffer << endl;
					strcpy(buffer, "CilentRecivedFile.");
					send(sock, buffer, sizeof(buffer), 0);
					break;
				}
				fwrite(buffer, nCount, 1, fp);
				strcpy(buffer, "CilentRecivedDATA.");
				send(sock, buffer, sizeof(buffer), 0);
			}
			fclose(fp);

			cout << filename_path << " Recived" << endl;

			recv(sock, buffer, BUF_SIZE, 0);	//接收此文件传完后的下一条信息
			//cout << "收到：" << buffer << endl;
			flag = 0;
			for (int i = 0; buffer[i] != '\0'; i++)	//判断信息是否带.(文件标志)
			{
				if (buffer[i] == '.')
					flag = 1;
			}
			if (strcmp(buffer, "ThisSendIsEnd.") == 0)//判断传输是否结束
			{
				flag = 0;
				strcpy(buffer, "CilentRecivedAllFile.");
				send(sock, buffer, sizeof(buffer), 0);
			}
		}
		puts("文件传输成功!"); }
	//文件接收完毕后直接关闭套接字，无需调用shutdown()
	closesocket(sock);
	WSACleanup();
	system("pause");
	return 0;
}
