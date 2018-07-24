/*
 * ping.h
 *
 *  Created on: 2018年7月24日
 *      Author: garvin
 */

#ifndef PING_H_
#define PING_H_

#include <iostream>
#include <string.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>

using namespace std;

struct ping_result{
	int has_sent;
	int has_received;
	int datalen;
	string ip;
	string error_information;
	vector<icmp_echo_reply> IcmpEchoReplys;
};

struct icmp_echo_reply{
	int seq;
	int len;
	int ttl;
	int rtt;
	bool isempty;
	string addr;
};

class Ping{
private:
	int m_datalen;
	int m_hassent;
	int m_hasreceived;
	int m_seq;
	int m_maxpacketsize;
	pid_t m_pid;
	int m_sockfd;
	sockaddr_in* m_dest_addr;
	sockaddr_in* m_from_addr;

	bool ping(string& host_or_ip, int maxpacketsize, ping_result& pingresult);
	bool getsockaddr(const char* host_or_ip);
public:
	Ping();
	~Ping(){}
};




#endif /* PING_H_ */
