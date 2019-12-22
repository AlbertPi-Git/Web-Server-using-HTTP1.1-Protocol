#include "logger.hpp"
#include "HttpdServer.hpp"

#define MAXPENDING 5    /* Maximum outstanding connection requests */
#define RCVBUFSIZE 1024   /* Size of socket receive buffer */

using namespace std;

HttpdServer::HttpdServer(INIReader& t_config)
	: config(t_config)
{
	auto log = logger(); //Initialize SPD logger

	//Read port number to listen from config
	string pstr = config.Get("httpd", "port", "");
	if (pstr == "") {
		log->error("port was not in the config file");
		exit(EX_CONFIG);
	}
	port = pstr;
	//Read doc root directory from config
	string dr = config.Get("httpd", "doc_root", "");
	if (dr == "") {
		log->error("doc_root was not in the config file");
		exit(EX_CONFIG);
	}
	doc_root = dr;
	if(doc_root[0]=='~')
		doc_root=getenv("HOME")+doc_root.substr(1,string::npos);
	//Get mime.type
	string get_mime=config.Get("httpd","mime_types", "");
	if (get_mime == "") {
		log->error("mime_types was not in the config file");
		exit(EX_CONFIG);
	}
	mime_dir=get_mime;
	if(mime_dir[0]=='~')
		mime_dir=getenv("HOME")+mime_dir.substr(1,string::npos);
}

void *get_usr_addr(struct sockaddr *s_addr){
    if (s_addr->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)s_addr)->sin_addr);
    }
    return &(((struct sockaddr_in6*)s_addr)->sin6_addr);
}

//Server launching function
void HttpdServer::launch()
{
	auto log = logger();
	log->info("Launching web server");
	log->info("Port: {}", port);
	log->info("doc_root: {}", doc_root);

	struct addrinfo hints, *servinfo, *ptr;
	int addr_status=0;
	int sockfd,new_sockfd;
	int yes=1; //A parameter of setsockopt
	struct sockaddr_storage usrs_addr; //store connected users' addresses
	socklen_t usraddr_len;

	//Specify some options of socket, return all IP linked to this DNS name (although in current localhost situation there is only one) in 'servinfo'
	memset(&hints,0,sizeof hints);
	hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
	if ((addr_status= getaddrinfo(NULL, port.c_str(), &hints, &servinfo)) != 0) {
        log->error("get address info error"); 
		return;
    }
	//Create socket, set socket option and bind to port
	for(ptr = servinfo; ptr != NULL; ptr = ptr->ai_next) {
        if (-1==(sockfd = socket(ptr->ai_family, ptr->ai_socktype,ptr->ai_protocol))) { 
            log->error("unable to create socket");
            continue;
        }
        if (-1==setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int))) { 
            log->error("unable to set socket option");
            exit(1);
        }
        int bind_res= bind(sockfd, ptr->ai_addr, ptr->ai_addrlen);
       	if (bind_res==-1) {
            close(sockfd); 
			log->error("unable to bind socket to port");
            continue;
        }
        break;
    }
	freeaddrinfo(servinfo); // all done with this structure, free it

	if (NULL==ptr) {
        log->error("failed to create socket and bind");
        exit(1);
    }
	if (-1==listen(sockfd, LOGQUEUE_LEN)) {
        log->error("listen error");
        exit(1);
    }

	log->info("Waiting for connection ...");

	char str_usraddr[INET6_ADDRSTRLEN];
	// main accept() loop, wait for clients' requests
	while(1) { 
		usraddr_len=sizeof usrs_addr;
        new_sockfd = accept(sockfd, (struct sockaddr *)&usrs_addr, &usraddr_len);	//Create a new socket descriptor for every incoming request
		if (-1==new_sockfd) {
            log->error("unable to accept");
           continue;
        }
		//Show new connection's IP
        inet_ntop(usrs_addr.ss_family,
        get_usr_addr((struct sockaddr *)&usrs_addr),
        str_usraddr, sizeof str_usraddr);
        log->info("server: got connection from {}", str_usraddr);
		//Set timeout time
		struct timeval timeout;
		timeout.tv_sec=5;
		timeout.tv_usec=0;
		if(setsockopt(new_sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout))<0){
			log->error("Set time out error");
		}

		//Create a new thread for each request to response
		thread response(bind(&HttpdServer::responseThread,this,new_sockfd));
		response.detach();
    }
}

//Thread function that handles each client's requests
void HttpdServer::responseThread(int new_sockfd){
	auto log=logger();
	int byte_receive;
	char buf[RCVBUFSIZE];
	bool IsClientError=false; //Client timeout flag
	bool IsClientClose=false; //Client connection: close flag
	//Recv loop
	while((byte_receive=recv(new_sockfd,buf,RCVBUFSIZE,0))>0){
		//Keep recving reqs until: client close the connection(recv()==0) or recv error occours(recv()==-1) or client timeout(recv()==-1) 
		string data_recv;
		data_recv.append(buf);
		string req_pipeline=data_recv.substr(0,data_recv.rfind("\r\n\r\n")+4); //Read pipelined reqs in each iteration
		log->info("Pipeline msgs from client are:\n{}",req_pipeline);
		//Loop for handling each req in reqs pipeline separately
		while(req_pipeline.length()){
		int CRLF_pos=req_pipeline.find("\r\n\r\n");
			string req_msg=req_pipeline.substr(0,CRLF_pos+2); //handle_request() only deal with the first \r\n in the \r\n\r\n so it's +2 rather than 4
			int handle_res=handle_request(req_msg.c_str(),new_sockfd);
			req_pipeline=req_pipeline.substr(CRLF_pos+4,string::npos); //Elimilate this req in pipeline
			if(400==handle_res){ //Break from handle loop and ready to break from recv loop if 400 error detected
				IsClientError=true;
				break;
			}
			if(0==handle_res){ //Break from handle loop and ready to break from recv loop if client is about to close connection
				IsClientClose=true;
				break;
			}
		}
		memset(buf,0,RCVBUFSIZE);
		if(IsClientError)
			break;
		if(IsClientClose)
			break;
	}
	if(-1==byte_receive){
		if(errno==EWOULDBLOCK){
			log->error("Client timeout");
		}
		log->error("server receive error");
	}
	if(IsClientError)
		log->error("Malformed request in pipeline requests");
	if(IsClientClose)
		log->error("Client closed the connection");
	close(new_sockfd); //Close the connection 
}

//Handle each request, send headers and requested file. Return 400 if req is malformed, return 404 if file not found, return 200 if everything is fine, return 0 if client closes the connection.
int HttpdServer::handle_request(const char* req_msg, int client_sock){

	auto log=logger();
	//Copy the buffer to parse, since strsep() will modify the string
	char* msg_copy = (char*)malloc(strlen(req_msg) + 1); 
	strcpy(msg_copy, req_msg);

	//Bad request check
	string headers;
	int is_badreq=badreq_check(msg_copy);
	if(is_badreq){
		log->error("Bad request");
		headers=build_400_badreq_headers();
		send(client_sock, (void*) headers.c_str(), (ssize_t) headers.size(), 0);
		return 400;
	}

	// Get the filename
	char* first_line = strsep(&msg_copy, "\r\n");
	strsep(&first_line," ");
	char* url = strsep(&first_line," ");

	// Prepend document root to get the absolute path
	string full_rela_path = doc_root+url;
	if(full_rela_path[full_rela_path.length()-1]=='/')
		full_rela_path+="index.html";
	// log->info("request relative path is: {}",full_rela_path);
	char abs_path[256];
	realpath(full_rela_path.c_str(),abs_path);
	// log->info("request absolute path is: {}",abs_path);

	// Validate the file path requested.
	int is_valid = validate_file(abs_path);
	// log->info("file valid status {}",is_valid);

	if(is_valid==-1){
		headers = build_404_notfound_headers();
		send(client_sock, (void*) headers.c_str(), (ssize_t) headers.size(), 0);
		return 404;
	}

	headers = build_200_ok_headers(abs_path);
	send(client_sock, (void*) headers.c_str(), (ssize_t) headers.size(), 0);

	//Send body
	struct stat finfo;
	int fd = open(abs_path, O_RDONLY);
	fstat(fd, &finfo);
	off_t offset = 0;
	int file_size=get_file_size(abs_path);
	int h = sendfile(client_sock,fd,&offset,file_size);
	log->info("Sendfile status: {} bytes have been sent", h);

	int is_connectClose=connectClose_check(msg_copy);
	//return 0 if client closes the connection else return 200
	if(is_connectClose){
		return 0;
	}else
		return 200;
}

//Check whether a request is malformed, return 1 if req is malformed else return 0
int HttpdServer::badreq_check(char* req_msg){
	auto log=logger();
	char* msg_copy = (char*)malloc(strlen(req_msg) + 1); 
	strcpy(msg_copy, req_msg);  //Copy the msg since strsep() will modify msg
	
	char* first_line = strsep(&msg_copy, "\r\n"); //Get the init line, notice strsep() will left a \n in \r\n at the beginning of msg_copy
	string req_type=strsep(&first_line," ");
	if(req_type!="GET"){
		log->error("No GET in the request message");
		return 1;
	}
	string url=strsep(&first_line," ");
	if(url[0]!='/'){
		log->error("Missing forwarding slash");
		return 1;
	}	
	string http_proto = first_line;
	if(http_proto!="HTTP/1.1"){
		log->error("No HTTP/1.1 in the request message");
		return 1;
	}
	do{
		msg_copy+=1; //skip the \n belongs to previous line
		char* sep_res=strsep(&msg_copy,"\r\n");
		if(sep_res==NULL){
			log->error("seperation NULL break");
			break;
		}
		string key_val(sep_res);
		int pos_colon=key_val.find(":");
		if(key_val.find(":")==string::npos and key_val.length()!=0){
			log->error("No colon(:) in key value pair");
			return 1;
		}
		string tmp(1,key_val[pos_colon+1]);
		if(tmp!=" "){
			log->error("No space after :");
			return 1;
		}
	}while(string(msg_copy)!="\n");
	return 0;
}

//Check whether there is a valid file under the requested path, return 1 if it's valid else return -1
int HttpdServer::validate_file(string abs_path){
	auto log=logger();
	// log->info("Absolute path of requested file is: {}",abs_path);
    char doc_roor_abs[256];
    realpath(doc_root.c_str(),doc_roor_abs);
    string prefix(doc_roor_abs);
	if (access(abs_path.c_str(), F_OK) == -1){
		return -1;
	}else{
		// Check if the file path doesn't escape the document root.
        if(abs_path.find(prefix)==string::npos) //In normal situation, the root path should be a substr of request path
            return -1;
		return 1;
	}
}

//Check whether there is connection: close line in req, return 1 if there is, else return 0
int HttpdServer::connectClose_check(char* key_vals){
	do{
		key_vals+=1;
		string key_val=string(strsep(&key_vals,"\r\n"));
		if(key_val=="Connection: close")
			return 1;
	}while(string(key_vals)!="\n");
	return 0;
}

string HttpdServer::build_200_ok_headers(string filepath){
	struct stat buf;
    stat(filepath.c_str(), &buf);
	char time[128];
    strftime(time, sizeof time, "%a, %d %b %y %T %z",localtime(&buf.st_mtime));
	string response;
	response += "HTTP/1.1 200 OK\r\n";
	response += "Server: My Server 1.0\r\n";
	response += "Last-Modified: "+string(time)+"\r\n";
	response += "Content-Length: "+ to_string(get_file_size(filepath.c_str())) +"\r\n";
    string extstring=filepath.substr(filepath.find_last_of('.'),string::npos);
	response += "Content-Type: "+readfrommime(extstring)+"\r\n";
	response += "\r\n";

	return response;
}
string HttpdServer::build_400_badreq_headers(){
	string response;
	response +="HTTP/1.1 400 Client Error\r\n";
	response +="Server: Myserver 1.0\r\n";
	response +="\r\n";

	return response;
}
string HttpdServer::build_404_notfound_headers(){
	string response;
	response +="HTTP/1.1 404 Not Found\r\n";
	response +="Server: Myserver 1.0\r\n";
	response +="\r\n";

	return response;
}

string HttpdServer::readfrommime(string value){
    string type="";
    map<string, string>  dict;
    ifstream fin;
    fin.open(mime_dir);
    string str;
    vector<string> v;
	//Read contents of mime.types into a hashmap
    while (getline(fin, str)){
        char* str_cp=(char*)str.c_str();
        char* key=strsep(&str_cp," ");
        char* val=str_cp;
        dict.insert(pair<string,string>(string(key),string(val)));
    }
    map<string, string>::iterator iter;
	//Search the type
    if((iter=dict.find(value))!=dict.end())
        type=iter->second;
    else
        type="application/octet-stream";
    return type;
}

int HttpdServer::get_file_size(const char* filepath){
    struct stat finfo;
	auto log=logger();
    if (stat(filepath, &finfo) != 0) {
        log->error("Get filesize error");
    }
    return (int) finfo.st_size;
}
