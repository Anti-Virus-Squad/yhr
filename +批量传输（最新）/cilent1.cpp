#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include<iostream>
#include<direct.h>
#include<io.h>
#include<string>
#include "md5.h"
#include <string.h>
#include <fstream>
using namespace std;
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996) 

SOCKET clntSock;
SOCKADDR clntAddr;
#define BUF_SIZE 1024
char buffer[BUF_SIZE];  //文件缓冲区
void sendfile(const char* filename, string logname, SOCKET sock,unsigned int filelength);
int main() {

	//先输入文件保存路径
	string in_filepath;
	printf("输入要保存文件的路径: ");
	getline(cin, in_filepath);
	const char* filepath;  //文件保存路径
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

	/*初始化服务器地址*/
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;

	string server_ip;
	cout << "请输入服务器的ip地址：";
	cin >> server_ip;
	const char* server;
	server = server_ip.c_str();
	sockAddr.sin_addr.s_addr = inet_addr(server);

	//sockAddr.sin_addr.s_addr = inet_addr("192.168.0.106");

	int server_port;
	cout << "请输入服务器的端口号：";
	cin >> server_port;
	sockAddr.sin_port = htons(server_port);

	if (bind(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR)) < 0)
	{/*绑定失败*/
		fprintf(stderr, "Bind Error:%s\n", strerror(errno));
		exit(1);
	}

	/*侦听客户端请求*/
	if (listen(sock, 20) < 0)
	{
		fprintf(stderr, "Listen Error:%s\n", strerror(errno));
		closesocket(sock);
		exit(1);
	}

	cout << "等待客户端接入..." << endl;

	/* 等待接收客户连接请求*/
	int nSize = sizeof(SOCKADDR);
	clntSock = accept(sock, (SOCKADDR*)&clntAddr, &nSize);
	if (clntSock <= 0)
	{
		fprintf(stderr, "Accept Error:%s\n", strerror(errno));
	}

	recv(sock, buffer, BUF_SIZE, 0);
	strcpy(buffer, "iGet");
	send(sock, buffer, sizeof(buffer), 0);
	for (int j = int(buffer[0]); j > 0; j--)
	{
		char buffer[BUF_SIZE];  //文件缓冲区
		//接收发送过来的原文件路径
		recv(clntSock, buffer, BUF_SIZE, 0);
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
				strcpy(filefoldername, "\0");
			}

			//告知服务端，我们收到了此信息
			strcpy(buffer, "CilentGet");
			send(clntSock, buffer, sizeof(buffer), 0);

			//接收下一条信息，判断是文件夹名还是文件名
			recv(clntSock, buffer, BUF_SIZE, 0);
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

			FILE* fp = fopen(filename_path, "wb");  //以二进制方式打开（创建）文件
			string filename = filename_path;
			// ‘\'的位置
			int backslashIndex;
			// 识别最后一个'\'的位置。
			backslashIndex = filename.find_last_of('.');
			//string logname = filename.substr(0,backslashIndex)+".log";
			string logname = filename + ".log";
			//告知服务端我们收到此文件名
			strcpy(buffer, "CilentGet");
			send(clntSock, buffer, sizeof(buffer), 0);
			unsigned int filelength;
			char buf[4];
			memset(buf, 0, 4);
			recv(clntSock, buf, 4, 0);
			memcpy_s(&filelength, 4, buf, 4);
			sendfile(filename_path, logname, clntSock, filelength);

			fp = fopen(filename_path, "rb");
			//验算MD5
			MD5_CTX md5;
			unsigned char buff[1024];
			unsigned char decrypt[16];
			unsigned int len;
			MD5Init(&md5);
			while (len = fread(buff, 1, 1024, fp))
			{
				MD5Update(&md5, buff, len);
			}
			MD5Final(&md5, decrypt);
			char result[50] = { 0 };
			strcpy(result, toHexstream(decrypt, 16));

			fclose(fp);

			cout << filename_path << " Recived" << endl;

			//接收MD5
			recv(clntSock, buffer, BUF_SIZE, 0);
			char trueresult[50] = { 0 };
			strcpy(trueresult, buffer);
			if (strcmp(trueresult, result) == 0) {
				cout << "接收文件无误" << endl;
				strcpy(buffer, "TransmissionCorrect.");
				send(clntSock, buffer, sizeof(buffer), 0);
			}
			else {
				cout << "接收文件" << filename_path << "有误，尝试重新传输" << endl;
				strcpy(buffer, "ErrorCode.");
				send(clntSock, buffer, sizeof(buffer), 0);
				char* log = const_cast<char*>(logname.c_str());
				remove(log);//删除文件
				remove(filename_path);
				sendfile(filename_path, logname, clntSock, filelength);
				//验算MD5
				fp = fopen(filename_path, "rb");
				MD5_CTX md5_2;
				unsigned char buff_2[1024];
				unsigned char decrypt_2[16];
				MD5Init(&md5_2);
				while (len = fread(buff_2, 1, 1024, fp))
				{
					MD5Update(&md5_2, buff, len);
				}
				MD5Final(&md5_2, decrypt_2);
				strcpy(result, toHexstream(decrypt_2, 16));
				fclose(fp);

				cout << filename_path << " Recived" << endl;

				if (strcmp(trueresult, result) == 0)
					cout << "接收文件无误" << endl;
				else {
					cout << "文件" << filename_path << "传输失败" << endl;
				}
			}


			recv(clntSock, buffer, BUF_SIZE, 0);	//接收此文件传完后的下一条信息
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
				send(clntSock, buffer, sizeof(buffer), 0);
			}
		}
		puts("文件传输完毕");
	}
	//文件接收完毕后直接关闭套接字，无需调用shutdown()
	closesocket(clntSock);
	closesocket(sock);
	WSACleanup();
	system("pause");
	return 0;
}

void sendfile(const char* filename, string logname,SOCKET sock, unsigned int filelength)
{
	FILE* fp = fopen(filename, "ab");
	ofstream file;
	unsigned int pos;
	file.open(logname, ios::_Nocreate | ios::binary);
	if (file.is_open())
	{
		ifstream readlog;
		readlog.open(logname);
		if (readlog.is_open())readlog >> pos;
		else pos = 0;
		readlog.close();
	}
	else
	{
		file.open(logname, ios::app | ios::binary);
		pos = 0;
	}

	send(sock, (char*)&pos, 4, 0);
	
	//循环接收数据，直到文件传输完毕，每收到一段文件内容遍回传一段"CilentRecivedDATA."信息。
	int nCount=0;
	for (unsigned int i = pos; i < filelength; i += nCount)
	{
		nCount = recv(sock, buffer, BUF_SIZE, 0);
		if (nCount == SOCKET_ERROR || nCount == 0)
		{
			ofstream writelog;
			writelog.open(logname, ios::trunc);
			writelog << i;
			writelog.close();
			file.close();
			closesocket(sock);
			return;

		}

		fwrite(buffer, nCount, 1, fp);
		strcpy(buffer, "CilentRecivedDATA.");
		send(sock, buffer, sizeof(buffer), 0);
	}
	fclose(fp);
	nCount = recv(sock, buffer, BUF_SIZE, 0);
	if (strcmp(buffer, "ThisFileIsEnd.") == 0)
	{
		cout << "收到：" << buffer << endl;
		strcpy(buffer, "CilentRecivedFile.");
		send(sock, buffer, sizeof(buffer), 0);
		
	}
}