#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include<string>
#include<iostream>
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

SOCKET sock;
SOCKADDR clntAddr;
char buffer[BUF_SIZE] = { 0 };  //缓冲区
string filenames;
void singleFile(const char* filename, string afilename);
void folder();
void getFiles(std::string path, std::vector<std::string>& files, std::vector<std::string>& names, string& fatherpath);
vector<string>files;		//存文件夹内所有子文件的全路径
vector<string>names;		//存文件夹内所有子文件的文件名
string fatherpath = "";		//用于截取文件夹名称
int trans_flag = 1;//判断是否继续上传文件
int main() {

	//先检查文件是否存在
	cout << "请输入要上传到服务器的文件路径：";
	getline(cin, filenames);
	int flag = 1;//0表示文件 1表示文件夹
	

	/*创建连接的SOCKET */
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

	/*while (trans_flag == 1)*/
	{

		//通过是否含“.”判断是文件夹还是文件名
		for (int i = 0; i < filenames.length(); i++)
		{
			if (filenames[i] == '.')
				flag = 0;
			else flag = 1;
		}
		//如果是单文件
		if (flag == 0) {
			// ‘\'的位置
			int backslashIndex;
			// 识别最后一个'\'的位置。
			backslashIndex = filenames.find_last_of('\\');
			// 路径名尾部是文件名
			singleFile(filenames.c_str(), filenames.substr(backslashIndex + 1, -1));
		}
		//文件夹
		else {
			folder();
		}
		strcpy(buffer, "ThisSendIsEnd.");
		send(sock, buffer, sizeof(buffer), 0);
		recv(sock, buffer, BUF_SIZE, 0);
		cout << buffer << "  此次传输完毕." << endl;

		//清空容器
		files.clear();
		names.clear();

		//如果要删去循环只需去掉while语句和下面7行
		/*cout<<	"请继续输入需要传输的文件路径或输入0退出" << endl;
		getline(cin, filenames);
		if (strcmp(filenames.c_str(), "0") == 0) {
			trans_flag = 0;
			send(sock,"0" ,1,0 );//跳出循环并发送0
		}
		else send(sock, "1",1,0);//收到其他继续
		recv(sock, buffer, BUF_SIZE, 0);*/
	}
	shutdown(sock, SD_SEND);  //文件读取完毕，断开输出流，向客户端发送FIN包
	recv(sock, buffer, BUF_SIZE, 0);  //阻塞，等待客户端接收完毕
	closesocket(sock);

	WSACleanup();
	system("pause");
	return 0;
	
}
void singleFile(const char* filename, string afilename) {
	/*
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
	}*/

	//以二进制方式打开文件
	FILE* fp = fopen(filename, "rb");
	if (fp == NULL) {
		printf("Cannot open file, press any key to exit!\n");
		exit(0);
	}
	//先发送文件名
	strcpy(buffer, afilename.c_str());
	send(sock, buffer, sizeof(buffer), 0);
	cout << "发送文件名：" << afilename.c_str() << endl;
	recv(sock, buffer, BUF_SIZE, 0);
	//循环发送数据，直到文件结尾
	int nCount;
	while ((nCount = fread(buffer, 1, BUF_SIZE, fp)) > 0) {
		send(sock, buffer, nCount, 0);
		recv(sock, buffer, BUF_SIZE, 0);	//确认cilent接收到上一条数据
	}
	fclose(fp);
	strcpy(buffer, "ThisFileIsEnd.");
	send(sock, buffer, sizeof(buffer), 0);
	recv(sock, buffer, BUF_SIZE, 0);
	cout << buffer << "  文件接收完毕." << endl;//********************************修改时注意send和recv的第一个参数均改为sock
	/*fp = fopen(filename, "rb");
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
	}*/
}

void folder() {
	getFiles(filenames, files, names, fatherpath);//获取文件夹下所有子文件对应的名称与路径，并发送文件夹名给cilent
	for (int i = 0; i < files.size(); i++)		//将所有文件逐个发送
	{
		singleFile(files[i].c_str(), names[i].c_str());
	}
}

void getFiles(std::string path, std::vector<std::string>& files, std::vector<std::string>& names, string& fatherpath)
{
	/*	path----输入的全路径 D:\sql（内包括一个kkk子文件夹）
		files----存着所有子文件的全路径
		names----存着要发送给客户端的文件名（带路径）
		比如：sql\111.txt	sql\kkk\111.txt
		fatherpath----存着文件夹的名称（子文件夹带路径）
		比如：sql	sql\kkk
	*/

	string filefoldername;
	// ‘\'的位置
	int backslashIndex;
	// 识别最后一个'\'的位置。
	backslashIndex = path.find_last_of('\\');
	// 路径名尾部是文件名
	filefoldername = path.substr(backslashIndex + 1, -1);
	fatherpath += filefoldername;
	cout << "发送文件夹名：" << fatherpath << endl;
	strcpy(buffer, fatherpath.c_str());
	send(sock, buffer, sizeof(buffer), 0);
	recv(sock, buffer, BUF_SIZE, 0);	//接收到cilent的回应

	//文件句柄，win10用long long，win7用long就可以了
	long long hFile = 0;
	//文件信息 
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之 //如果不是,加入列表 
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					string subfatherpath = fatherpath + "\\";
					//getFiles(p.assign(path).append("\\").append(fileinfo.name), files, names, fatherpath.append("\\"));
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files, names, subfatherpath);
				}
			}
			else
			{
				string filename = fatherpath + "\\" + fileinfo.name;
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
				names.push_back(filename);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}
