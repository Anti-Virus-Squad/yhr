#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include<string>
#include<iostream>
#include<direct.h>
#include<io.h>
#include<fstream>
#include<vector>
//#include "md5.h"
#include <windows.h>
#include <tchar.h>
//#include "zip.h"
using namespace std;

#pragma comment (lib, "ws2_32.lib")  //加载 ws2_32.dll
#define BUF_SIZE 1024
#pragma warning(disable:4996) 

SOCKET clntSock;
SOCKADDR clntAddr;
char buffer[BUF_SIZE] = { 0 };  //缓冲区
string filenames;
//void singleFile(const char* filename, string afilename);
//void folder();
vector<string>files;		//存文件夹内所有子文件的全路径
vector<string>names;		//存文件夹内所有子文件的文件名
string fatherpath = "";		//用于截取文件夹名称
int trans_flag = 1;
int main() {

	//先输入文件名，看文件是否能创建成功
	char filepath[100] = { 0 };  //文件名
	printf("输入要保存文件的路径: ");
	gets_s(filepath);

	/*创建连接的SOCKET*/
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	SOCKET servSock = socket(AF_INET, SOCK_STREAM, 0);
	if (servSock < 0)
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

	//sockAddr.sin_port = htons(1234);

	if (bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR)) < 0)
	{/*绑定失败*/
		fprintf(stderr, "Bind Error:%s\n", strerror(errno));
		exit(1);
	}

	/*侦听客户端请求*/
	if (listen(servSock, 20) < 0)
	{
		fprintf(stderr, "Listen Error:%s\n", strerror(errno));
		closesocket(servSock);
		exit(1);
	}

	cout << "等待客户端接入..." << endl;

	/* 等待接收客户连接请求*/
	int nSize = sizeof(SOCKADDR);
	clntSock = accept(servSock, (SOCKADDR*)&clntAddr, &nSize);
	if (clntSock <= 0)
	{
		fprintf(stderr, "Accept Error:%s\n", strerror(errno));
		closesocket(servSock);
		closesocket(clntSock);
		exit(1);
	}

	

	char buffer[BUF_SIZE];  //文件缓冲区
	//接收发送过来的原文件路径
	/*while (trans_flag == 1)*/
	{
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
			}

			//告知客户端，我们收到了此信息
			strcpy(buffer, "CilentGet");
			send(clntSock, buffer, sizeof(buffer), 0);

			//接收下一条信息，判断是文件夹名还是文件名
			recv(clntSock, buffer, BUF_SIZE, 0);
			cout << "收到：" << buffer << endl;
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
			send(clntSock, buffer, sizeof(buffer), 0);

			//循环接收数据，直到文件传输完毕，每收到一段文件内容遍回传一段"CilentRecivedDATA."信息。
			int nCount;
			while ((nCount = recv(clntSock, buffer, BUF_SIZE, 0)) > 0)
			{
				if (strcmp(buffer, "ThisFileIsEnd.") == 0)
				{
					cout << "收到：" << buffer << endl;
					strcpy(buffer, "CilentRecivedFile.");
					send(clntSock, buffer, sizeof(buffer), 0);
					break;
				}
				fwrite(buffer, nCount, 1, fp);
				strcpy(buffer, "CilentRecivedDATA.");
				send(clntSock, buffer, sizeof(buffer), 0);
			}
			fclose(fp);

			cout << filename_path << " Recived" << endl;

			recv(clntSock, buffer, BUF_SIZE, 0);	//接收此文件传完后的下一条信息
			cout << "收到：" << buffer << endl;
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
		puts("文件传输成功!");

		//以下两行是控制循环部分
		/*recv(clntSock, buffer, BUF_SIZE, 0);
		send(clntSock, "go on", 6, 0);
		if (strcmp(buffer, "0") == 0)trans_flag = 0;*/

	}
	//文件接收完毕后直接关闭套接字，无需调用shutdown()
	closesocket(clntSock);
	closesocket(servSock);
	WSACleanup();
	system("pause");
	return 0;
}


/*void singleFile(const char* filename, string afilename) {

	//选择是否发送压缩包
	string choice;
	cout << "选择压缩后发送输入y，不压缩输入n" << endl;
	cin >> choice;
	if (choice == "y") {
		char realname[50] = { 0 };
		for (int i = 0; afilename[i] != '.' && i < afilename.length(); i++) {
			realname[i] = afilename[i];
		}
		char zipname[50] = { 0 };
		strcpy(zipname, realname);
		strcat(zipname, ".zip");
		HZIP hz = CreateZip(zipname, 0);
		ZipAdd(hz, afilename.c_str(), filename);
		CloseZip(hz);
		filename = zipname;
		afilename = zipname;
	}

	//以二进制方式打开文件
	FILE* fp = fopen(filename, "rb");
	if (fp == NULL) {
		printf("Cannot open file, press any key to exit!\n");
		exit(0);
	}
	//先发送文件名
	strcpy(buffer, afilename.c_str());
	send(clntSock, buffer, sizeof(buffer), 0);
	cout << "发送文件名：" << afilename.c_str() << endl;
	recv(clntSock, buffer, BUF_SIZE, 0);
	//循环发送数据，直到文件结尾
	int nCount;
	while ((nCount = fread(buffer, 1, BUF_SIZE, fp)) > 0) {
		send(clntSock, buffer, nCount, 0);
		recv(clntSock, buffer, BUF_SIZE, 0);	//确认cilent接收到上一条数据
	}
	fclose(fp);

	fp = fopen(filename, "rb");
	//计算MD5
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

	strcpy(buffer, "ThisFileIsEnd.");
	send(clntSock, buffer, sizeof(buffer), 0);
	recv(clntSock, buffer, BUF_SIZE, 0);
	cout << buffer << "  文件接收完毕." << endl;

	//发送MD5
	strcpy(buffer, result);
	send(clntSock, buffer, sizeof(buffer), 0);
	recv(clntSock, buffer, BUF_SIZE, 0);
	//存在误码,重新传输
	if (strcmp(buffer, "ErrorCode.") == 0)
	{
		cout << "传输出错，正在重传" << endl;
		fp = fopen(filename, "rb");  //以二进制方式打开文件
		//循环发送数据，直到文件结尾
		while ((nCount = fread(buffer, 1, BUF_SIZE, fp)) > 0) {
			send(clntSock, buffer, nCount, 0);
			recv(clntSock, buffer, BUF_SIZE, 0);	//确认cilent接收到上一条数据
		}
		fclose(fp);
		strcpy(buffer, "ThisFileIsEnd.");
		send(clntSock, buffer, sizeof(buffer), 0);
		recv(clntSock, buffer, BUF_SIZE, 0);
		cout << buffer << "  文件接收完毕." << endl;
	}
}*/

/*void folder() {
	getFiles(filenames, files, names, fatherpath);//获取文件夹下所有子文件对应的名称与路径，并发送文件夹名给cilent
	for (int i = 0; i < files.size(); i++)		//将所有文件逐个发送
	{
		singleFile(files[i].c_str(), names[i].c_str());
	}
}*/

