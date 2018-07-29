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

	int size = 50*1024;
	setsockopt(m_sockfd,SOL_SOCKET,SO_RCVBUF,&size,sizeof(size) );

	pingresult.ip = inet_ntoa(m_dest_addr.sin_addr);
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
		memcpy(&m_dest_addr.sin_addr, host->h_addr, host->h_length);
	}
	else{
		if(!inet_aton(host_or_ip.c_str(), &m_dest_addr.sin_addr))
			return false;
	}
	return true;
}

bool Ping::sendpacket(){
	int packetsize;
	while(m_hassent < 3){
		m_hassent++;
		m_seq++;
		packetsize = packIcmp(m_seq, (struct icmp*)m_sendpacket);
		if( sendto(m_sockfd, m_sendpacket, packetsize, 0, (struct sockaddr*)&m_dest_addr, sizeof(m_dest_addr)) <0){
			perror("sendto error");
			continue;
		}
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
		sum += tmp;
	}
	while(sum>>16){
		sum = (sum>>16) + (sum&0xffff);
	}
	sum = ~sum;
	return sum;
}

bool Ping::recvpacket(ping_result& pingresult){
	fd_set fdset;
	FD_ZERO(&fdset);
	struct timeval timeout;
	timeout.tv_sec = 5, timeout.tv_usec = 0;
	struct icmp_echo_reply icmpechoreply;
	int fdnum = 0, length = 0;
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
			length = recvfrom(m_sockfd, this->m_recvpacket, sizeof(char*)*4096, 0,(struct sockaddr*)&m_from_addr, &fromaddrlen );
			if(length < 0){
				if(errno == EINTR)
					continue;
				perror("recvfrom error");
				continue;
			}
			icmpechoreply.addr = inet_ntoa(m_from_addr.sin_addr);
			if(icmpechoreply.addr != pingresult.ip){
				i--;
				continue;
			}
		}
		if(unpackIcmp(m_recvpacket, sizeof(char*)*4096, &icmpechoreply) == -1){
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
	struct ip* Ip;
	struct icmp* Icmp;
	struct timeval *tvsend, tvrecv, tvresult;
	Ip = (struct ip*)recvpacket;
	Icmp = (struct icmp*)(recvpacket + Ip->ip_hl*4);
	packlen -= Ip->ip_hl*4;
	if(packlen < 8){
		cout<<"ICMP 报文长度出错"<<endl;
		return false;
	}
	if (Icmp->icmp_type == ICMP_ECHOREPLY && Icmp->icmp_hun.ih_idseq.icd_id == m_pid) {
		tvsend = (struct timeval *) Icmp->icmp_dun.id_data;
		gettimeofday(&tvrecv, NULL);
		tvresult = tvsub(tvrecv, *tvsend);
		icmpechoreply->rtt = tvresult.tv_sec * 1000 + tvresult.tv_usec / 1000;;
		icmpechoreply->seq = Icmp->icmp_seq;
		icmpechoreply->ttl = Ip->ip_ttl;
		icmpechoreply->len = packlen;
		return true;
	}
	else {
		return false;
	}
}

struct timeval Ping::tvsub(timeval tv1, timeval tv2){
	struct timeval result = tv1;
	if(tv1.tv_usec<tv2.tv_usec){
		result.tv_sec--;
		result.tv_usec += 1000000;
	}
	result.tv_sec -= tv2.tv_sec;
	result.tv_usec -= tv2.tv_usec;
	return result;
}



