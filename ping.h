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
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
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
	bool getsockaddr(const string& host_or_ip);
	bool sendpacket();
	bool recvpacket(ping_result& pingresult);
	int packIcmp(int seq, struct icmp* icmppac);
	unsigned short getcksum(unsigned short* icmppac, int packsize);
public:
	Ping();
	~Ping(){}
};




#endif /* PING_H_ */
