/*
任务分配程序
监测任务分配数据库情况，生成任务分配数据发送给相关的地面站接收端
*/

#include "assignment.h"

DWORD WINAPI assignment(LPVOID lpParameter) {
	const char* SERVER = MYSQL_SERVER.data();//连接的数据库ip
	const char* USERNAME = MYSQL_USERNAME.data();
	const char* PASSWORD = MYSQL_PASSWORD.data();
	const char DATABASE[20] = "satellite_message";
	const int PORT = 3306;
	while (1) {
		//5秒监测数据库的任务分配表
		Sleep(5000);
		//cout << "| 任务分配模块     | 监测数据库分配表..." << endl;
		MySQLInterface mysql;
		if (mysql.connectMySQL(SERVER, USERNAME, PASSWORD, DATABASE, PORT)) {
			string selectSql = "select 任务编号,任务类型,unix_timestamp(计划开始时间),unix_timestamp(计划截止时间),卫星编号,地面站编号, 任务状态 from 任务分配表 where 任务状态 = 1 || 任务状态 = 3;";
			vector<vector<string>> dataSet;//获取分配任务
			mysql.getDatafromDB(selectSql, dataSet);
			if (dataSet.size() == 0) {
				continue;//无任务静默5秒后继续查询
			}

			for (int i = 0,len = dataSet.size() ; i < len ; i++) {
				//报文数据接收空间
				char* messageDate;
				int messageDataSize = 0;
				if (dataSet[i][1]._Equal("101")) {
					//如果是遥控分配报文需要查报文数据
					string dadaSql = "select 任务内容 from 遥控内容表 where 任务编号 =" + dataSet[i][0];
					vector<vector<string>> dataBlob;
					mysql.getDatafromDB(dadaSql, dataBlob);
					if (dataBlob.size() == 0)continue;
					messageDataSize = dataBlob[0][0].size()+1;
					messageDate = new char[messageDataSize];//分配空间
					strcpy_s(messageDate, messageDataSize, dataBlob[0][0].c_str());
				}

				//转换工具
				StringNumUtils util;
				UINT16 messageId = 4010;//报文标识
				if (dataSet[i][6]._Equal("3"))messageId = 4020;//如果是撤销
				long long dateTime = Message::getSystemTime();//获取当前时间戳
				bool encrypt = false;//是否加密
				UINT32 taskNum = util.stringToNum<UINT32>(dataSet[i][0]);//任务编号
				UINT16 taskType = util.stringToNum<UINT16>(dataSet[i][1]);//任务类型
				long long taskStartTime = util.stringToNum<long long>(dataSet[i][2]);//计划开始时间
				long long taskEndTime = util.stringToNum<long long>(dataSet[i][3]);//计划截至时间
				char* satelliteId = new char[20];//卫星编号
				strcpy_s(satelliteId, dataSet[i][4].size()+1, dataSet[i][4].c_str());
				char* groundStationId = new char[20];//地面站编号
				strcpy_s(groundStationId, dataSet[i][5].size()+1, dataSet[i][5].c_str());
				AllocationMessage message (messageId, dateTime, encrypt, taskNum, taskType, taskStartTime, taskEndTime, satelliteId, groundStationId);
				if (messageDataSize > 0) { 
					message.setterMessage(messageDate, messageDataSize);
					delete messageDate;
				}
				//释放空间
				delete groundStationId;
				delete satelliteId;
				

				//查找地面站ip地址发送报文
				string groundStationSql = "select IP地址 from 地面站信息表 where 地面站编号 ='" + dataSet[i][5] + "';";
				vector<vector<string>> ipSet;
				mysql.getDatafromDB(groundStationSql, ipSet);
				if (ipSet.size() == 0)continue;//没有找到ip地址
				Socket socketer;
				const char* ip = ipSet[0][0].c_str();//获取到地址
				if (!socketer.createSendServer(ip, 4998, 0))continue;
				const int bufSize = 66560;//65K发送
				int returnSize = 0;
				char sendBuf[bufSize];//申请发送buf
				ZeroMemory(sendBuf, bufSize);//置0
				message.createMessage(sendBuf, returnSize, bufSize);
				if (socketer.sendMessage(sendBuf, bufSize) == -1)continue;

				////写日志
				//string sql_date = "insert into 系统日志表 (对象,事件类型,参数) values ('任务分配模块',15000,'";
				//sql_date = sql_date + dataSet[i][0] + "号任务分配成功');";
				//mysql.writeDataToDB(sql_date);

				socketer.offSendServer();
				//修改数据库分发标志
				string ackSql = "update 任务分配表 set 任务状态 = 2 where 任务编号 = " + dataSet[i][0];
				string logSql  = "insert into 系统日志表 (时间,对象,事件类型,参数) values (now(),'任务分配模块',15000,'"+dataSet[i][0] + "号任务分配成功 ');";
				if (dataSet[i][6]._Equal("3")) { 
					ackSql = "update 任务分配表 set 任务状态 = 4 where 任务编号 = " + dataSet[i][0]; 
					cout << "| 任务分配模块     | " << dataSet[i][0] << "号任务撤销发布" << endl;
					logSql = "insert into 系统日志表 (时间,对象,事件类型,参数) values (now(),'任务分配模块',15000,'" + dataSet[i][0] + "号任务撤销发布 ');";
				}
				else {
					cout << "| 任务分配模块     | " << dataSet[i][0] << "号任务分配成功" << endl;
				}
				mysql.writeDataToDB(ackSql);
				mysql.writeDataToDB(logSql);
				
			}
			mysql.closeMySQL();
		}
		else {
			cout << "| 任务分配         | 连接数据库失败" << endl;
			cout << "| 任务分配错误信息 | " << mysql.errorNum << endl;
		}
		cout << "| 任务分配         | 任务分配结束" << endl;
		
	}
	
}
