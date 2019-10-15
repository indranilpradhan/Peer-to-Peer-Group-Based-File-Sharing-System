#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <string>
#include <string.h> 
#include <cstring>
#include <cstdlib>
#include <sys/wait.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <fstream> 
#include <algorithm>
#include <openssl/sha.h>
#include <pthread.h>
#include <time.h>

using namespace std;
#define chunk_size 524288
#define ll long long int

class Package{
public:
	int groupid;
	ll filesize;
	char filename[10000];
	vector<int> chunks;
};

struct chunkdetail{
public:
	string filename;
	int port;
	string ip;
	int groupid;
	int noofchunks;
	vector<int> chunks;
	vector<string> hashes;
	int chunknumber;
	ll filesize;
};

vector<Package> v;
vector<chunkdetail> vchunkdetail;
//vector<chunkshashes> vchhs;
vector<int> tempchunks;
vector<string> temphashes;

int counter = 0;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
char* ip;
int portadd;
int cport;
char* ipc;
FILE *fpc;
char sc[100000];
ll filesz;
string globaluserid;
int isnotintegrated = 0;

string ToString(char* a, int size) 
{ 
    int i; 
    string s = ""; 
    for (i = 0; i < size; i++) { 
        s = s + a[i]; 
    } 
    return s; 
}

void stringtochararray(string st, char* arr)
{ 
    strcpy(arr, st.c_str());
}

/*void sharablefile(int sockfd, chunkdetail tdetail)
{
	char s2[1000];
	char tip[100];
	strcpy(s2,tdetail.filename.c_str());
    cout<<"sharablefile filename at client "<<s2<<endl;
    send(sockfd,s2,sizeof(s2),0);
    //		cout<<"644"<<endl;
    int chunkcount = tdetail.noofchunks;
    cout<<"sharablefile chunkcount at client "<<chunkcount<<endl;
    send(sockfd,&chunkcount, sizeof(chunkcount),0);
    //		cout<<"646"<<endl;
    char tes[]="hello";
    send(sockfd, tes, sizeof(tes), 0);

    int tport = tdetail.port;
    cout<<"sharablefile port at client "<<tport<<endl;
    send(sockfd, &tport, sizeof(tport), 0);
    //		cout<<"649"<<endl;
    strcpy(tip,tdetail.ip.c_str());
    cout<<"sharablefile ip at client "<<tip<<endl;
    send(sockfd, tip, sizeof(tip), 0);
    //		cout<<"652"<<endl;
    string temp="";
    for(int e = 0;e<chunkcount;e++)
    {
    	int q= tempchunks.at(e);
    	cout<<"sharablefile chunks at client "<<q<<endl;
    	send(sockfd,&q,sizeof(q),0);

    	char thash[20];
    	temp = temp+temphashes.at(e);
    	strcpy(thash,temphashes.at(e).c_str());
    	cout<<"sharablefile hash at client "<<thash<<endl;
    	send(sockfd,thash,sizeof(thash), 0);
    }

    int gid = tdetail.groupid;
    cout<<"sharablefile group id at client "<<gid<<endl;
    send(sockfd, &gid, sizeof(gid), 0);

    ll size = tdetail.filesize;
    cout<<"sharablefile filesize at client "<<size<<endl;
    send(sockfd, &size, sizeof(size),0);

    string finalst = globaluserid+" "+to_string(gid)+" "+tdetail.ip+" "+to_string(tdetail.port)+" "+tdetail.filename+" "+to_string(size)+" "+temp;
    char sendingchar[100000];
    cout<<"sharablefile final string at client "<<finalst<<endl;
    strcpy(sendingchar,finalst.c_str());
    send(sockfd, &sendingchar, sizeof(sendingchar),0);
} */

void *sendfilebychunks(void *clientfd_desc)
{
	int clientfd = *((int *) clientfd_desc);
	char filename[1000];
//	cout<<"before receiving filename"<<endl;
	recv(clientfd, &filename, sizeof(filename), 0);
//	cout<<"filename "<<filename<<endl;
	//	pthread_mutex_lock(&mtx);
	FILE *fp = fopen(filename,"r");
//	cout<<"filename matched"<<endl;

	int t =0;
	send(clientfd, &t, sizeof(t),0);
//	cout<<" after test"<<endl;

	int chunknumber= 0;
	recv(clientfd,&chunknumber,sizeof(chunknumber),0);
	cout<<"chunk number from client sernder "<<chunknumber<<endl;

	ll offset = chunk_size*(chunknumber);
	cout<<"offset from client sender "<<offset<<endl;
	int off = fseek(fp,offset,SEEK_SET);
	char Buffer[chunk_size];
	int len = fread(Buffer, sizeof(char) ,sizeof(Buffer), fp);

	fseek(fp,offset,SEEK_SET);
	char newBuffer[2048];
	cout<<"size from client sender "<<len<<endl;
	send(clientfd,&len,sizeof(len),0);
	int n = 0;
	while ((n = fread(newBuffer, sizeof(char), sizeof(newBuffer) , fp)) > 0  && len > 0 )
	{
		int t1,t2;
		send(clientfd, &n, sizeof(n), 0);

		recv(clientfd,&t1,sizeof(t1),0);

		newBuffer[n] = '\0';
		send(clientfd, newBuffer, n, 0 );
   		memset(newBuffer , '\0', sizeof(newBuffer));
		len = len - n;
		cout<<"remaining size from client sender "<<len<<endl;

		recv(clientfd,&t2,sizeof(t2),0);
	}
					//	cout<<"size of new buffer "<<sizeof(newBuffer)<<endl			
    memset( Buffer , '\0', sizeof(Buffer));
    rewind(fp);		
	fclose(fp);
	//	pthread_mutex_lock(&mtx);
	memset(filename ,'\0',sizeof(filename));
	close(clientfd);
}

void *sendfile(void *socket_desc)
{
	int sockfd = socket( AF_INET, SOCK_STREAM, 0 );;
	struct sockaddr_in   addr;
	struct sockaddr_in address;
	bzero(&addr, sizeof(addr));
	bzero(&address, sizeof(address));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(portadd);
	addr.sin_addr.s_addr=inet_addr(ip);
//	cout<<"listening ip "<<ip<<endl;
//	cout<<"listening port "<<portadd<<endl;
	//cout<<"addrlen "<<addrlen<<endl;
	bind (sockfd, (struct sockaddr *)&addr, sizeof(addr));
//	cout<<"listening sockfd "<<sockfd<<endl; 
	int l=listen (sockfd, 3); 
//	cout<<"l "<<l<<endl;
	int clientfd;

	while(1)
	{
//		cout<<"here at listening"<<endl;
		int addrlen = sizeof(sockaddr_in);
		clientfd = accept(sockfd , (struct sockaddr *)&address , (socklen_t*)&addrlen);
//		cout<<"clientfd "<<clientfd<<endl;
		pthread_t ctid;
		if(pthread_create( &ctid , NULL, sendfilebychunks, (void*) &clientfd) < 0)
    	{
    	    perror("could not create thread");
      	    return 0;
    	}
   		pthread_detach(ctid);
	}
//	close(clientfd);
	close(sockfd);
}

void *recievefile(void *chunkstruct)
{
    chunkdetail cdetail = *( (chunkdetail*) (chunkstruct));
//	cout<<"here at recievefile"<<endl;
	int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	struct sockaddr_in  addr, peer_addr;
	bzero(&addr, sizeof(addr));
	bzero(&peer_addr, sizeof(peer_addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(0);
	//cout<<"ip at receiver "<<ip<<endl;
	addr.sin_addr.s_addr=inet_addr(ip);
	if(bind(sockfd, (struct sockaddr*) &addr, sizeof(struct sockaddr_in)) != 0)
	{
    	printf("Unable to bind"); 
    		return 0;
	}

	peer_addr.sin_family = AF_INET;
	peer_addr.sin_port = htons(cdetail.port);
//	cout<<"141 server port "<<cdetail.port<<" server ip "<<cdetail.ip<<endl;
	peer_addr.sin_addr.s_addr=inet_addr(cdetail.ip.c_str());
	if(connect(sockfd, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) != 0)
	{
		cout<<"unbale to connect";
		return 0;
	}

	char cfile[100];
	strcpy(cfile,cdetail.filename.c_str());
	cout<<"file name sent from client recver "<<cfile<<endl;
	send(sockfd,&cfile,sizeof(cfile),0);

	int t;
	recv(sockfd, &t,sizeof(t),0);
//	cout<<" after test"<<endl;

	int chunknumber = cdetail.chunknumber;
	cout<<"chunknumber sent from client recver "<<chunknumber<<endl;
	send(sockfd, &chunknumber,sizeof(chunknumber),0);

	int size = 0;
	recv(sockfd,&size,sizeof(size),0);
//	cout<<"size in client recver "<<size<<endl;

	ll offset = chunk_size*chunknumber;
//	cout<<"offset in client recver "<<offset<<endl;
	int off = fseek(fpc,offset,SEEK_SET);
//	fileintegrity fi;
//	memset(fi.hashbuf,'\0',sizeof(fi.hashbuf));
	unsigned char hashbuf[chunk_size];
	size_t j =0;
	while(size > 0)
	{
		int t1=0,t2=0;

		int buflen;
		recv(sockfd,&buflen,sizeof(buflen),0);
		cout<<"buflen in client recver "<<buflen<<endl;

		send(sockfd,&t1,sizeof(t1),0);

		unsigned char Buffer[buflen];
		recv(sockfd,Buffer,sizeof(Buffer) , 0);

		Buffer[buflen] = '\0';
		fwrite (Buffer, sizeof(char), sizeof(Buffer), fpc);

		//strcat((char*)fi.hashbuf,(const char*)Buffer);
	//	cout<<"here concat of signed and unsigned "<<endl;
		memcpy(hashbuf+j,Buffer, sizeof(Buffer));
		j = j + sizeof(Buffer);
	//	cout<<"concat done "<<j<<endl; 

    	memset( Buffer , '\0', sizeof(Buffer));
    	size = size - buflen;
	   	cout<<"remaining size "<<size<<endl;

    	send(sockfd, &t2, sizeof(t2), 0);
	}	 

//	cout<<"hash at chunkdetail "<<chunknumber<<" is "<<cdetail.hashes.at(chunknumber)<<endl;
//	fi.hashst = cdetail.hashes.at(chunknumber);

	unsigned char hash[chunk_size];
  	string hashed="";
  	SHA1(hashbuf, sizeof(hashbuf) - 1, hash);
    for (int i = 0; i < 10; i++) {
    	char buf[100];
        sprintf((char*)&buf,"%02x", hash[i]);
        	//		cout<<"buf "<<buf<<endl;
        string t(buf);
        hashed = hashed + t;
        	//		cout<<"hashed first "<<hashed<<endl;
    }

    if(hashed == cdetail.hashes.at(chunknumber))
    {
    	isnotintegrated = 0;
  // 		cout<<"hash matched"<<endl;
   		tempchunks.at(chunknumber) = 1;
   		temphashes.at(chunknumber) = hashed;
    }
    else
    {
    	isnotintegrated = 1;
 //   	cout<<"hash not matched"<<endl;
    }
//	fclose(fpc);
	memset(sc, '\0', sizeof(sc));
	close(sockfd);
}

int main(int argc, char** argv)
{
	int sockfd = socket( AF_INET, SOCK_STREAM, 0 );

	struct sockaddr_in  serv_addr, client_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	bzero(&client_addr, sizeof(client_addr));
//	cout<<"arg 1 "<<argv[1]<<" arg 2 "<<argv[2]<<endl;
	ip = strtok(argv[1],":");
	char* port = strtok(NULL,":");
	portadd = atoi(port); 
	//string ipaddr(ip);
	client_addr.sin_family = AF_INET;
//	cout<<"client port "<<port<<endl;
	client_addr.sin_port = htons(0);
//	cout<<"client ip "<<ip<<endl;
	client_addr.sin_addr.s_addr=inet_addr(ip);
	if(bind(sockfd, (struct sockaddr*) &client_addr, sizeof(struct sockaddr_in)) != 0)
	{
    	printf("Unable to bind"); 
    		return 0;
	}

	ifstream file(argv[2]);
	string line;
	if(file.good())
	{
    	getline(file, line);
	}
	int len = line.length();
	char arr[len+1];
	stringtochararray(line,arr);
	char* ips = strtok(arr," ");
	char* ports = strtok(NULL," ");
	int portadds = atoi(ports); 
	//string ipaddrs(ips);
//	cout<<"server port "<<portadds<<endl;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portadds);
//	cout<<"server ip "<<ips<<endl;
	serv_addr.sin_addr.s_addr=inet_addr(ips);

	if(connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
	{
		for (int i=0; i<2;i++)
    	{
        	getline(file,line);
   		}
    	int len = line.length();
		char arra[len+1];
		stringtochararray(line,arra);
    	char* ips2 = strtok(arra," ");
		char* ports2 = strtok(NULL," ");
		int portadds2 = atoi(ports2); 
		//string ipaddrs2(ips2);
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(portadds2);
		serv_addr.sin_addr.s_addr=inet_addr(ips2);
		if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
		{
			cout<<"unbale to connect";
			return 0;
		}
	}
	//pthread_t thread1,thread2;
	//pthread_mutex_init(&mtx, NULL);
	pthread_t tid[100];
	int i=0;
	
	//pthread_t ctid[60];
	//int j=0;
	int isloggedin = 0;
	int ifprelim = 0;
	while(1)
	{
		if(pthread_create( &tid[i] , NULL ,  sendfile , (void*) &sockfd) < 0)
    	{
    		perror("could not create thread");
        	return 1;
  		}
  		pthread_detach(tid[i]);
    	i++;
   		char * input =new char[1000];
    	int count = 0;
    	int i = 0;
		int j = 0;
    	char *a[1000], *b, *newa[1000];
    	char sizeoffile[100000];

    	while(1)
    	{
    		int p = fgetc(stdin);
    		input[count++] = (char) p;
    		if( p == '\n')
        		break;
    	}

    	input[count] = '\0';
    	if(count == 1)
    	return 0;

    	b = strtok(input," \n");
    	i=0;
    	while(b != NULL)
    	{
    		a[i++] = strdup(b);
       		b = strtok(NULL," \n");
    	}
    	a[i]=NULL;
 	//	cout<<"here "<<a[0]<<endl;

 		if(strcmp(a[0],"create_user") == 0)
 		{
 			ifprelim = 1;
 			int iscorrect = 1;
 			char command[1000];
			strcpy(command,"create_user");
 			send(sockfd , &command, sizeof(command), 0);
 			string st1(a[1]);
 			string st2(a[2]);
 			string s = st1+" "+st2;
 			char buffer[1000];
 			strcpy(buffer,s.c_str());
 			send(sockfd,buffer,sizeof(buffer), 0);
 			recv(sockfd,&iscorrect, sizeof(iscorrect),0);
 			if(iscorrect == 0)
 			{
 				cout<<"Userid exists"<<endl;
 				continue;
 			}
 			else
 				cout<<"User created successfully"<<endl;
 			memset(command,'\0',sizeof(command));
 			fflush(stdin);
			fflush(stdout);
 		}

 		if(strcmp(a[0],"login") == 0)
 		{
 			ifprelim = 1;
 			int iscorrect = 1;
 			char command[1000];
			strcpy(command,"login");
 			send(sockfd , &command, sizeof(command), 0);
 			string st1(a[1]);
 			string st2(a[2]);
 			string s = st1+" "+st2;

 			char buffer[1000];
 			strcpy(buffer,s.c_str());
 			send(sockfd,buffer,sizeof(buffer), 0);
 			recv(sockfd,&iscorrect, sizeof(iscorrect),0);
 			if(iscorrect == 0)
 			{
 				cout<<"Username or password mismatch"<<endl;
 				continue;
 			}
 			else
 			{
 				globaluserid = st1;
 				isloggedin = 1;
 				cout<<"Login successful"<<endl;
 			}
 			memset(command,'\0',sizeof(command));
 			fflush(stdin);
			fflush(stdout);
 		}

 		if(isloggedin == 0)
 		{
 			cout<<"User is not logged in"<<endl;
 			continue;
 		}

 		if(ifprelim == 1)
 		{
 			ifprelim =0;
 			continue;
 		}

 		if(strcmp(a[0],"list_groups") == 0)
 		{
 			char command[1000];
			strcpy(command,"list_groups");
 			send(sockfd , &command, sizeof(command), 0);
 			int n = 0;
 			recv(sockfd, &n, sizeof(n),0);		
 			while(n--)
 			{
 				int x=0;
 				recv(sockfd, &x, sizeof(x),0);
 				cout<<x<<endl;
 				//cout<<"n "<<n<<endl;
 			}
 			memset(command,'\0',sizeof(command));
 			fflush(stdin);
			fflush(stdout);
 		//	cout<<"end list_groups in client"<<endl;
 		}

 		else if(strcmp(a[0],"join_group") == 0)
 		{
 			char command[1000];
			strcpy(command,"join_group");
 			send(sockfd , &command, sizeof(command), 0);
 			string st(a[1]);
 			char buffer[10];
 			strcpy(buffer,st.c_str());
 			send(sockfd, buffer, sizeof(buffer), 0);
 			memset(command,'\0',sizeof(command));
 			fflush(stdin);
			fflush(stdout);
 		}

 		else if(strcmp(a[0],"leave_group") == 0)
 		{
 			char command[1000];
			strcpy(command,"leave_group");
 			send(sockfd , &command, sizeof(command), 0);
 			string st(a[1]);
 			char buffer[10];
 			strcpy(buffer,st.c_str());
 			send(sockfd, buffer, sizeof(buffer), 0);
 			memset(command,'\0',sizeof(command));
 			fflush(stdin);
			fflush(stdout);
 		}

 		else if(strcmp(a[0],"list_requests") == 0)
 		{
 			int iscorrect = 1;
 			char command[1000];
			strcpy(command,"list_requests");
 			send(sockfd , &command, sizeof(command), 0);
 			string st(a[1]);
 			char buffer[10];
 			strcpy(buffer,st.c_str());
 			send(sockfd, buffer, sizeof(buffer), 0);
 			recv(sockfd, &iscorrect, sizeof(iscorrect), 0);
 			if(iscorrect == 1)
 			{
 				int n=0;
 				recv(sockfd, &n, sizeof(n),0);
 				while(n--)
 				{
 					char buffer[512];
 					recv(sockfd,buffer, sizeof(buffer),0);
 					cout<<buffer<<endl;
 					//memset(buffer , '\0', sizeof(buffer));
 				}
 			}
 			else
 			{
 				cout<<"Invalid request"<<endl;
 				continue;
 			}
 			memset(command,'\0',sizeof(command));
 			fflush(stdin);
			fflush(stdout);
 		//	cout<<"end list_requests in client"<<endl;
 		}

 		else if(strcmp(a[0],"list_files") == 0)
 		{
 			int ispre = 0;
 			char command[1000];
			strcpy(command,"list_files");
 			send(sockfd , &command, sizeof(command), 0);
 			string st(a[1]);
 			char buffer[10];
 			strcpy(buffer,st.c_str());
 			send(sockfd, buffer, sizeof(buffer),0);
 			recv(sockfd, &ispre, sizeof(ispre) ,0);
 			if(ispre == 1)
 			{
 				int n = 0;
 				recv(sockfd, &n, sizeof(n) ,0);				
 				while(n--)
 				{
 					char buffer[1000];
 					recv(sockfd,buffer, sizeof(buffer), 0);
 					cout<<buffer<<endl;
 					//memset(buffer , '\0', sizeof(buffer));
 				}
 			}
 			else
 			{
 				cout<<"Group has no files"<<endl;
 				continue;
 			}
 			memset(command,'\0',sizeof(command));
 			fflush(stdin);
			fflush(stdout);
 		}

 		else if(strcmp(a[0],"create_group") == 0)
 		{
 			int ispre = 0;
 			char command[1000];
			strcpy(command,"create_group");
 			send(sockfd , &command, sizeof(command), 0);
 			string st(a[1]);
 			char buffer[10];
 			strcpy(buffer,st.c_str());
 			send(sockfd, buffer, sizeof(buffer),0);
 			recv(sockfd, &ispre, sizeof(ispre), 0);
 			if(ispre == 1)
 			{
 				cout<<"Group already present"<<endl;
 				continue;
 			}
 			memset(command,'\0',sizeof(command));
 			fflush(stdin);
			fflush(stdout);
 		}

 		else if(strcmp(a[0],"accept_request") == 0)
 		{
 			char command[1000];
			strcpy(command,"accept_request");
 			send(sockfd , &command, sizeof(command), 0);

 			string st1(a[1]);
 			string st2(a[2]);
 			string s = st1+" "+st2;

 			char buffer[1000];
 			strcpy(buffer,s.c_str());
 			send(sockfd,buffer,sizeof(buffer), 0);
 			memset(command,'\0',sizeof(command));
 			fflush(stdin);
			fflush(stdout);
 		}

 		else if(strcmp(a[0],"stop_share") == 0)
 		{
 			char command[1000];
			strcpy(command,"stop_share");
 			send(sockfd , &command, sizeof(command), 0);

 			string st1(a[1]);
 			string st2(a[2]);
 			string s = st1+" "+st2;


 			char buffer[1000];
 			strcpy(buffer,s.c_str());
 			send(sockfd,buffer,sizeof(buffer), 0);
 			memset(command,'\0',sizeof(command));
 			fflush(stdin);
			fflush(stdout);
 		}

 		else if(strcmp(a[0],"logout") == 0)
 		{
 			char command[1000];
			strcpy(command,"logout");
 			send(sockfd , &command, sizeof(command), 0);
 			
 			isloggedin = 0;	
 			memset(command,'\0',sizeof(command));
 			fflush(stdin);
			fflush(stdout);
 		}

		else if(strcmp(a[0],"upload_file") == 0)
		{
			char command[1000];
			strcpy(command,"upload_file");
 			send(sockfd , &command, sizeof(command), 0);

 			vector<int> chunks;
 			vector<string> hashes;
			Package p;
  	//		cout<<"filename "<<a[1]<<endl;
  			FILE *fp = fopen ( a[1]  , "r" );

			fseek ( fp , 0 , SEEK_END);
	  		ll size = ftell ( fp );
  			rewind ( fp );

  			string str = to_string(size);
  			strcpy(sizeoffile, str.c_str());
  			int lens = str.length();
  			sizeoffile[lens] = '\0';

  			strcpy(p.filename,a[1]);
  			p.filesize = size;
  			p.groupid = 1;
  			unsigned char Buffer[chunk_size];
  			char s[100000];
  			int n =0; 
  			int eof =0;
  			ll chunkcount =0;
  			if(size%chunk_size == 0)
  			{
  				chunkcount = size/chunk_size;
  			}
  			else
  			{
  				chunkcount = size/chunk_size;
  				chunkcount = chunkcount + 1;
  			}
  	//		cout<<"chunkcount "<<chunkcount<<endl;
  			chunks.assign(chunkcount,0);
  			hashes.assign(chunkcount,"");
  			int m = 0;
  			while(( n = fread(Buffer, sizeof(char) ,sizeof(Buffer) , fp)) > 0  && size > 0){
  				unsigned char hash[chunk_size];
  				string hashed="";
  				SHA1(Buffer, sizeof(Buffer) - 1, hash);
    			for (i = 0; i < 10; i++) {
    				char buf[100];
        			sprintf((char*)&buf,"%02x", hash[i]);
        	//		cout<<"buf "<<buf<<endl;
        			strcat(s,buf);
        			string t(buf);
        			hashed = hashed + t;
        	//		cout<<"hashed first "<<hashed<<endl;
    			}
    		//	chunkcount++;
    		//	cout<<"after hash"<<endl;
    			chunks.at(m) = 1;
    		//	cout<<"after chunk assign"<<endl;
    			hashes.at(m) = hashed;
			//    cout<<"hash "<<hashed<<endl;
    			memset( Buffer , '\0', sizeof(Buffer));
				size = size - n ;
			//	cout<<"remaining size "<<size<<endl;
				m++;
    		}
    	//	p.chunks.push_back(chunkcount);
    //		cout<<"637"<<endl;
    		v.push_back(p);

    		char s2[100];
    		char tip[100];
    		char uid[100];
    //		cout<<"641"<<endl;
    		strcpy(uid,globaluserid.c_str());
    		cout<<"globaluserid at client "<<uid<<endl;
    		send(sockfd,uid,sizeof(uid),0);

    		strcpy(s2,a[1]);
    		cout<<"filename at client "<<s2<<endl;
    		send(sockfd,s2,sizeof(s2),0);
    //		cout<<"644"<<endl;
    		cout<<"chunkcount at client "<<chunkcount<<endl;
    		send(sockfd,&chunkcount, sizeof(chunkcount),0);
    //		cout<<"646"<<endl;
    		char tes[]="hello";
    		send(sockfd, tes, sizeof(tes), 0);

    		int tport = atoi(port);
    		cout<<"port at client "<<tport<<endl;
    		send(sockfd, &tport, sizeof(tport), 0);
    //		cout<<"649"<<endl;
    		strcpy(tip,ip);
    		cout<<"ip at client "<<tip<<endl;
    		send(sockfd, tip, sizeof(tip), 0);
    //		cout<<"652"<<endl;
    		for(int e = 0;e<chunkcount;e++)
    		{
    			int q= chunks.at(e);
    			char thash[20];
    			cout<<"chunks at client "<<q<<endl;
    			send(sockfd,&q,sizeof(q),0);
    			strcpy(thash,hashes.at(e).c_str());
    	//		cout<<"hash at client "<<thash<<endl;
    			send(sockfd,thash,sizeof(thash), 0);
    		}

    		int gid = atoi(a[2]);
    		cout<<"group id at client "<<gid<<endl;
    		send(sockfd, &gid, sizeof(gid), 0);

    		ll tempsize = p.filesize;
    		cout<<"file size at client "<<tempsize<<endl;
    		send(sockfd, &tempsize, sizeof(tempsize),0);

    		fflush(stdin);
			fflush(stdout);
			char s1[100000];
			memset(s1,'\0',sizeof(s1));
		//	cout<<"hash in s "<<s<<endl;
			strcat(s1,globaluserid.c_str());
			strcat(s1," ");
			strcat(s1,a[2]);
			strcat(s1," ");
			strcat(s1,ip);
			strcat(s1," ");
			strcat(s1,port);
			strcat(s1," ");
			strcat(s1, a[1]);
			strcat(s1," ");
			strcat(s1,sizeoffile);
			strcat(s1," ");
			strcat(s1,s);
			int len = strlen(s1);
			s1[len] = '\0';
			cout<<"message sent "<<s1<<endl;
			send(sockfd,s1, sizeof(s1), 0);
			fclose ( fp );
			memset(s,'\0',sizeof(s));
			memset(command,'\0',sizeof(command));
			memset(s2,'\0',sizeof(s2));
			memset(tip,'\0',sizeof(tip));
			fflush(stdin);
			fflush(stdout);
		}

		else if(strcmp(a[0],"download_file")==0)
		{
		//	cout<<"download"<<endl;
			char command[1000];
			int issamegroup = 0;
			int issharable = 0;
			int countpeer = 0;
			int noofchunks =0;
			ll filesize;
			char filepath[1000];
			vector<chunkdetail> lchunkdetail;
			chunkdetail tdetail;

 			strcpy(command,"download_file");
 			send(sockfd , &command, sizeof(command), 0);
  		//	char s[100000];
 			int gid = atoi(a[1]);
 			send(sockfd , &gid, sizeof(gid), 0);

 			recv(sockfd, &issamegroup, sizeof(issamegroup), 0);
			if(issamegroup == 0)
			{
				cout<<"Different group, please join"<<endl;
				continue;
			}

  			strcpy(sc,a[2]);
  			send(sockfd, &sc, sizeof(sc),0);

			recv(sockfd, &issharable, sizeof(issharable), 0);
			if(issharable == 0)
			{
				cout<<"File is not sharable"<<endl;
				continue;
			}

			strcpy(filepath,a[3]);
   			strcat(filepath,"/");
   			strcat(filepath,a[2]);
   			fpc = fopen(filepath,"w");

			recv(sockfd, &countpeer, sizeof(countpeer), 0);

			cout<<"countpeer at client "<<countpeer<<endl;
			for(int i=0; i<countpeer; i++)
			{
				chunkdetail cdetail;

				char vfile[100];
				recv(sockfd, vfile, sizeof(vfile), 0);
				cout<<"filename recved from tracker "<<vfile<<endl;
				string sfile(vfile);
				cdetail.filename = sfile;
	//			cout<<"filename inserted to clietn struct "<<cdetail.filename<<endl;

				int vport;
				recv(sockfd, &vport, sizeof(vport), 0);
				cout<<"port recved from tracker "<<vport<<endl;
				cdetail.port = vport;
//				cout<<"port inserted to clietn struct "<<cdetail.port<<endl;

				char vip[100];
				recv(sockfd,vip, sizeof(vip), 0);
				cout<<"ip recved from tracker "<<vip<<endl;
				string sip(vip);
				cdetail.ip = vip;
//				cout<<"ip inserted to clietn struct "<<cdetail.ip<<endl;
				

				int vnoofchunks;
				recv(sockfd, &vnoofchunks, sizeof(vnoofchunks), 0);
				cout<<"no of chunks recved from tracker "<<vnoofchunks<<endl;
				cdetail.noofchunks = vnoofchunks;
//				cout<<"no of chunks inserted to clietn struct "<<cdetail.noofchunks<<endl;
				noofchunks = vnoofchunks;

				for(int cin = 0; cin < vnoofchunks; cin++)
				{
					int tin;
					recv(sockfd, &tin, sizeof(tin), 0);
					cout<<"chunk recved from tracker "<<tin<<endl;
					cdetail.chunks.push_back(tin);
				}

				for(int h = 0; h < vnoofchunks; h++)
				{
					char hs[100];
					recv(sockfd, hs, sizeof(hs), 0);
					string shs(hs);
					cout<<"hash recved from tracker "<<shs<<endl;
					cdetail.hashes.push_back(shs);
				}

				recv(sockfd, &filesize, sizeof(filesize), 0);
				cout<<"filesize recved from tracker "<<filesize<<endl;
				cdetail.filesize = filesize;
//				cout<<"no of chunks inserted to clietn struct "<<cdetail.filesize<<endl;

				lchunkdetail.push_back(cdetail);
			}

			tempchunks.assign(noofchunks,0);
			temphashes.assign(noofchunks,"");

			int temp[countpeer][noofchunks];
			vector<chunkdetail>::iterator it;
			int m=0,n=0;
			for(it = lchunkdetail.begin(); it != lchunkdetail.end(); it++)
			{
				vector<int>::iterator i;
				for(i = (*it).chunks.begin(); i != (*it).chunks.end(); i++)
				{
					temp[m][n] = (*i);
					n++;
				}
				m++;
				n=0;
			}

			int k =0;
			int next = 0;
			int j=0;
			int count =0 ;
			for(int i=0; i<noofchunks;i++)
			{
				j = next%countpeer;
	//	cout<<i<<" chunk"<<endl;
				while(count != countpeer)
				{
					if(temp[j%countpeer][k] == 1)
					{
						//cout<<"j "<<(j%3)<<" k "<<k<<" "<<temp[j%3][k]<<endl;
						pthread_t ctid;
						chunkdetail peerdetail = lchunkdetail.at(j%countpeer);
						peerdetail.chunknumber = k;
						if(pthread_create( &ctid , NULL, recievefile, &peerdetail) < 0)
    					{
    	    				perror("could not create thread");
      	    				return 1;
    					}
   						pthread_join(ctid,NULL);
						next = j+1;
						break;
					}
					else
						j++;
					count++;
				}
				k++;
				count = 0;
			}

			fflush(stdin);
			fflush(stdout);

			tdetail.groupid = gid;
			string stfile(a[2]);
			tdetail.filename = stfile;
			string stip(ip);
			tdetail.ip = stip;
			tdetail.port = portadd;
			tdetail.noofchunks = noofchunks;
			tdetail.filesize = filesize;

	//		sharablefile(sockfd, tdetail);
	/*		char tes1[] = "hello";
     		send(sockfd, tes1, sizeof(tes1), 0);
   		 
			int x;
			char us2[1000];
			char utip[100];

	//		recv(sockfd,&x,sizeof(x),0);

			strcpy(us2,tdetail.filename.c_str());
    		cout<<"sharablefile filename at client "<<us2<<endl;
    		send(sockfd,us2,sizeof(us2),0);

    //		recv(sockfd,&x,sizeof(x),0);

    		time_t my_time = time(NULL); 
    		printf("time at client %s", ctime(&my_time));
    //		cout<<"644"<<endl;
    		int uchunkcount = tdetail.noofchunks;
    		cout<<"sharablefile chunkcount at client "<<uchunkcount<<endl;
    		send(sockfd,&uchunkcount, sizeof(uchunkcount),0);
    //		cout<<"646"<<endl;
    		// char tes[]="hello";
    		// send(sockfd, tes, sizeof(tes), 0);
    //		recv(sockfd,&x,sizeof(x),0);

    		int utport = tdetail.port;
    		cout<<"sharablefile port at client "<<utport<<endl;
    		send(sockfd, &utport, sizeof(utport), 0);

    //		recv(sockfd,&x,sizeof(x),0);
    //		cout<<"649"<<endl;
    		strcpy(utip,tdetail.ip.c_str());
    		cout<<"sharablefile ip at client "<<utip<<endl;
    		send(sockfd, utip, sizeof(utip), 0);

    		//recv(sockfd,&x,sizeof(x),0);
    //		cout<<"652"<<endl;
    		string utemp="";
    		for(int e = 0;e<uchunkcount;e++)
    		{
    			int uq= tempchunks.at(e);
    			cout<<"sharablefile chunks at client "<<uq<<endl;
    			send(sockfd,&uq,sizeof(uq),0);

    	//		recv(sockfd,&x,sizeof(x),0);

    			char uthash[20];
    			utemp = utemp+temphashes.at(e);
    			strcpy(uthash,temphashes.at(e).c_str());
    			cout<<"sharablefile hash at client "<<uthash<<endl;
    			send(sockfd,uthash,sizeof(uthash), 0);

    	//		recv(sockfd,&x,sizeof(x),0);
   			}

    		int ugid = tdetail.groupid;
    		cout<<"sharablefile group id at client "<<ugid<<endl;
    		send(sockfd, &ugid, sizeof(ugid), 0);

    	//	recv(sockfd,&x,sizeof(x),0);

    		ll usize = tdetail.filesize;
    		cout<<"sharablefile filesize at client "<<usize<<endl;
    		send(sockfd, &usize, sizeof(usize),0);

    	//	recv(sockfd,&x,sizeof(x),0);

    		string finalst = globaluserid+" "+to_string(ugid)+" "+tdetail.ip+" "+to_string(tdetail.port)+" "+tdetail.filename+" "+to_string(usize)+" "+utemp;
    		char sendingchar[100000];
    		cout<<"sharablefile final string at client "<<finalst<<endl;
    		strcpy(sendingchar,finalst.c_str());
    		send(sockfd, &sendingchar, sizeof(sendingchar),0); 

			temphashes.clear();
			tempchunks.clear(); */

			if(isnotintegrated == 1)
			{
				cout<<"File has been corrupted"<<endl;
			}
			else
			{
				cout<<"File is perfect"<<endl;
			}

			fflush(stdin);
			fflush(stdout);
			fclose(fpc);		
		}

		else
		{
			cout<<"Command not found"<<endl;
		}
		fflush(stdin);
		fflush(stdout);
	//	cout<<"end"<<endl;
	}
//	pthread_join( thread1 , NULL);
	close( sockfd);
	return 0;
}
