#include <iostream>
#include <sstream>
#include<unistd.h>
#include<string>
#include<cstring>
#include<stdio.h>
#include<cstdlib>
#include<stdlib.h>
#include <string>
#include<sys/wait.h>
#include<pthread.h>
#include<vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <fstream> 
#include <algorithm>
#include <time.h>

using namespace std;
#define ll long long int

class Package{
public:
	string uid;
	string groupid;
	string ipaddr;
	string port;
	string hash;
	string filename;
	string filesize;
};

struct chunkdetail{
public:
	string filename;
	int port;
	string ip;
	int groupid;
	int noofchunks;
	ll filesize;
	vector<int> chunks;
	vector<string> hashes;
};

struct userdetail{
public:
	string userid;
	string password;
	bool islogin;
};

struct groupown
{
public:
	string userid;
	vector<int> groupids;
};

struct group
{
public:
	int groupid;
	vector<string> userids;
	vector<string> pendreqs;
};

struct groupfiles
{
public:
	int groupid;
	vector<pair<string,bool>> files;
};

vector<userdetail> vuser;
vector<groupown> vgroupown;
vector<group> vgroup;
vector<Package> v;
vector<groupfiles> vgrpfiles;
vector<chunkdetail> vchunkdetail;

void processcommand(char *a[],char cmd[])
{
	char* token = strtok(cmd," ");
	int i = 0;
	while(token != NULL)
	{
		a[i] = token; 
		i++;
        token = strtok(NULL, " "); 
	}
	return;
}

void uplpoadsharableinfo(int sockfd)
{
	char s2[1000];
	char s1[100000];
	char s[100000];
	char ttip[100];

	chunkdetail cdetail;
	//string s2;
 	//while ((n = recv( sockfd , s ,sizeof(s), 0)) > 0){
	recv(sockfd, s2,sizeof(s2),0);
	cout<<"uplpoadsharableinfo recieved filename "<<s2<<endl;
	string ft(s2);
	cdetail.filename = ft;
	//cout<<"uplpoadsharableinfo filename at tracker "<<cdetail.filename<<endl;

	int chunkcount = 0;
	recv(sockfd, &chunkcount, sizeof(chunkcount),0);
	cout<<"uplpoadsharableinfo recieved chunkcount "<<chunkcount<<endl;
	cdetail.noofchunks = chunkcount;
//	cout<<"chunkcount at tracker "<<cdetail.noofchunks<<endl;

    char tes[10];
    recv(sockfd, tes, sizeof(tes), 0);
   	cout<<"test "<<tes<<endl;

	int ttport;
	recv(sockfd,&ttport,sizeof(ttport),0);
	cout<<"uplpoadsharableinfo recieved port "<<ttport<<endl;
	cdetail.port = ttport;
//	cout<<"port at tracker "<<cdetail.port<<endl;

	recv(sockfd, ttip,sizeof(ttip), 0);
	cout<<"uplpoadsharableinfo recieved ip "<<ttip<<endl;
	string tp(ttip);
	cdetail.ip = tp;
//	cout<<"ip at tracker "<<cdetail.ip<<endl;

	for(int p = 0;p<chunkcount; p++)
	{
		int chunkv;
		char h[20];

		recv(sockfd,&chunkv, sizeof(chunkv), 0);
		cout<<"uplpoadsharableinfo chunks at tracker "<<chunkv<<endl;
		cdetail.chunks.push_back(chunkv);

		recv(sockfd,h,sizeof(h),0);
		string temp(h);
		cout<<"uplpoadsharableinfo hashes at tracker "<<temp<<endl;
		cdetail.hashes.push_back(temp);
	}

	int tgid;
	recv(sockfd,&tgid,sizeof(tgid),0);
	cout<<"uplpoadsharableinfo recieved group id "<<tgid<<endl;
	cdetail.groupid = tgid;
//	cout<<"uplpoadsharableinfo group id at tracker "<<cdetail.groupid<<endl;

	ll size;
	recv(sockfd,&size, sizeof(size),0);
	cout<<"uplpoadsharableinfo recieved filesize "<<size<<endl;
	cdetail.filesize = size;
//	cout<<"filesize at tracker "<<cdetail.filesize<<endl;

	vchunkdetail.push_back(cdetail);

	fflush(stdin);
	fflush(stdout);
	memset(s,'\0',sizeof(s));
	recv(sockfd , s ,sizeof(s), 0);

	cout<<"uplpoadsharableinfo recieved s "<<s<<endl;
	ofstream fileobj("client_info.txt",ios::out | ios::app);
	//cout<<s<<endl;
	strcpy(s1,s);
	int len = strlen(s1);
	s1[len]='\0';
	cout<<s1<<endl;
	fileobj<<s1;
	fileobj<<'\n';

	fileobj.close();
}

void* processthread(void *sockdesc)
{
	char command[1000];
	int sockfd = *((int *)sockdesc);
	string globaluserid;
	int isloggedin = 0;
	while(1)
	{
		char *a[1000];
		char cmd[10000];

		recv(sockfd, &command, sizeof(command), 0);
	//	cout<<"command "<<command<<endl;
	/*	if(strcmp(command,"test") == 0)
		{
			cout<<"a0 "<<a[0]<<endl;
			cout<<"a1 "<<a[1]<<endl;
		} */

		if(strcmp(command,"create_user") == 0)
 		{
 			cout<<"create_user"<<endl;	
 			recv(sockfd, &cmd, sizeof(cmd), 0);
			processcommand(a,cmd);

 			int iscorrect = 1;
 			vector<userdetail>::iterator it;
 			for(it = vuser.begin();it != vuser.end(); it++)
 			{
 		//		cout<<"here create user"<<endl;
 				if(strcmp((*it).userid.c_str(),a[0]) == 0)
 				{
 					iscorrect = 0;
 					send(sockfd, &iscorrect, sizeof(iscorrect), 0);
 					continue;
 				}
 			}
 			send(sockfd, &iscorrect, sizeof(iscorrect), 0);
 			userdetail ud;
 			string st1(a[0]);
 			string st2(a[1]);
 			ud.userid = st1;
 			ud.password = st2;
 	//		cout<<"uid "<<ud.userid<<endl;
 	//		cout<<"pwd "<<ud.password<<endl;
 			vuser.push_back(ud);
 			fflush(stdin);
			fflush(stdout);
 		}

 		else if(strcmp(command,"login") == 0)
 		{
 			cout<<"login"<<endl;
 			recv(sockfd, &cmd, sizeof(cmd), 0);
			processcommand(a,cmd);

 			int iscorrect = 1;
 			vector<userdetail>::iterator it;
 			for(it = vuser.begin(); it != vuser.end(); it++)
 			{
 				if(strcmp(a[0],(*it).userid.c_str()) == 0 && strcmp(a[1],(*it).password.c_str()) == 0)
 				{
 					globaluserid = a[0];
 					isloggedin = 1;
 					(*it).islogin =  true;
 					break;
 				}
 			}
 			if(isloggedin == 0)
 			{
 				iscorrect = 0;
 				send(sockfd, &iscorrect, sizeof(iscorrect), 0);
 				continue;
 			}
 			send(sockfd, &iscorrect, sizeof(iscorrect), 0);
 			fflush(stdin);
			fflush(stdout);
 		}

 		// if(isloggedin == 0)
 		// {
 		// 	cout<<"User is not logged in"<<endl;
 		// 	continue;
 		// }

 		else if(strcmp(command,"list_groups") == 0)
 		{
 			cout<<"list_groups"<<endl;
 			vector<group>::iterator it;
 			int numofgroups = vgroup.size();
 			send(sockfd, &numofgroups, sizeof(numofgroups), 0);
 			for(it = vgroup.begin(); it != vgroup.end(); it++)
 			{
 				int x = (*it).groupid;
 				send(sockfd, &x, sizeof(x), 0);
 			}
 			fflush(stdin);
			fflush(stdout);
 		//	cout<<"end list_groups in tracker"<<endl;
 		}

 		else if(strcmp(command,"join_group") == 0)
 		{
 			cout<<"join_group"<<endl;
 			recv(sockfd, &cmd, sizeof(cmd), 0);
			processcommand(a,cmd);

 			vector<group>::iterator it;
 			for(it = vgroup.begin(); it != vgroup.end(); it++)
 			{
 				if((*it).groupid == atoi(a[0]))
 				{
 					(*it).pendreqs.push_back(globaluserid);
 				}
 			}
 			fflush(stdin);
			fflush(stdout);
 		}

 		else if(strcmp(command,"leave_group") == 0)
 		{
 			cout<<"leave_group"<<endl;
 			recv(sockfd, &cmd, sizeof(cmd), 0);
			processcommand(a,cmd);

			int ispre = 0;
 			vector<group>::iterator it;
 			for(it = vgroup.begin(); it != vgroup.end(); it++)
 			{
 				if((*it).groupid == atoi(a[0]))
 				{
 					vector<string>::iterator i;
 					for(i = (*it).userids.begin(); i != (*it).userids.end(); i++)
 					{
 						if((*i) == globaluserid)
 						{
 							(*it).userids.erase(i);
 							ispre = 1;
 							break;
 						}
 					}
 				}
 				if(ispre == 1)
 					break;
 			}
 			fflush(stdin);
			fflush(stdout);
 		}

 		else if(strcmp(command,"list_requests") == 0)
 		{
 			cout<<"list_requests"<<endl;
 			recv(sockfd, &cmd, sizeof(cmd), 0);
			processcommand(a,cmd);

 			int iscorrect = 1;
 			vector<groupown>::iterator it;
 			//it = vgroupown.find(globaluserid);
 		//	cout<<"globaluserid "<<globaluserid<<endl;
 			for(it = vgroupown.begin(); it != vgroupown.end(); it++)
 			{
 				if((*it).userid == globaluserid)
 					break;
 			}
 			if(it != vgroupown.end())
 			{
 				vector<int>::iterator i;
 				i = find((*it).groupids.begin(),(*it).groupids.end(),atoi(a[0]));
 				if(i != (*it).groupids.end())
 				{
 					vector<group>::iterator j;
 					for(j = vgroup.begin(); j != vgroup.end(); j++)
 					{
 						if((*j).groupid == atoi(a[0]))
 						{
 							vector<string>::iterator l;
 							send(sockfd, &iscorrect, sizeof(iscorrect), 0);
 							int numofpendreqs = (*j).pendreqs.size();
 							send(sockfd, &numofpendreqs, sizeof(numofpendreqs), 0);
 							for(l = (*j).pendreqs.begin(); l!= (*j).pendreqs.end(); l++)
 							{
 								char buffer[512];
 								strcpy(buffer,(*l).c_str());
 								send(sockfd, &buffer, sizeof(buffer), 0);
 							}
 						}
 					}
 				}
 				else
 				{
 					iscorrect = 0;
 					send(sockfd, &iscorrect, sizeof(iscorrect), 0);
 					continue;
 				}
 			}
 			else
 			{
 				iscorrect = 0;
 				send(sockfd, &iscorrect, sizeof(iscorrect), 0);
 				continue;
 			}
 			fflush(stdin);
			fflush(stdout);
 			//cout<<"end list_requests in tracker"<<endl;
 		}

 		else if(strcmp(command,"list_files") == 0)
 		{
 			cout<<"list_files"<<endl;
 			recv(sockfd, &cmd, sizeof(cmd), 0);
			processcommand(a,cmd);

 			int ispre = 0;
 			vector<groupfiles>::iterator it;
 			for(it = vgrpfiles.begin(); it != vgrpfiles.end(); it++)
 			{
 				if((*it).groupid == atoi(a[0]))
 				{
 					ispre = 1;
 					send(sockfd, &ispre, sizeof(ispre), 0);
 					vector<pair<string,bool>>::iterator i;
 					int numofpendfiles = (*it).files.size();
 					send(sockfd, &numofpendfiles, sizeof(numofpendfiles), 0);
 					for(i = (*it).files.begin(); i != (*it).files.end(); i++)
 					{
 						char buffer[1000];
 						strcpy(buffer, i->first.c_str());
 						send(sockfd, buffer, sizeof(buffer) ,0);
 					}
 				}
 			}
 			if(ispre == 0)
 			{
 				send(sockfd, &ispre, sizeof(ispre), 0);
 				continue;
 			}
 			fflush(stdin);
			fflush(stdout);
 		}

 		else if(strcmp(command,"create_group") == 0)
 		{
 			cout<<"create_group"<<endl;
 			recv(sockfd, &cmd, sizeof(cmd), 0);
			processcommand(a,cmd);

 			int ispre = 0;
 			vector<group>::iterator it;
 			for(it = vgroup.begin(); it != vgroup.end(); it++)
 			{
 				if((*it).groupid == atoi(a[0]))
 				{
 					ispre = 1;
 					break;
 				}
 			}

 			send(sockfd ,&ispre, sizeof(ispre), 0);
 			if(ispre == 0)
 			{
 				vector<groupown>::iterator i;
 				for(i = vgroupown.begin(); i != vgroupown.end(); i++)
 				{
 					if((*i).userid == globaluserid)
 						break;
 				}

 				if(i != vgroupown.end())
 				{
 					(*i).groupids.push_back(atoi(a[0]));
 				}
 				else
 				{
 					groupown g;
 					g.userid = globaluserid;
 					g.groupids.push_back(atoi(a[0]));
 					vgroupown.push_back(g);
 					
 				}
 				group gr;
 				gr.groupid = atoi(a[0]);
 				gr.userids.push_back(globaluserid);
 				vgroup.push_back(gr);
 			}
 			fflush(stdin);
			fflush(stdout);
  		}

  		else if(strcmp(command,"accept_request") == 0)
  		{
  			cout<<"accept_request"<<endl;
 			recv(sockfd, &cmd, sizeof(cmd), 0);
			processcommand(a,cmd);

			int isowner = 0;
			vector<groupown>::iterator k;
			for(k = vgroupown.begin(); k != vgroupown.end(); k++)
 			{
 				if((*k).userid == globaluserid)
 				{
 					vector<int>::iterator j;
 					for(j = (*k).groupids.begin(); j != (*k).groupids.end(); j++)
 					{
 						if((*j) == atoi(a[0]))
 							isowner = 1;
 					}
 				}
 			}

 			if(isowner == 0)
 			{
 				continue;
 			}
 			
 		//	cout<<"groupid "<<a[0]<<endl;
 		//	cout<<"userid "<<a[1]<<endl;
 			int ispre =0;
			vector<group>::iterator it;
			for(it = vgroup.begin(); it != vgroup.end(); it++)
			{
				if((*it).groupid == atoi(a[0]))
				{
				//	cout<<"groupid matched"<<endl;
					vector<string>::iterator i;
					for(i = (*it).pendreqs.begin(); i != (*it).pendreqs.end(); i++)
					{
						if(strcmp((*i).c_str(), a[1]) == 0)
						{
//							cout<<"user id "<<(*i)<<endl;
							(*it).pendreqs.erase(i);
							string st(*i);
							(*it).userids.push_back(st);
//							cout<<"userid matched "<<st<<endl;
							ispre =1;
							break;
						}
					}
				}
				if(ispre == 1)
					break;
			}
  		}

  		else if(strcmp(command,"stop_share") == 0)
  		{
  			cout<<"stop_share"<<endl;
 			recv(sockfd, &cmd, sizeof(cmd), 0);
			processcommand(a,cmd);

			int isowner = 0;
			vector<groupown>::iterator k;
			for(k = vgroupown.begin(); k != vgroupown.end(); k++)
 			{
 				if((*k).userid == globaluserid)
 				{
 					vector<int>::iterator j;
 					for(j = (*k).groupids.begin(); j != (*k).groupids.end(); j++)
 					{
 						if((*j) == atoi(a[0]))
 						{
 							isowner = 1;
 							break;
 						}
 					}
 				}
 				if(isowner == 1)
 					break;
 			}

 			if(isowner == 0)
 			{
 				continue;
 			}

			vector<groupfiles>::iterator it;
			for(it = vgrpfiles.begin(); it != vgrpfiles.end(); it++)
			{
				if((*it).groupid == atoi(a[0]))
				{
					vector<pair<string,bool>>::iterator m;
					for(m = (*it).files.begin(); m != (*it).files.end(); m++)
					{
						if(strcmp(m->first.c_str(),a[1]) == 0)
						{
	//						cout<<"here in stop share"<<endl;
							m->second = false;
						}
					}
				}
			}
			fflush(stdin);
			fflush(stdout);
  		}

  		else if(strcmp(command,"logout") == 0)
  		{
  			cout<<"logout"<<endl;
  			vector<userdetail>::iterator it;
  			for(it = vuser.begin(); it != vuser.end(); it++)
  			{
  				if((*it).userid == globaluserid)
  				{
  					(*it).islogin = false;
  					break;
  				}
  			}
  			fflush(stdin);
			fflush(stdout);
  		}

  		else if(strcmp(command,"upload_file") == 0)
  		{
			cout<<"upload"<<endl;
			char s[100000];
			char s1[100000];
			char s2[100];
			char ttip[100];
			int isgrppre = 0;
			int isfilepre = 0;
			chunkdetail cdetail;
	//string s2;
 	//while ((n = recv( sockfd , s ,sizeof(s), 0)) > 0){
			recv(sockfd, s2,sizeof(s2),0);
			cout<<"recieved filename "<<s2<<endl;
			string ft(s2);
			cdetail.filename = ft;
	//		cout<<"filename at tracker "<<cdetail.filename<<endl;

			int chunkcount = 0;
			recv(sockfd, &chunkcount, sizeof(chunkcount),0);
			cout<<"recieved chunkcount "<<chunkcount<<endl;
			cdetail.noofchunks = chunkcount;
	//		cout<<"chunkcount at tracker "<<cdetail.noofchunks<<endl;

			char tes[10];
    		recv(sockfd, tes, sizeof(tes), 0);
   // 		cout<<"test "<<tes<<endl;

			int ttport;
			recv(sockfd,&ttport,sizeof(ttport),0);
			cout<<"recieved port "<<ttport<<endl;
			cdetail.port = ttport;
	//		cout<<"port at tracker "<<cdetail.port<<endl;

			recv(sockfd, ttip,sizeof(ttip), 0);
			cout<<"recieved ip "<<ttip<<endl;
			string tp(ttip);
			cdetail.ip = tp;
	//		cout<<"ip at tracker "<<cdetail.ip<<endl;

			for(int p = 0;p<chunkcount; p++)
			{
				int chunkv;
				char h[20];

				recv(sockfd,&chunkv, sizeof(chunkv), 0);
				cout<<"chunks at tracker "<<chunkv<<endl;
				cdetail.chunks.push_back(chunkv);

				recv(sockfd,h,sizeof(h),0);
				string temp(h);
				cout<<"hashes at tracker "<<temp<<endl;
				cdetail.hashes.push_back(temp);
			}

			int tgid;
			recv(sockfd,&tgid,sizeof(tgid),0);
			cout<<"recieved group id "<<tgid<<endl;
			cdetail.groupid = tgid;
	//		cout<<"group id at tracker "<<cdetail.groupid<<endl;

			ll size;
			recv(sockfd,&size, sizeof(size),0);
			cout<<"recieved filesize "<<size<<endl;
			cdetail.filesize = size;
	//		cout<<"filesize at tracker "<<cdetail.filesize<<endl;

			vchunkdetail.push_back(cdetail);

			fflush(stdin);
			fflush(stdout);
			memset(s,'\0',sizeof(s));
			recv(sockfd , s ,sizeof(s), 0);
		//	string ts(s);
		//	memset(s,'\0',sizeof(s));
		//	ts.erase(0,4);
		//	strcpy(s,ts.c_str());
			cout<<"recieved s "<<s<<endl;
			ofstream fileobj("client_info.txt",ios::out | ios::app);
	//cout<<s<<endl;
			strcpy(s1,s);
			int len = strlen(s1);
			s1[len]='\0';
		//	cout<<s1<<endl;
			fileobj<<s1;
			fileobj<<'\n';
			char* id = strtok(s," ");
			int gid = atoi(strtok(NULL," "));
			char* uip = strtok(NULL," ");
			char* uport = strtok(NULL," ");
			char* ufilename = strtok(NULL," ");

			vector<groupfiles>::iterator it;
			for(it = vgrpfiles.begin(); it != vgrpfiles.end(); it++)
			{
				if((*it).groupid == gid)
				{
					vector<pair<string,bool>>::iterator m;
					for(m = (*it).files.begin(); m != (*it).files.end(); m++)
					{
						if(strcmp(m->first.c_str(),ufilename) == 0)
						{
							m->second = true;
							isfilepre =1;
						}
					}
					isgrppre = 1;
					if(isfilepre == 0)
					{
						string t(ufilename);
						(*it).files.push_back(make_pair(t,true));
					}
				}
			}

			if(isgrppre == 0)
			{
				groupfiles gf;
				gf.groupid = gid;
				string t(ufilename);
				gf.files.push_back(make_pair(t,true));
				vgrpfiles.push_back(gf);
			}

			memset ( s , '\0', sizeof(s));
			memset ( s1 , '\0', sizeof(s1));
	//s2.clear();
	//fileobj << s;
	//file_size = file_size - n;
			fileobj.close();
			fflush(stdin);
			fflush(stdout);
 	// }
  		}

		else if(strcmp(command,"download_file") == 0)
		{	
			cout<<"download"<<endl;
			int issamegroup = 0;
			int issharable = 0;
			int isuserlogout = 0;
			int gid;
			int countpeer = 0;
			recv(sockfd, &gid, sizeof(gid), 0);

			vector<group>::iterator it;
			for(it = vgroup.begin(); it!=vgroup.end(); it++)
			{
				if((*it).groupid == gid)
				{
	//				cout<<"group id "<<(*it).groupid<<" "<<gid<<endl;
					vector<string>::iterator s;
					for(s = (*it).userids.begin(); s!=(*it).userids.end(); s++)
					{
	//					cout<<"globaluserid "<<globaluserid<<endl;
						if((*s) == globaluserid)
						{
	//						cout<<"same group check"<<endl;
							issamegroup = 1;
							break;
						}
					}
				}
				if(issamegroup == 1)
					break;
			}

			send(sockfd, &issamegroup, sizeof(issamegroup), 0);
			if(issamegroup == 0)
				continue;

			char s[100000];
			recv(sockfd, &s, sizeof(s), 0);

			vector<groupfiles>::iterator vg;
			for(vg = vgrpfiles.begin(); vg != vgrpfiles.end(); vg++)
			{
				if((*vg).groupid == gid)
				{
					vector<pair<string, bool>>::iterator sf;
					for(sf = (*vg).files.begin(); sf != (*vg).files.end(); sf++)
					{
						if(strcmp(sf->first.c_str(),s) == 0 && sf->second == true)
						{
	//						cout<<"sharable check"<<endl;
							issharable = 1;
							break;
						}
					}
				}
				if(issharable == 1)
					break;
			}

			send(sockfd, &issharable, sizeof(issharable), 0);
			if(issharable == 0)
				continue;

    		vector<chunkdetail>::iterator cd;
			for(cd = vchunkdetail.begin(); cd != vchunkdetail.end(); cd++)
			{
				if(strcmp(s, (*cd).filename.c_str())==0 && (*cd).groupid == gid)
					countpeer++;
			}
			send(sockfd,&countpeer, sizeof(countpeer), 0);

			for(cd = vchunkdetail.begin(); cd != vchunkdetail.end(); cd++)
			{
				cout<<"filename "<<(*cd).filename<<" port "<<(*cd).port<<" ip "<<(*cd).ip<<" no of chunks "<<(*cd).noofchunks<<" chunks ";
				vector<int>::iterator cin;
				for(cin = (*cd).chunks.begin(); cin != (*cd).chunks.end(); cin++)
				{
					cout<<(*cin)<<" ";
				}
				cout<<" hashes ";
				vector<string>::iterator h;
				for(h = (*cd).hashes.begin(); h != (*cd).hashes.end(); h++)
				{
					cout<<(*h)<<" ";
				}
				cout<<endl;
			}

			for(cd = vchunkdetail.begin(); cd != vchunkdetail.end(); cd++)
			{
				if(strcmp(s, (*cd).filename.c_str())==0 && (*cd).groupid == gid)
				{
					char vfile[100];
					strcpy(vfile,(*cd).filename.c_str());
					cout<<"filename sent from tracker "<<vfile<<endl;
					send(sockfd, vfile, sizeof(vfile), 0);

					int vport = (*cd).port;
					cout<<"port sent from tracker "<<vport<<endl;
					send(sockfd, &vport, sizeof(vport), 0);

					char vip[100];
					strcpy(vip,(*cd).ip.c_str());
					cout<<"ip sent from tracker "<<vip<<endl;
					send(sockfd,vip, sizeof(vip), 0);

					int vnoofchunks = (*cd).noofchunks;
					cout<<"no of chunks sent from tracker "<<vnoofchunks<<endl;
					send(sockfd, &vnoofchunks, sizeof(vnoofchunks), 0);

					vector<int>::iterator cin;
					for(cin = (*cd).chunks.begin(); cin != (*cd).chunks.end(); cin++)
					{
						int tin = (*cin);
						cout<<"chunks sent from tracker "<<tin<<endl;
						send(sockfd, &tin, sizeof(tin), 0);
					}

					vector<string>::iterator h;
					for(h = (*cd).hashes.begin(); h != (*cd).hashes.end(); h++)
					{
						char hs[100];
						strcpy(hs,(*h).c_str());
						cout<<"hash sent from tracker "<<hs<<endl;
						send(sockfd, hs, sizeof(hs), 0);
					}

					ll vsize = (*cd).filesize;
					cout<<"filesize sent from tracker "<<vsize<<endl;
					send(sockfd, &vsize, sizeof(vsize), 0);
				}
			}

		//	uplpoadsharableinfo(sockfd);
		/*	int x=0;

			char tes1[100];
     		recv(sockfd, tes1, sizeof(tes1), 0);
     		cout<<"test "<<tes1<<endl;

			char us2[1000];
			char us1[100000];
			char us[100000];
			char uttip[100];

			chunkdetail ucdetail;

		//	send(sockfd,&x,sizeof(x),0);
	//string s2;
 	//while ((n = recv( sockfd , s ,sizeof(s), 0)) > 0){
			recv(sockfd, us2,sizeof(us2),0);
			cout<<"uplpoadsharableinfo recieved filename "<<us2<<endl;
			string uft(us2);
			ucdetail.filename = uft;

		//	send(sockfd,&x,sizeof(x),0);

			time_t my_time = time(NULL); 
  		    printf("time at tracker %s", ctime(&my_time));
	//cout<<"uplpoadsharableinfo filename at tracker "<<cdetail.filename<<endl;

			int uchunkcount = 0;
			recv(sockfd, &uchunkcount, sizeof(uchunkcount),0);
			cout<<"uplpoadsharableinfo recieved chunkcount "<<uchunkcount<<endl;
			ucdetail.noofchunks = uchunkcount;
//	cout<<"chunkcount at tracker "<<cdetail.noofchunks<<endl;
		//	send(sockfd,&x,sizeof(x),0);
    		// char tes[10];
    		// recv(sockfd, tes, sizeof(tes), 0);
   			// cout<<"test "<<tes<<endl;

			int uttport;
			recv(sockfd,&uttport,sizeof(uttport),0);
			cout<<"uplpoadsharableinfo recieved port "<<uttport<<endl;
			ucdetail.port = uttport;
//	cout<<"port at tracker "<<cdetail.port<<endl;
		//	send(sockfd,&x,sizeof(x),0);

			recv(sockfd, uttip,sizeof(uttip), 0);
			cout<<"uplpoadsharableinfo recieved ip "<<uttip<<endl;
			string utp(uttip);
			ucdetail.ip = utp;
//	cout<<"ip at tracker "<<cdetail.ip<<endl;
		//	send(sockfd,&x,sizeof(x),0);

			for(int p = 0;p<uchunkcount; p++)
			{
				int uchunkv;
				char uh[20];

				recv(sockfd,&uchunkv, sizeof(uchunkv), 0);
				cout<<"uplpoadsharableinfo chunks at tracker "<<uchunkv<<endl;
				ucdetail.chunks.push_back(uchunkv);

		//		send(sockfd,&x,sizeof(x),0);

				recv(sockfd,uh,sizeof(uh),0);
				string temp(uh);
				cout<<"uplpoadsharableinfo hashes at tracker "<<temp<<endl;
				ucdetail.hashes.push_back(temp);

		//		send(sockfd,&x,sizeof(x),0);
			}

			int utgid;
			recv(sockfd,&utgid,sizeof(utgid),0);
			cout<<"uplpoadsharableinfo recieved group id "<<utgid<<endl;
			ucdetail.groupid = utgid;
//	cout<<"uplpoadsharableinfo group id at tracker "<<cdetail.groupid<<endl;
		//	send(sockfd,&x,sizeof(x),0);

			ll usize;
			recv(sockfd,&usize, sizeof(usize),0);
			cout<<"uplpoadsharableinfo recieved filesize "<<usize<<endl;
			ucdetail.filesize = usize;
//	cout<<"filesize at tracker "<<cdetail.filesize<<endl;

			vchunkdetail.push_back(ucdetail);

		//	send(sockfd,&x,sizeof(x),0);

			fflush(stdin);
			fflush(stdout);
			memset(us,'\0',sizeof(us));
			recv(sockfd , us ,sizeof(us), 0);

			cout<<"uplpoadsharableinfo recieved s "<<us<<endl;
			ofstream ufileobj("client_info.txt",ios::out | ios::app);
	//cout<<s<<endl;
			strcpy(us1,us);
			int ulen = strlen(us1);
			us1[ulen]='\0';
		//	cout<<us1<<endl;
			ufileobj<<us1;
			ufileobj<<'\n';

			ufileobj.close();

			// char tes1[100];
   //  		recv(sockfd, tes1, sizeof(tes1), 0);
   //  		cout<<"test "<<tes1<<endl;  */

		}
		memset(command,'\0',sizeof(command));
		fflush(stdin);
		fflush(stdout);
	}
	
}

int main()
{
	int server_fd = socket (AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in   addr;
	struct sockaddr_in address;
	bzero(&addr, sizeof(addr));
	bzero(&address, sizeof(address));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(10011);
	addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	int addrlen = sizeof(sockaddr_in);
	//cout<<"addrlen "<<addrlen<<endl;
	bind (server_fd  , (struct sockaddr *)&addr , sizeof ( addr ) );
	int l=listen (server_fd, 3);
	cout<<"l "<<l<<endl;
//	int sockfd = accept(server_fd , (struct sockaddr *)&address , (socklen_t*)&addrlen);
//	cout<<"sockfd "<<sockfd<<endl;
	int sockfd;
	char command[1000];
	pthread_t tid[1000];
	int i=0;
	while(1)
	{
		sockfd = accept(server_fd , (struct sockaddr *)&address , (socklen_t*)&addrlen);
		cout<<"sockfd "<<sockfd<<endl;
		if( pthread_create(&tid[i], NULL, processthread, &sockfd) != 0 )
           printf("Failed to create thread\n");
      	int ret = pthread_detach(tid[i]);
      	i++;
	}
//	close( sockfd);
	close( server_fd);
	return 0;
}
