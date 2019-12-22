#include<string.h>
#include<string>
#include<stdlib.h>
#include<netinet/in.h>
#include<unistd.h>
#include<errno.h>
#include<iostream>
#include<sys/socket.h>
#include<sys/types.h>
#include<netdb.h>
#include<fstream>

#define RECV_MAXLEN 1024
using namespace std;
int main(int argc, char** argv){
	//Read arguments
	if(3!=argc){
		fprintf(stderr,"usage: client [SITE] [PORT]");
		return 0;
	}
	const char* SITE=argv[1];
	const char* PORT=argv[2];
	struct addrinfo hint,*servinfo_res;
	memset(&hint, 0, sizeof hint);

	//Set socket type
	hint.ai_family=AF_UNSPEC;
	hint.ai_socktype=SOCK_STREAM;
	hint.ai_flags=AI_PASSIVE;

	//Get all IPs connected to DNS name
	int status;
	status=getaddrinfo(SITE,PORT,&hint,&servinfo_res);
	if(status!=0){
		fprintf(stderr,"getaddrinfo error: %s\n",gai_strerror(errno));
		exit(1);	
	}

	//Create socket
	int sock_descr;
	sock_descr=socket(servinfo_res->ai_family,servinfo_res->ai_socktype,servinfo_res->ai_protocol);
	if(-1==sock_descr){
		fprintf(stderr,"get socket desciptor error%s\n",strerror(sock_descr));
		exit(1);
	}else{
		cout<<"create socket successfully!"<<endl;
	}
	//Connect to socket on the server
	int connect_res;
	connect_res=connect(sock_descr,servinfo_res->ai_addr,servinfo_res->ai_addrlen);
	if(-1==connect_res){
		fprintf(stderr,"connect error%s\n",strerror(connect_res));
		exit(1);
	}else{
		cout<<"connect server successfully!"<<endl;
	}
	//Send msg
	int byte_sent;
	const char* msg="GET /test.txt HTTP/1.1\r\nHost: www.cs.ucsd.edu\r\nUser-Agent: My Agent\r\nCookie: 123\r\nMy-header: mykey0123\r\n\r\n";
	byte_sent=send(sock_descr,msg,strlen(msg),0);
	cout<<byte_sent<<"bytes were sent"<<endl;
	cout<<"sent msg is"<<endl<<msg<<endl;

	//Receive response
	int byte_receive;
	char* buf=new char[RECV_MAXLEN];
	ofstream outfile;
	outfile.open("recv_data.dat");
	while((byte_receive=recv(sock_descr,buf,RECV_MAXLEN,0))>0){
		if(-1==byte_receive){
			fprintf(stderr,"receive error%s\n",strerror(errno));
		}
		outfile<<buf;
	}

	cout<<"received msg is"<<endl<<buf<<endl;

	close(sock_descr);

	return 0;
}
