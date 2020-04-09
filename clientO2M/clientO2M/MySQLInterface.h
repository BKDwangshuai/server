#pragma once
#ifndef __MYSQL_INTERFACE_H__
#define __MYSQL_INTERFACE_H__

#include "winsock.h"
#include <iostream>
#include <string>
#include <mysql.h>
#include <vector>
#include <string>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libmysql.lib")
using namespace std;

class MySQLInterface
{
public:
	MySQLInterface();
	virtual ~MySQLInterface();

	bool connectMySQL(char* server, char* username, char* password, char* database, int port);
	bool createDatabase(std::string& dbname);
	bool createdbTable(const std::string& query);

	void errorIntoMySQL();
	bool writeDataToDB(string queryStr);
	bool getDatafromDB(string queryStr, std::vector<std::vector<std::string> >& data);
	void closeMySQL();

public:
	int errorNum;                    //�������
	const char* errorInfo;             //������ʾ

public:
	MYSQL mysqlInstance;                      //MySQL���󣬱ر���һ�����ݽṹ
	MYSQL_RES *result;                 //���ڴ�Ž�� ������char* ���齫�˽��ת��
};

#endif
