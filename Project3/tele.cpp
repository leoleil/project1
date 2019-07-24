#define _CRT_SECURE_NO_WARNINGS 1
#include "tele.h"
vector<message_buf> DATA_MESSAGES;//全局上传数据报文池
CRITICAL_SECTION data_CS;//数据线程池关键代码段对象

string getTime()//获取时间字符化输出
{
	time_t timep;
	time(&timep); //获取time_t类型的当前时间
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));//对日期和时间进行格式化
	return tmp;
}
string getType(UINT16 num) {
	switch (num)
	{
	case 1:
		return "INT";
	case 2:
		return "INT";
	case 3:
		return "INT";
	case 4:
		return "INT";
	case 5:
		return "BIGINT";
	case 6:
		return "BIGINT";
	case 7:
		return "FLOAT";
	case 8:
		return "DOUBLE";
	case 9:
		return "VARCHAR(1)";
	case 10:
		return "VARCHAR(255)";
	case 11:
		return "VARCHAR(255)";
	case 12:
		return "BLOB";
	case 13:
		return "BLOB";
	case 14:
		return "BLOB";
	case 15:
		return "BOOL";
	case 16:
		return "BIGINT";
	default:
		return "";
	}
}

DWORD message_rec(LPVOID lpParameter)
{
	InitializeCriticalSection(&data_CS);//初始化关键代码段对象
	HANDLE hThread1;//创建数据解析线程，读取数据池中数据
	hThread1 = CreateThread(NULL, 0, message_pasing, NULL, 0, NULL);
	CloseHandle(hThread1);
	while (1) {
		TeleSocket service;//创建接收任务服务
		service.createReceiveServer(4996, DATA_MESSAGES);
	}
	DeleteCriticalSection(&data_CS);//删除关键代码段对象
	return 0;
}

DWORD message_pasing(LPVOID lpParameter)
{
	const char* SERVER = MYSQL_SERVER.data();//连接的数据库ip
	const char* USERNAME = MYSQL_USERNAME.data();
	const char* PASSWORD = MYSQL_PASSWORD.data();
	const char DATABASE[20] = "satellite_teledata";
	const char DATABASE_2[20] = "satellite_message";
	const int PORT = 3306;
	unordered_map<string, int> MAP;//用来标记第一个来的数据
	while (1) {
		Sleep(100);
		EnterCriticalSection(&data_CS);//进入关键代码段
		if (DATA_MESSAGES.empty()) {
			LeaveCriticalSection(&data_CS);
			continue;
		}
		//报文集合大小
		int setLen = DATA_MESSAGES.size();
		LeaveCriticalSection(&data_CS);//离开关键代码段

		for (int i = 0; i < setLen; i++) {
			EnterCriticalSection(&data_CS);//进入关键代码段
			char byte_data[70 * 1024];//每个报文空间最大70K
			memcpy(byte_data, DATA_MESSAGES[i].val, 70 * 1024);//将报文池中数据取出
			LeaveCriticalSection(&data_CS);//离开关键代码段
			UINT16 type;//报文类型
			UINT16 length;//报文长度
			memcpy(&type, byte_data, 2);
			memcpy(&length, byte_data + 2, 4);
			//解析数据
			if (type == 1000) {//定义报文
							   //解析定义报文
							   //解析指针
				int ptr = 6;
							//时间戳
				long long timestamp;
				memcpy(&timestamp, byte_data + ptr, sizeof(long long));
				ptr = ptr + sizeof(long long);
				//加密标识
				bool flag;
				memcpy(&flag, byte_data + ptr, 1);
				ptr = ptr + 1;
				//任务编号
				UINT32 taskNum;
				memcpy(&taskNum, byte_data + ptr, sizeof(UINT32));
				ptr = ptr + sizeof(UINT32);
				//设备名
				char name[40];
				memcpy(name, byte_data + ptr, 40);

				//写入日志
				MySQLInterface date_db;
				if (date_db.connectMySQL(SERVER, USERNAME, PASSWORD, DATABASE_2, PORT)) {
					string sql_difinition = "insert into 系统日志表 (时间,对象,事件类型,参数) values (now(),'遥测通信模块',11010,'通信中心机收到";
					sql_difinition = sql_difinition + name + "遥测报表定义报文');";
					date_db.writeDataToDB(sql_difinition);
					date_db.closeMySQL();
				}

				cout << "| 卫星遥测         | ";
				cout << getTime();
				cout << "| 接收定义报文";
				cout.setf(ios::left);
				cout.width(37);
				cout << name;
				cout << "|" << endl;
				MAP[name] = 0;//标记设备
				ptr = ptr + 40;
				//父设备名
				char parentName[40];
				memcpy(parentName, byte_data + ptr, 40);
				ptr = ptr + 40;
				//卫星编号
				char satillitId[20];
				memcpy(satillitId, byte_data + ptr, 20);
				ptr = ptr + 20;
				//识别字段名称
				vector<field> fields;
				while (ptr < length) {
					field sub_field;
					//字段名
					memcpy(sub_field.name, byte_data + ptr, 40);
					ptr = ptr + 40;
					//字段类型编号
					memcpy(&sub_field.type, byte_data + ptr, 2);
					ptr = ptr + 2;
					//最小值
					memcpy(&sub_field.min, byte_data + ptr, 8);
					ptr = ptr + 8;
					//最大值
					memcpy(&sub_field.max, byte_data + ptr, 8);
					ptr = ptr + 8;
					//单位
					memcpy(sub_field.unit, byte_data + ptr, 8);
					ptr = ptr + 8;
					//显示标识
					memcpy(&sub_field.display, byte_data + ptr, 1);
					ptr = ptr + 1;
					fields.push_back(sub_field);
				}
				//入库操作

				MySQLInterface db;
				//连接数据库
				if (db.connectMySQL(SERVER, USERNAME, PASSWORD, DATABASE, PORT)) {
					//查询是否有该定义字段
					string f_sql = "select * from 字段定义表 where 设备名 =";
					f_sql = f_sql + "'" + name + "'and 卫星编号='"+ satillitId+"';";
					vector<vector <string>>res;
					if (!db.getDatafromDB(f_sql, res)) {
						cout << db.errorNum << endl;
						cout << db.errorInfo << endl;
					}
					if (res.size() != 0) {
						cout << name << "设备在定义表中已经存在" << endl;
						//将该报文删除
						EnterCriticalSection(&data_CS);//进入关键代码段
													   //报文池-1
						DATA_MESSAGES.erase(DATA_MESSAGES.begin() + i);
						i--;
						LeaveCriticalSection(&data_CS);//离开关键代码段

					}
					else {
						//不存在则创建

						//创建字段定义
						for (int i = 0; i < fields.size(); i++) {
							//创建定义的sql语句
							string sql = "insert into 字段定义表 (字段名,数据类型,最大值,最小值,单位,显示标志,设备名,卫星编号) values(";
							sql = sql + "'" + fields[i].name + "',";
							sql = sql + to_string(fields[i].type) + ",";
							//如果没有最大最小值
							if (fields[i].max == NEW_NULL) {
								sql = sql + "NULL,";
							}
							else {
								sql = sql + to_string(fields[i].max) + ",";
							}
							if (fields[i].min == NEW_NULL) {
								sql = sql + "NULL,";
							}
							else {
								sql = sql + to_string(fields[i].min) + ",";
							}

							sql = sql + "'" + fields[i].unit + "',";
							sql = sql + to_string(fields[i].display) + ",";
							sql = sql + "'" + name + "','"+satillitId+"');";
							if (!db.writeDataToDB(sql)) {
								cout << db.errorNum << endl;
								cout << db.errorInfo << endl;
							}
						}
						//创建该数据的表
						string d_sql = "create table ";
						d_sql = d_sql+ satillitId+"_" + name + " (主键 INT AUTO_INCREMENT,时间 BIGINT,";
						for (int i = 0; i < fields.size(); i++) {
							d_sql = d_sql + fields[i].name + " " + getType(fields[i].type);
							if (i != fields.size() - 1)d_sql = d_sql + ",";
						}
						d_sql = d_sql + ",primary key(主键));";
						//cout << d_sql << endl;
						if (!db.writeDataToDB(d_sql)) {
							//cout << db->errorNum << endl;
							//cout << db->errorInfo << endl;
						}
						//创建关系表
						string r_sql = "insert into 设备关系表(设备名,父设备名,卫星编号) values('";
						r_sql = r_sql + name + "','" + parentName + "','"+ satillitId +"');";
						if (!db.writeDataToDB(r_sql)) {
							//cout << db->errorNum << endl;
							//cout << db->errorInfo << endl;
						}
						
						//将该报文删除
						EnterCriticalSection(&data_CS);//进入关键代码段
													//报文池-1
						DATA_MESSAGES.erase(DATA_MESSAGES.begin() + i);
						i--;
						LeaveCriticalSection(&data_CS);//离开关键代码段

					}
					//关闭连接
					db.closeMySQL();
				}
				else {
					//cout << db->errorNum << endl;
					//cout << db->errorInfo << endl;
				}

			}
			else if (type == 2000) {
				//解析数据报文
				//解析指针
				int ptr = 6;
							//时间戳
				long long timestamp;
				memcpy(&timestamp, byte_data + ptr, sizeof(long long));
				ptr = ptr + sizeof(long long);
				//加密标识
				bool flag;
				memcpy(&flag, byte_data + ptr, sizeof(bool));
				ptr = ptr + sizeof(bool);
				UINT32 taskNum;
				memcpy(&taskNum, byte_data + ptr, sizeof(UINT32));
				ptr = ptr + sizeof(UINT32);
				//设备名
				char name[40];
				memcpy(name, byte_data + ptr, 40);
				ptr = ptr + 40;

				//写日志
				if (MAP[name] == 0) {
					MAP[name] = 1;
					MySQLInterface date_db;
					if (date_db.connectMySQL(SERVER, USERNAME, PASSWORD, DATABASE_2, PORT)) {
						string sql_data = "insert into 系统日志表 (时间,对象,事件类型,参数) values (now(),'遥测通信模块',11011,'通信中心机收到第一份";
						sql_data = sql_data + name + "遥测报表数据报文');";
						cout << "| 通信模块 | ";
						cout << getTime();
						cout << "| 接收数据报文";
						cout.setf(ios::left);
						cout.width(37);
						cout << name;
						cout << "|" << endl;
						date_db.writeDataToDB(sql_data);
						date_db.closeMySQL();
					}
					
				}
				//卫星编号
				char satillitId[20];
				memcpy(satillitId, byte_data + ptr, 20);
				ptr = ptr + 20;
				//采样时间
				long long getTime;
				memcpy(&getTime, byte_data + ptr, sizeof(long long));
				ptr = ptr + sizeof(long long);
				//如果数据库有此字段
				//识别数据入库
				MySQLInterface db;
				//连接数据库
				if (db.connectMySQL(SERVER, USERNAME, PASSWORD, DATABASE, PORT)) {

					//入库操作
					//查询字段数据类型
					vector<vector <string>>res;
					string ff_sql = "select 数据类型,字段名 from 字段定义表 where 设备名=";
					ff_sql = ff_sql + "'" + name + "'and 卫星编号='"+ satillitId +"' order by id;";
					if (db.getDatafromDB(ff_sql, res)) {
						if (res.size() == 0) {
							cout << name << "设备不存在" << endl;
						}
						//数据库中数据
						else {
							string sql = "insert into ";
							sql = sql + satillitId + "_" + name + "(时间,";
							string d_sql = " values(";
							d_sql = d_sql + to_string(timestamp) + ",";
							for (int i = 0; i < res.size(); i++) {
								//类型号码
								UINT16 code = 0;
								StringNumUtils util;
								code = util.stringToNum<UINT16>(res[i][0]);
								sql = sql + res[i][1];
								if (i < res.size() - 1)sql = sql + ",";
								if (i == res.size() - 1)sql = sql + ")";
								if (code == 1) {//16位整数
									INT16 data = 0;
									memcpy(&data, byte_data + ptr, sizeof(INT16));
									ptr = ptr + sizeof(INT16);
									d_sql = d_sql + to_string(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 2) {//16位无符号整数
									UINT16 data = 0;
									memcpy(&data, byte_data + ptr, sizeof(UINT16));
									ptr = ptr + sizeof(UINT16);
									d_sql = d_sql + to_string(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 3) {//32位整数
									INT32 data = 0;
									memcpy(&data, byte_data + ptr, sizeof(INT32));
									ptr = ptr + sizeof(INT32);
									d_sql = d_sql + to_string(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 4) {//32位无符号整数
									UINT32 data = 0;
									memcpy(&data, byte_data + ptr, sizeof(UINT32));
									ptr = ptr + sizeof(UINT32);
									d_sql = d_sql + to_string(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 5) {//64位整数
									INT64 data = 0;
									memcpy(&data, byte_data + ptr, sizeof(INT64));
									ptr = ptr + sizeof(INT64);
									d_sql = d_sql + to_string(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 6) {//64位无符号整数
									UINT64 data = 0;
									memcpy(&data, byte_data + ptr, sizeof(UINT64));
									ptr = ptr + sizeof(UINT64);
									d_sql = d_sql + to_string(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 7) {//单精度浮点数值
									float data = 0;
									memcpy(&data, byte_data + ptr, sizeof(float));
									ptr = ptr + sizeof(float);
									d_sql = d_sql + to_string(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 8) {//双精度浮点数值
									double data = 0;
									memcpy(&data, byte_data + ptr, sizeof(double));
									ptr = ptr + sizeof(double);
									d_sql = d_sql + to_string(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 9) {//字符类型
									char data;
									memcpy(&data, byte_data + ptr, sizeof(char));
									ptr = ptr + sizeof(char);
									d_sql = d_sql + "'" + data + "'";
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 10) {//短字符串类型
									char data[15];
									memcpy(&data, byte_data + ptr, sizeof(char) * 15);
									ptr = ptr + sizeof(char) * 15;
									d_sql = d_sql + "'" + data + "'";
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 11) {//长字符串类型
									char data[255];
									memcpy(&data, byte_data + ptr, sizeof(char) * 255);
									ptr = ptr + sizeof(char) * 255;
									d_sql = d_sql + "'" + data + "'";
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								//有bug
								else if (code == 12) {//短字节数组
									char data[255];
									memcpy(&data, byte_data + ptr, sizeof(char) * 255);
									ptr = ptr + sizeof(char) * 255;
									d_sql = d_sql + "'" + data + "'";
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 13) {//中字节数组
									char data[32 * K];
									memcpy(&data, byte_data + ptr, sizeof(char) * 32 * K);
									ptr = ptr + sizeof(char) * 32 * K;
									d_sql = d_sql + "'" + data + "'";
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 14) {//长字节数组
									char data[60 * K];
									memcpy(&data, byte_data + ptr, sizeof(char) * 60 * K);
									ptr = ptr + sizeof(char) * 60 * K;
									d_sql = d_sql + "'" + data + "'";
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 15) {//布尔型
									bool data;
									memcpy(&data, byte_data + ptr, sizeof(bool));
									ptr = ptr + sizeof(bool);
									d_sql = d_sql + to_string(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else if (code == 16) {//时间戳
									long long data;
									memcpy(&data, byte_data + ptr, sizeof(long long));
									ptr = ptr + sizeof(long long);
									d_sql = d_sql + to_string(data);
									if (i != res.size() - 1)d_sql = d_sql + ",";
								}
								else {
									cout << "未定义数据类型错误" << endl;
								}
							}
							d_sql = d_sql + ");";
							sql = sql + d_sql;
							//cout << sql << endl;
							if (!db.writeDataToDB(sql)) {
								//cout << db->errorNum << endl;
								//cout << db->errorInfo << endl;
							}
							//Sleep(10);
							EnterCriticalSection(&data_CS);//进入关键代码段
														//报文池-1
							DATA_MESSAGES.erase(DATA_MESSAGES.begin() + i);
							i--;
							LeaveCriticalSection(&data_CS);//离开关键代码段
						}

					}
					else {
						//cout << db->errorNum << endl;
						//cout << db->errorInfo << endl;
					}
					//关闭连接
					db.closeMySQL();
				}
				else {
					//cout << db->errorNum << endl;
					//cout << db->errorInfo << endl;
				}
			}
			else {
				
			}

			//更新报文池的大小
			//Sleep(1);
			EnterCriticalSection(&data_CS);//进入关键代码段
			setLen = (int)DATA_MESSAGES.size();
			//system("cls");
			//cout << "目前报文池大小：" << MESSAGE_VECTOR.size() << endl;
			LeaveCriticalSection(&data_CS);//离开关键代码段
		}

	}
	return 0;
}
