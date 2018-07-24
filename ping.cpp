/*
 * ping.cpp
 *
 *  Created on: 2018年7月24日
 *      Author: garvin
 */

#include "ping.h"

Ping::Ping(){
	m_datalen = 56;
	m_hassent = 0;
	m_hasreceived = 0;
	m_seq = 0;
	m_maxpacketsize = 4;
}

bool Ping::ping(string& host_or_ip, int maxpacketsize, ping_result& pingresult){
	bool respond = false;
	icmp_echo_reply icmpechoreply;
	struct protoent* protocol;
	m_hassent = 0;
	m_hasreceived = 0;
	pingresult.IcmpEchoReplys.clear();
	m_maxpacketsize = maxpacketsize;
	m_pid = getpid();
	pingresult.datalen = m_datalen;

	if((protocol=getprotobyname("icmp")) == NULL){
		perror("在getprocolbyname中发生了一个错误");
		return false;
	}
	if((m_sockfd = socket(AF_INET, SOCK_RAW, protocol->p_proto)) < 0){
		extern int errno;
		pingresult.error_information = strerror(errno);
		return false;
	}

	if(!getsockaddr(host_or_ip, &m_dest_addr)){
		pingresult.error_information = "无法解析的host";
		return false;
	}

}

bool Ping::getsockaddr(const char* host_or_ip){
	if(inet_addr(host_or_ip) == INADDR_NONE){
		//说明是主机名
		struct hostent *host;
		host = gethostbyname(host_or_ip);
		if(host == NULL)
			return false;
		memcpy(&m_dest_addr->sin_addr, host->h_addr_list[0], host->h_length);
	}
	else{
		if(!inet_aton(host_or_ip, &m_dest_addr->sin_addr))
			return false;
	}
	return true;
}


