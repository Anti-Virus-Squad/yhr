i#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include<string>
#include<iostream>
#include<io.h>
#include<fstream>
#include<vector>
#include "md5.h"
#include <windows.h>
#include <tchar.h>
#include "zip.h"
using namespace std;

#pragma comment (lib, "ws2_32.lib")  //���� ws2_32.dll
#define BUF_SIZE 1024
#pragma warning(disable:4996) 

SOCKET servSock;
char buffer[BUF_SIZE] = { 0 };  //������
string filenames; string temp;
char extra_file[8][30] = { 0 };
void singleFile(const char* filename, string afilename);
void folder();
void getFiles(std::string path, std::vector<std::string>& files, std::vector<std::string>& names, string& fatherpath);
vector<string>files;		//���ļ������������ļ���ȫ·��
vector<string>names;		//���ļ������������ļ����ļ���
string fatherpath = "";		//���ڽ�ȡ�ļ�������
int choice;//1.��������Ҫ�ϴ������������ļ�·��'��'0.·��ȫ���������'
int main() {

	//�ȼ���ļ��Ƿ����
	cout << "������Ҫ�ϴ������������ļ�·����";
	getline(cin, filenames);

	cout << "������'1.��������Ҫ�ϴ������������ļ�·��'��'0.·��ȫ���������'��" << endl;
	cin >> choice;
	int total = 0;
	while (choice == 1 && total <= 7)
	{
		cout << "������Ҫ�ϴ������������ļ�·����" << endl;
		cin >> temp;
		strcpy(extra_file[total], temp.c_str());
		total++;
		cout << "������'1.��������Ҫ�ϴ������������ļ�·��'��'0.·��ȫ���������'��" << endl;
		cin >> choice;
	}

	
	/*�������ӵ�SOCKET*/
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
servSock = socket(AF_INET, SOCK_STREAM, 0);
	if (servSock < 0)
	{/*����ʧ�� */
		fprintf(stderr, "socker Error:%s\n", strerror(errno));
		exit(1);
	}

	/* ��ʼ���ͻ��˵�ַ*/
	sockaddr_in sockAddr;
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	//sockAddr.sin_addr.s_addr = inet_addr("192.168.0.106");
	sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);		//Ĭ�ϰ󶨱�������ip
	sockAddr.sin_port = htons(2345);					//��һ���˿ڣ��˴��Լ�ѡ�����2345
	if (bind(servSock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR)) < 0)
	{ /*����ʧ�� */
		fprintf(stderr, "Bind Error:%s\n", strerror(errno));
		system("pause");
		exit(1);
	}
	/*��ʼ����������ַ*/
	sockaddr_in ser_addr;
	
	ser_addr.sin_family = AF_INET;

	string server_ip;
	cout << "�������������ip��ַ��";
	cin >> server_ip;
	const char* server;
	server = server_ip.c_str();
	ser_addr.sin_addr.s_addr = inet_addr(server);

	//sockAddr.sin_addr.s_addr = inet_addr("192.168.0.106");

	int server_port;
	cout << "������������Ķ˿ںţ�";
	cin >> server_port;
	ser_addr.sin_port = htons(server_port);

	//sockAddr.sin_port = htons(1234);

	if (connect(servSock, (SOCKADDR*)&ser_addr, sizeof(SOCKADDR)) < 0)	//��������
	{/*����ʧ�� */
		fprintf(stderr, "Connect Error:%s\n", strerror(errno));
		closesocket(servSock);
		exit(1);
	}
	else
	{
		char p[2];
		p[0] = total + 1;
		send(servSock, p, 1, 0);
		recv(servSock, buffer, BUF_SIZE, 0);
		for (int num = 0; num <= total; num++) {

			if (num > 0)filenames = extra_file[num - 1];

			int flag = 1;//0��ʾ�ļ� 1��ʾ�ļ���
		//ͨ���Ƿ񺬡�.���ж����ļ��л����ļ���
			for (int i = 0; i < filenames.length(); i++)
			{
				if (filenames[i] == '.')
					flag = 0;
			}
			//����ǵ��ļ�
			if (flag == 0) {
				// ��\'��λ��
				int backslashIndex;
				// ʶ�����һ��'\'��λ�á�
				backslashIndex = filenames.find_last_of('\\');
				// ·����β�����ļ���
				singleFile(filenames.c_str(), filenames.substr(backslashIndex + 1, -1));
			}
			//�ļ���
			else {
				folder();
			}
			strcpy(buffer, "ThisSendIsEnd.");
			send(servSock, buffer, sizeof(buffer), 0);
			recv(servSock, buffer, BUF_SIZE, 0);
			names.clear();
			files.clear();
			//cout << buffer << "  �˴δ������." << endl;

			shutdown(servSock, SD_SEND);  //�ļ���ȡ��ϣ��Ͽ����������ͻ��˷���FIN��
			recv(servSock, buffer, BUF_SIZE, 0);  //�������ȴ��ͻ��˽������
			closesocket(servSock);

			WSACleanup();
			system("pause");
			return 0;
		}
	}

}

void singleFile(const char* filename, string afilename) {

	//ѡ���Ƿ���ѹ����
	string choice;
	cout << "ѡ��ѹ����������y����ѹ������n" << endl;
	cin >>choice;
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

	//�Զ����Ʒ�ʽ���ļ�
	FILE* fp = fopen(filename, "rb");  
	if (fp == NULL) {
		printf("Cannot open file, press any key to exit!\n");
		exit(0);
	}
	unsigned int filelength;
	fseek(fp, 0, SEEK_END);
	filelength = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char buf[4];
	memcpy_s(buf, 4, &filelength, 4);
	//�ȷ����ļ���
	strcpy(buffer, afilename.c_str());
	send(servSock, buffer, sizeof(buffer), 0);
	cout << "�����ļ�����" << afilename.c_str() << endl;
	
	recv(servSock, buffer, BUF_SIZE, 0);

	send(servSock, buf, sizeof(buf), 0);
	//��ȡλ��
	recv(servSock, buffer, 4, 0);
	unsigned int pos;
	memcpy_s(&pos, 4, buffer, 4);
	unsigned int se=0;
	//ѭ���������ݣ�ֱ���ļ���β
	int nCount;
	for(unsigned int i=pos;i<filelength;i+=se)
	{
		nCount=fread(buffer, 1, BUF_SIZE, fp);
		se=send(servSock, buffer, nCount, 0);
		if (se == SOCKET_ERROR || se == 0)
		{
			closesocket(servSock);
			cout << "����ʧ��" << endl;
			return;
		}
		recv(servSock, buffer, BUF_SIZE, 0);	//ȷ��cilent���յ���һ������
		
	}
	fclose(fp);

	fp = fopen(filename, "rb");
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

	strcpy(buffer, "ThisFileIsEnd.");
	send(servSock, buffer, sizeof(buffer), 0);
	recv(servSock, buffer, BUF_SIZE, 0);
	cout << buffer << "  �ļ��������." << endl;
	
	//����MD5
	strcpy(buffer, result);
	send(servSock, buffer, sizeof(buffer), 0);
	recv(servSock, buffer, BUF_SIZE, 0);
	//��������,���´���
	if (strcmp(buffer, "ErrorCode.") == 0)
	{
		cout << "������������ش�" << endl;
		fp = fopen(filename, "rb");  //�Զ����Ʒ�ʽ���ļ�
		recv(servSock, buffer, 4, 0);
		memcpy_s(&pos, 4, buffer, 4);
		//ѭ���������ݣ�ֱ���ļ���β
		for (unsigned int i = pos; i < filelength; i += se)
		{
			nCount = fread(buffer, 1, BUF_SIZE, fp);
			se = send(servSock, buffer, nCount, 0);
			if (se == SOCKET_ERROR || se == 0)
			{
				closesocket(servSock);
				cout << "����ʧ��" << endl;
				return;
			}
			recv(servSock, buffer, BUF_SIZE, 0);	//ȷ��cilent���յ���һ������

		}
		fclose(fp);
		strcpy(buffer, "ThisFileIsEnd.");
		send(servSock, buffer, sizeof(buffer), 0);
		recv(servSock, buffer, BUF_SIZE, 0);
		cout << buffer << "  �ļ��������." << endl;
	}
}

void folder() {
	getFiles(filenames, files, names, fatherpath);//��ȡ�ļ������������ļ���Ӧ��������·�����������ļ�������cilent
	for (int i = 0; i < files.size(); i++)		//�������ļ��������
	{
		singleFile(files[i].c_str(), names[i].c_str());
	}
}

void getFiles(std::string path, std::vector<std::string>& files, std::vector<std::string>& names, string& fatherpath)
{
	/*	path----�����ȫ·�� D:\sql���ڰ���һ��kkk���ļ��У�
		files----�����������ļ���ȫ·��
		names----����Ҫ���͸��ͻ��˵��ļ�������·����
		���磺sql\111.txt	sql\kkk\111.txt
		fatherpath----�����ļ��е����ƣ����ļ��д�·����
		���磺sql	sql\kkk
	*/

	string filefoldername;
	// ��\'��λ��
	int backslashIndex;
	// ʶ�����һ��'\'��λ�á�
	backslashIndex = path.find_last_of('\\');
	// ·����β�����ļ���
	filefoldername = path.substr(backslashIndex + 1, -1);
	fatherpath += filefoldername;
	cout << "�����ļ�������" << fatherpath << endl;
	strcpy(buffer, fatherpath.c_str());
	send(servSock, buffer, sizeof(buffer), 0);
	recv(servSock, buffer, BUF_SIZE, 0);	//���յ�cilent�Ļ�Ӧ

	//�ļ������win10��long long��win7��long�Ϳ�����
	long long hFile = 0;
	//�ļ���Ϣ 
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//�����Ŀ¼,����֮ //�������,�����б� 
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

