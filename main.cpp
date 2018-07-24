/*
 * main.cpp
 *
 *  Created on: 2018年7月24日
 *      Author: garvin
 */

#include "ping.h"

using namespace std;

int main(){
	//记录ip或host
	string host_or_ip = "";
	//记录已发送packet和已接受packet,用于统计丢包率
	int has_sent = 0, has_received = 0;
	//用于记录ping函数的返回值
	bool respond = false;
	//用于记录ping操作所得数据的结构体
	ping_result pingresult;
	//生成一个ping对象
	Ping ping = Ping();
	cout<<"请输入ip地址或域名"<<endl;
	cin >> host_or_ip;

	for(int i=0; i<4; i++){
		respond = ping.ping(host_or_ip, 1, pingresult);
	}
}


