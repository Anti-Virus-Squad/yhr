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
char buffer[BUF_SIZE];  //�ļ�������
void sendfile(const char* filename, string logname, SOCKET sock,unsigned int filelength);
int main() {

	//�������ļ�����·��
	string in_filepath;
	printf("����Ҫ�����ļ���·��: ");
	getline(cin, in_filepath);
	const char* filepath;  //�ļ�����·��
	filepath = in_filepath.c_str();

	/*�������ӵ�SOCKET */
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0)
	{/*����ʧ�� */
		fprintf(stderr, "socker Error:%s\n", strerror(errno));
		exit(1);
	}

	/*��ʼ����������ַ*/
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;

	string server_ip;
	cout << "�������������ip��ַ��";
	cin >> server_ip;
	const char* server;
	server = server_ip.c_str();
	sockAddr.sin_addr.s_addr = inet_addr(server);

	//sockAddr.sin_addr.s_addr = inet_addr("192.168.0.106");

	int server_port;
	cout << "������������Ķ˿ںţ�";
	cin >> server_port;
	sockAddr.sin_port = htons(server_port);

	if (bind(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR)) < 0)
	{/*��ʧ��*/
		fprintf(stderr, "Bind Error:%s\n", strerror(errno));
		exit(1);
	}

	/*�����ͻ�������*/
	if (listen(sock, 20) < 0)
	{
		fprintf(stderr, "Listen Error:%s\n", strerror(errno));
		closesocket(sock);
		exit(1);
	}

	cout << "�ȴ��ͻ��˽���..." << endl;

	/* �ȴ����տͻ���������*/
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
		char buffer[BUF_SIZE];  //�ļ�������
		//���շ��͹�����ԭ�ļ�·��
		recv(clntSock, buffer, BUF_SIZE, 0);
		cout << "�յ���" << buffer << endl;

		int flag = 0;	//0----���͵����ļ��������ƣ�����.����1----���͵����ļ�����(��.)
		for (int i = 0; buffer[i] != '\0'; i++)
		{
			if (buffer[i] == '.')
				flag = 1;
		}
		while (flag == 0)	//���͵����ļ������ƣ��ж��Ƿ��д��ļ��У�û�оʹ���һ��
		{
			//���Լ������·��ƴ�ӳɱ����ļ���·��
			char filefoldername[200] = { 0 };
			strcpy(filefoldername, filepath);
			strcat(filefoldername, "\\");
			strcat(filefoldername, buffer);

			if (0 != access(filefoldername, 0))
			{
				//�����ڴ��ļ��У����´���һ��
				mkdir(filefoldername);
				cout << "�����ļ��У�" << filefoldername << endl;
				strcpy(filefoldername, "\0");
			}

			//��֪����ˣ������յ��˴���Ϣ
			strcpy(buffer, "CilentGet");
			send(clntSock, buffer, sizeof(buffer), 0);

			//������һ����Ϣ���ж����ļ����������ļ���
			recv(clntSock, buffer, BUF_SIZE, 0);
			//cout << "�յ���" << buffer << endl;
			for (int i = 0; buffer[i] != '\0'; i++)
			{
				if (buffer[i] == '.')
					flag = 1;
			}
		}
		while (flag == 1)		//���͵����ļ���
		{
			//�������·��ƴ�ӳɱ����ļ�·��
			char filename_path[200] = { 0 };
			strcpy(filename_path, filepath);
			strcat(filename_path, "\\");
			strcat(filename_path, buffer);

			FILE* fp = fopen(filename_path, "wb");  //�Զ����Ʒ�ʽ�򿪣��������ļ�
			string filename = filename_path;
			// ��\'��λ��
			int backslashIndex;
			// ʶ�����һ��'\'��λ�á�
			backslashIndex = filename.find_last_of('.');
			//string logname = filename.substr(0,backslashIndex)+".log";
			string logname = filename + ".log";
			//��֪����������յ����ļ���
			strcpy(buffer, "CilentGet");
			send(clntSock, buffer, sizeof(buffer), 0);
			unsigned int filelength;
			char buf[4];
			memset(buf, 0, 4);
			recv(clntSock, buf, 4, 0);
			memcpy_s(&filelength, 4, buf, 4);
			sendfile(filename_path, logname, clntSock, filelength);

			fp = fopen(filename_path, "rb");
			//����MD5
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

			//����MD5
			recv(clntSock, buffer, BUF_SIZE, 0);
			char trueresult[50] = { 0 };
			strcpy(trueresult, buffer);
			if (strcmp(trueresult, result) == 0) {
				cout << "�����ļ�����" << endl;
				strcpy(buffer, "TransmissionCorrect.");
				send(clntSock, buffer, sizeof(buffer), 0);
			}
			else {
				cout << "�����ļ�" << filename_path << "���󣬳������´���" << endl;
				strcpy(buffer, "ErrorCode.");
				send(clntSock, buffer, sizeof(buffer), 0);
				char* log = const_cast<char*>(logname.c_str());
				remove(log);//ɾ���ļ�
				remove(filename_path);
				sendfile(filename_path, logname, clntSock, filelength);
				//����MD5
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
					cout << "�����ļ�����" << endl;
				else {
					cout << "�ļ�" << filename_path << "����ʧ��" << endl;
				}
			}


			recv(clntSock, buffer, BUF_SIZE, 0);	//���մ��ļ���������һ����Ϣ
			//cout << "�յ���" << buffer << endl;
			flag = 0;
			for (int i = 0; buffer[i] != '\0'; i++)	//�ж���Ϣ�Ƿ��.(�ļ���־)
			{
				if (buffer[i] == '.')
					flag = 1;
			}
			if (strcmp(buffer, "ThisSendIsEnd.") == 0)//�жϴ����Ƿ����
			{
				flag = 0;
				strcpy(buffer, "CilentRecivedAllFile.");
				send(clntSock, buffer, sizeof(buffer), 0);
			}
		}
		puts("�ļ��������");
	}
	//�ļ�������Ϻ�ֱ�ӹر��׽��֣��������shutdown()
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
	
	//ѭ���������ݣ�ֱ���ļ�������ϣ�ÿ�յ�һ���ļ����ݱ�ش�һ��"CilentRecivedDATA."��Ϣ��
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
		cout << "�յ���" << buffer << endl;
		strcpy(buffer, "CilentRecivedFile.");
		send(sock, buffer, sizeof(buffer), 0);
		
	}
}