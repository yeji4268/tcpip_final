#include <stdio.h>  
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

// 클라이언트로부터 받은 메시지 처리
void *handle_clnt(void *arg); 

// 클라이언트로부터 메시지를 수신
void send_msg(char *msg, int len); 

// 에러 처리 
void error_handling(char *msg); 

// 클라이언트 개수 초기값 : 0  
int clnt_cnt = 0 ; 

// 클라이언트 소켓  
int clnt_socks[MAX_CLNT]; 

// mutex 객체  
pthread_mutex_t mutx ; 

int main(int argc, char &argv[])
{
	// 서버 측 소켓, 클라이언트 측 소켓 
	int serv_sock, clnt_sock; 
	
	// 서버 측 주소, 클라이언트 측 주소  
	struct sockaddr_in serv_adr, clnt_adr ; 
	
	// 클라이언트 주소 사이즈  
	int clnt_adr_sz ; 
	
	// 쓰레드 식별자  
	pthread_t t_id ;
	
	// main 함수에 전달되는 정보의 개수(argc)가 2개가  아닌 경우 
	// 즉, 실행 시 실행경로(실행 파일 이름)와 포트 번호가 입력되지 않은 경우 올바른 사용법을 안내한다.  
	if(argc != 2){
		printf("Usage : %s <port> \n", argv[0]); 
		
		// 프로그램 종료  
		exit(1); 
	}
	
	// mutext 객체 초기화 
	// pthread_mutex_init() 함수의 첫번째 매개변수는 초기화 시킬 mutex 객체, 두번째 매개변수는 뮤텍스의 특징이다. 
	// 첫 번째 인자 : mutx, 두 번째 인자 : NULL (기본 값)
	// 반환 값 : 성공(0), 실패(-1)   
	pthread_mutex_init(&mutx, NULL); 
	
	// 서버 측 소켓 생성
	// socket() 함수는 매개변수로 domain(인터넷 통신/ 같은 시스템 내 통신 설정), type (데이터의 전송 형태)
	// , protocol(프로토콜 지정)을 사용한다. 
	// 첫 번째 인자 : PF_INET(IPv4 인터넷 프로토콜 사용), 두 번째 인자 : SOCK_STREAM(TCP/IP 프로토콜 사용)
	// 세 번째 인자 : 0 (자동으로 type에서 지정)
	// 반환 값 : 성공(0 이상의 값), 실패(-1)  
	serv_sock = socket(PF_INET, SOCK_STREAM, 0); 
	
	// 메모리 세팅 : 생성된 소켓 초기화 목적 (쓰레기 값 제거) 
	// memset() 함수는 매개변수로 *ptr(세팅하고자 하는 메모리의 시작 주소), value(메모리에 세팅하고자 하는 값)
	// , num(길이, 보통 sizeof(데이터)의 형태로 작성)을 사용한다. 
	// 첫 번째 인자 : &serv_sock(서버 측 소켓에서), 두 번째 인자 : 0 (0으로), 세 번째 인자 : size(serv_adr) (서버 측 주소의 길이만큼 설정)
	// 반환 값 : 성공(ptr), 실패(NULL) 
	memset(&serv_adr, 0, sizeof(serv_adr)); 
	
	// 서버 주소 체계 : AF_INET(IPv4) 
	serv_adr.sin_family = AF_INET ; 
	
	// 서버 IP 주소 설정 : htonl(INADDR_ANY)
	// INADDR_ANY : IP 주소를 자동으로 찾아서 대입 (해당 포트를 목적지로 하는 모든 연결 요청에 대해 해당 프로그램에서 처리) 
	// htonl : 호스트 바이트 순서를 따르는 데이터를 네트워크 바이트 순서로 변환(long integer를 네트워크 바이트 순서로)  
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY); 
	
	// 포트 설정 : htons(atoi(argv[1]))
	// argv[1] : 실행 시 입력된 인자 중 두 번째 값(포트번호) 
	// atoi : 문자열 형태로 입력된 포트 번호를 정수로 변환  
	// htons :  호스트 바이트 순서를 따르는 데이터를 네트워크 바이트 순서로 변환(short integer를 네트워크 바이트 순서로) 
	serv_adr.sin_port = htons(atoi(argv[1])); 
	
	// 소켓과 서버의 정보를 바인딩 
	// bind() 함수는 sockfd(소켓 디스크립터), *myaddr(서버의 IP 주소), addrlen(주소의 길이)를 매개변수로 사용한다. 
	// 첫 번째 인자 : serv_sock(서버 소켓), 두 번째 인자 : (struct sockaddr *)&serv_adr(서버의 IP 주소), 세 번째 인자 : sizeof(serv_adr)(서버 주소의 길이)
	// 반환 값 : 성공(0), 실패(-1) 
	if(bind(serv_sock, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) ==  -1)
		// 실패 시 에러 메시지 전달  
		error_handling("bind() error");
		
	// 클라이언트의 접속 요청을 기다리도록 설정 
	// listen() 함수의 매개변수는 sock(소켓 식별자/ 소켓 디스크립터), backlog(연결 요청을 대기시킬 공간)
	// 첫 번째 인자 : serv_sock(서버 소켓), 두 번째 인자 : 5 (큐의 크기를 5로 설정, 클라이언트의 연결 요청을 5개까지 대기시킴)
	// 반환 값 :  성공(0), 실패(-1) 
	if(listen(serv_sock, 5) == -1)
		// 실패 시 에러 메시지 전달  
		error_handling("listen() error"); 
	
	
	while(1)
	{
		// 클라이언트 주소 사이즈  
		clnt_adr_sz = sizeof(clnt_adr);
		
		// 요청을 기다리는 서버 소켓에 연결 요청이 왔을 때 연결 수락
		// 해당 소켓으로의 연결 요청이 없는 경우, 클라이언트가 연결을 요청할 때까지 소켓을 계속 감시하면서 대기 상태 유지  
		// accept() 함수의 매개변수는 sockfd(소켓 디스크립터), *serv_addr(서버 주소 정보 포인터), *addlen(구조체의 크기)
		// 첫 번째 인자 :  serv_sock(서버 소켓), 두 번째인자 : &clnt_adr(클라이언트 주소), 세 번 인자 : 클라이언트 주소의 길이 
		// 반환 값 : 성공(-1 외의 소켓 디스크립터), 실패(-1)  
		clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz); 
		
		// mutex 객체(mutx)는 mutex을 통해 임계구역 진입 요청
		// 진입한 쓰레드가 없다면(최근 상태가 unlocked) : lock 상태가 되고 임계구역 진입 
		// 진입한 쓰레드가 있다면(최근 상태가 locked) : 진입한 쓰레드가 unlock으로 임계구역을 빠져나오기 전까지 대기 
		pthread_mutex_lock(&mutx); 
		
		// 임계 구역  
		// 클라이언트 수와 파일 디스크립터 설정  
		clnt_socks[clnt_cnt ++] = clnt_sock ; 
		 
		// mutex 객체(mutx)는 mutex을 통해 임계구역을 빠져나온다. 최근 상태를 unlocked로 설정   
		pthread_mutex_unlock(&mutx); 
		
		// 쓰레드 생성 
		// pthread_create() 함수의 매개변수는 *thread(성공적으로 생성된 쓰레드의 식별자), attr(쓰레드 특성 지정)
		// , start_routine(분기시켜서 실행할 쓰레드 함수), arg(쓰레드 함수의 매개변수)
		// 첫 번째 인자 : &t_id(식별자), 두 번째 인자 : NULL(기본 특성 설정), 세 번째 인자 : handle_clnt(사용할 쓰레드 함수), 네 번째 인자 : &clnt_sock(handle_clnt의 인자)
		// 반환값 : 성공(0), 실패(0 이 아닌 값) 
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock); 
		
		// 쓰레드에 사용된 모든 자원 해제(쓰레드를 메인쓰레드에서 분리)
		// pthread_detach() 함수의 매개변수는 th(분리시킬 쓰레드 식별자) 
		// 반환값 : 성공(0), 실패(0이 아닌 값) 
		pthread_detach(t_id); 
		
		// 연결된 클라이언트 IP 출력
		// inet_ntoa(clnt_adr.sin_addr) : 네트워크 바이트 정렬 방식의 4바이트 정수의 IPv4 주소(clnt_adr.sin_addr)를 문자열 주소로 변환  
		printf("Connected client IP : %s \n", inet_ntoa(clnt_adr.sin_addr)); 
	}
	// 서버측 소켓 연결 종료  
	close(serv_sock); 
	return 0; 
}

// 쓰레드 함수 : handle_clnt, 매개변수를 소켓 형태로 받음  
void *handle_clnt(void *arg)
{
	// 매개변수로 받은 소켓  
	int clnt_sock = *((int*)arg); 
	int str_len = 0, i ; 
	char msg[BUF_SIZE]; 
	
	// 소켓에서 일정 크기만큼의 데이터를 읽어서 msg에 저장, 클라이언트에서 종료 시, 0 읽고 종료  
	// read() 함수의 매개변수는 fd(소켓 지정번호, 디스크립터), *buf(읽어들인 데이터가 저장될 버퍼 변수), count(읽어들일 데이터의 count 크기)
	// 첫 번 인자 : clnt_sock(클라이언트 소켓), 두 번째 인자 : msg(읽어들인 데이터를 저장할 버퍼), 세 번째 인자 : sizeof(msg)(버퍼의 크기)
	// 반환값 : 성공(읽어들인 데이터의 크기 : byte 단위), 실패(-1)  
	while((str_len = read(clnt_sock, msg, sizeof(msg))) != 0)
		// 모든 클라이언트에 데이터(메시지) 전송 : 인자(데이터(메시지), 데이터의 크기(길이)) 
		send_msg(msg, str_len); 
	
	// 연결이 끊어졌다면 도달  
	// mutex에 진입 요청
	// 상태가 locked인 경우 : unlocked가 될 때(실행중인 쓰레드가 끝날 때)까지 대기
	// 상태가 unlocked인 경우 : 진입  
	pthread_mutex_lock(&mutx);
	
	// 임계 구역 시작  
	// 연결 종료된 클라이언트 삭제  
	for(i = 0 ; i < clnt_cnt; i ++)
	{
		if(clnt_sock == clnt_socks[i])
		{
			while(i++ < clnt_cnt - 1)
				clnt_socks[i] = clnt_socks[i + 1]; 
			break ; 
		}
	}
	clnt_cnt -- ; 
	// 임계 구역 끝 
	
	// mutx(mutex 객체)의 상태를 unlocked로 
	pthread_mutex_unlock(&mutx); 
	
	// 클라이언트측 소켓 연결 종료  
	close(clnt_sock);
	return NULL ; 
}

// 모든 클라이언트에 데이터 전송 : 매개변수(데이터(메시지), 데이터의 크기) 
void send_msg(char *msg, int len)
{
	int i ; 
	
	// 진입 요청, locked가 아닌 경우( 실행 중인 쓰레드가 없는 경우) 진입 : 상태는 locked로 바뀜  
	pthread_mutex_lock(&mutx); 
	
	// 임계 구역
	// 모든 클라이언트에게 데이터(메시지) 전송  
	for(i = 0 ; i < clnt_cnt ; i ++)
		// 데이터 쓰기: 버퍼에 필요한 크기만큼fd에 작성 
		// write() 함수의 매개변수는 fd(디스크립터, 소켓 지정 번호), buf(데이터가 저장된 버퍼), count(보낼 데이터의 크기)
		// 첫 번째 인자 : clnt_socks[i](i번째 소켓), 두 번째 인자 : 보낼 데이터(메시지), 세 번째 인자 : 데이터(메시지)의 크기 
		write(clnt_socks[i], msg, len); 
	
	// 진입 해제, 상태를 unlocked로  
	pthread_mutex_unlock(&mutx); 
}

// 에러 표현 : 매개변수(에러 메시지) 
void error_handling(char *msg)
{
	fputs(msg, stderr); 
	fputs('\n', stderr); 
	exit(1); 
}
