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

	if(!getsockaddr(host_or_ip.c_str())){
		pingresult.error_information = "无法解析的host";
		return false;
	}

	pingresult.ip = inet_ntoa(m_dest_addr->sin_addr);
	sendpacket();
	recvpacket(pingresult);
	pingresult.has_sent = m_hassent;
	pingresult.has_received = m_hasreceived;
	close(m_sockfd);
	return true;
}

bool Ping::getsockaddr(const string& host_or_ip){
	if(inet_addr(host_or_ip.c_str()) == INADDR_NONE){
		//说明是主机名
		struct hostent *host;
		host = gethostbyname(host_or_ip.c_str());
		if(host == NULL)
			return false;
		memcpy(&m_dest_addr->sin_addr, host->h_addr_list[0], host->h_length);
	}
	else{
		if(!inet_aton(host_or_ip.c_str(), &m_dest_addr->sin_addr))
			return false;
	}
	return true;
}

bool Ping::sendpacket(){
	int packetsize;
	char* packethead;
	while(m_hassent < 3){
		m_hassent++;
		m_seq++;
		packIcmp(m_seq, (struct icmp*)packethead);
	}
	return true;
}

int Ping::packIcmp(int seq, struct icmp* icmppac){
	icmppac->icmp_type = ICMP_ECHO;
	icmppac->icmp_code = 0;
	icmppac->icmp_cksum = 0;
	icmppac->icmp_hun.ih_idseq.icd_seq = seq;
	icmppac->icmp_hun.ih_idseq.icd_id = m_pid;
	gettimeofday((struct timeval*)icmppac->icmp_dun.id_data, NULL);
	int packsize = m_datalen+8;
	icmppac->icmp_cksum = getcksum((unsigned short*) icmppac, packsize);
	return packsize;
}

unsigned short Ping::getcksum(unsigned short* icmppac, int packsize){
	int sum = 0;
	while((packsize = packsize-2)>=0){
		sum += *icmppac;
		icmppac++;
	}
	if(packsize == -1){
		unsigned short tmp = 0;
		*(char*)&tmp = *(char*)icmppac;
		sum += *tmp;
	}
	while(sum>>16){
		sum = (sum>>16) + (sum&0xffff);
	}
	sum = ~sum;
	return sum;
}

bool Ping::recvpacket(ping_result& pingresult){
	struct fd_set fdset;
	FD_ZERO(&fdset);
	struct timeval timeout;
	timeout.tv_sec = 0, timeout.tv_usec = 5;
	struct icmp_echo_reply icmpechoreply;
	int fdnum = 0, length = 0;
	char* recvpacket;
	socklen_t fromaddrlen = sizeof(m_from_addr);
	for(int i=0; i<3; i++){
		FD_SET(m_sockfd, &fdset);
		fdnum = select(m_sockfd+1, &fdset, NULL, NULL, &timeout);
		if(fdnum == -1){
			perror("select error");
			continue;
		}
		if(fdnum == 0){
			icmpechoreply.isreply = false;
			pingresult.IcmpEchoReplys.push_back(icmpechoreply);
			continue;
		}
		if(FD_ISSET(m_sockfd, &fdset)){
			length = recvfrom(m_sockfd, recvpacket, sizeof(char*)*4096, 0,(struct sockaddr*)&m_from_addr, &fromaddrlen );
			if(length < 0){
				if(errno == EINTR)
					continue;
				perror("recvfrom error");
				continue;
			}
			icmpechoreply.addr = inet_ntoa(m_from_addr);
			if(icmpechoreply.addr != pingresult.ip){
				i--;
				continue;
			}
		}
		if(unpackIcmp(recvpacket, sizeof(char*)*4096, &icmpechoreply) == -1){
			i--;
			continue;
		}
		icmpechoreply.isreply = true;
		pingresult.IcmpEchoReplys.push_back(icmpechoreply);
		m_hasreceived++;
	}
	return true;
}

bool Ping::unpackIcmp(const char* recvpacket, int packlen, icmp_echo_reply* icmpechoreply){

}




