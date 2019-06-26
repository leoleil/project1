#include "cancelAssignment.h"

DWORD cancelAssigement(LPVOID lpParameter)
{
	const char SERVER[10] = "127.0.0.1";
	const char USERNAME[10] = "root";
	const char PASSWORD[10] = "";
	const char DATABASE[20] = "satellite";
	const int PORT = 3306;
	while (1) {
		//5秒监测数据库的任务分配表
		Sleep(5000);
		MySQLInterface* mysql = new MySQLInterface();
		if (mysql->connectMySQL(SERVER, USERNAME, PASSWORD, DATABASE, PORT)) {
			string selectSql = "select 任务编号 from 任务分配表 where 分发标志 = 3";
			vector<vector<string>> dataSet;//获取分配任务
			mysql->getDatafromDB(selectSql, dataSet);
			if (dataSet.size() == 0) {
				continue;//无要撤销任务静默5秒后继续查询
			}
			for (int i = 0, len = dataSet.size(); i < len; i++) {
				char* messageDate;
				StringNumUtils * util = new StringNumUtils();

				UINT16 messageId = 4010;//报文标识
				long long dateTime = Message::getSystemTime();//获取当前时间戳
				bool encrypt = false;//是否加密
				UINT32 taskNum = util->stringToNum<UINT32>(dataSet[i][0]);//任务编号
				UINT16 taskType = util->stringToNum<UINT16>(dataSet[i][1]);//任务类型
				long long taskStartTime = util->stringToNum<long long>(dataSet[i][2]);//计划开始时间
				long long taskEndTime = util->stringToNum<long long>(dataSet[i][3]);//计划截至时间
				char* satelliteId = new char[20];//卫星编号
				strcpy_s(satelliteId, dataSet[i][4].size() + 1, dataSet[i][4].data());
				char* groundStationId = new char[20];//地面站编号
													 //dataSet[i][5].copy(groundStationId, dataSet[i][5].size(), 0);
				strcpy_s(groundStationId, dataSet[i][5].size() + 1, dataSet[i][5].data());
				AllocationMessage * message = new AllocationMessage(messageId, dateTime, encrypt, taskNum, taskType, taskStartTime, taskEndTime, satelliteId, groundStationId);

				//查找地面站ip地址发送报文
				string groundStationSql = "select IP地址 from 地面站信息表 where 地面站编号 =" + dataSet[i][5];
				vector<vector<string>> ipSet;
				mysql->getDatafromDB(groundStationSql, ipSet);
				if (ipSet.size() == 0)continue;//没有找到ip地址
				Socket* socketer = new Socket();
				const char* ip = ipSet[0][0].data();//获取到地址
				if (socketer->createSendServer(ip, 9000, 0))continue;
				const int bufSize = message->getterMessageLength();
				int returnSize = 0;
				char* sendBuf = new char[bufSize];//申请发送buf
				message->createMessage(sendBuf, returnSize, bufSize);
				if (socketer->sendMessage(sendBuf, returnSize) == -1)continue;
				cout << "| 任务分配         | " << dataSet[i][0] << "号任务分配成功" << endl;

				//修改数据库分发标志
				string ackSql = "update 任务分配表 set 分发标志 = 2 where task_number = " + dataSet[i][0];
				mysql->writeDataToDB(ackSql);

			}

		}
		else {
			cout << "| 任务分配         | 连接数据库失败" << endl;
			cout << "| 任务分配错误信息 | " << mysql->errorNum << endl;
		}
		cout << "| 任务分配         | 任务分配结束" << endl;

	}
}
