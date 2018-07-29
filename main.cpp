/*
 * main.cpp
 *
 *  Created on: 2018年7月24日
 *      Author: garvin
 */

#include "ping.h"

using namespace std;

void show_pingresult(const ping_result& pingresult){
	for (int i = 0; i < pingresult.IcmpEchoReplys.size(); i++) {
		icmp_echo_reply reply = pingresult.IcmpEchoReplys[i];
		if (reply.isreply) {
			cout <<"长度:"<<reply.len <<"地址:"<< reply.addr.c_str() <<"序号:"<<reply.seq <<"存活时间:"<<reply.ttl <<"往返时间:"<< reply.rtt << endl;
		}
		else {
			cout << "request timeout\n";
		}
	}
}

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
		if(i==0){
			cout<<"PING "<<host_or_ip<<"("<<pingresult.ip<<"):"<<pingresult.datalen<<"bytes data in ICMP packets."<<endl;
		}
		if(!respond){
			cout<<pingresult.error_information<<endl;
			break;
		}
		show_pingresult(pingresult);
		has_sent += pingresult.has_sent;
		has_received += pingresult.has_received;
	}

	if(respond){
		cout<<has_sent<<"packets transmitted,"<<has_received<< " received , "<<100*(has_sent-has_received)/has_sent<< "% lost"<<endl;
	}

	return 0;
}


