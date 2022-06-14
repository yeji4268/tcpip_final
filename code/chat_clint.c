#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

void *send_msg(void *arg); 
void *recv_msg(void *arg); 
void error_handling(char *msg); 

# 이름 선언, 초기화  
char name[NAME_SIZE] = "[DEFALUT]"; 
char msg[BUF_SIZE]; 

int main(int argc, char *argv[])
{
	# 소켓  
	int sock ;  
	# 주소  
	struct sockaddr_in serv_addr ;
	# 전송 쓰레드, 수신 쓰레드(식별자)  
	pthread_t snd_thread, rcv_thread ;
    # 리턴값  
	void *thread_return ; 
	
	# main 함수에 전달되는 정보의 개수(argc)가 4개가 아닌 경우 
	# 실행 시 실행경로(실행 파일 이름), IP, 포트 번호, 이름이 입력되지 않은 경우 올바른 사용법을 안내한다.  
	if(argc != 4){
		printf("Usage : %s <IP> <port> <name> \n", argv[0])
		
		# 프로그램 종료 
		exit(1); 
	}
	
	# 이름 입력  
	sprintf(name, "[%s]", argv[3]);
	
	# 소켓 생성 : 이 때부터, sock은 클라이언트 소켓  
	# socket() 함수는 매개변수로 domain(인터넷 통신/ 같은 시스템 내 통신 설정), type(데이터의 전송 형태)
	# , protocol(프로토콜 지정)을 사용한다. 
	# 첫 번째 인자 : PF_INET(IPv4 인터넷 프로토콜 사용), 두 번째 인자 : SOCK_STREAM(TCP/IP 프로토콜 사용)
	# 세 번째 인자 : 0(자동으로 type에서 지정)
	# 반환 값 : 성공(0 이상의 값), 실패(-1) 
	sock = socket(PF_INET, SOCK_STREAM, 0);
	
	# 메모리 세팅 : 생성된 소켓 초기화 목적(쓰레기 값 제거) 
	# memset() 함수는 매개변수로 *ptr(세팅하고자 하는 메모리의 시작 주소), value(메모리에 세팅하고자 하는 값)
	# , num(길이, 보통 sizeof(데이터)의 형태로 작성)을 사용한다. 
	# 첫 번째 인자 : &serv_addr(서버 주소에서), 두 번째 인자 : 0 (0으로), 세 번째 인자 : size(serv_adr) (서버 측 주소의 길이만큼 설정)
	# 반환 값 : 성공(ptr), 실패(NULL) 
	memset(&serv_addr, 0, sizeof(serv_addr)); 
	
	# 서버 주소 체계 : AF_INET(IPv4) 
	serv_addr.sin_family = AF_INET ;
	
	# 서버 IP 주소 설정 : argv[1](실행 시 인자로 받은 IP 주소, 문자열 형태)를 네트워크 바이트 정렬 방식으로 변환   
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]); 
	
	# 포트 설정 : htons(atoi(argv[2]))
	# argv[2] : 실행 시 입력된 인자 중 세 번째 값(포트 번호)
	# atoi : 문자열 형태로 입력된 포트 번호를 정수로 변환 
	# htons : 호스트 바이트 순서를 따른느 데이터를 네트워크 바이트 순서로 변환(short integer를 네트워크 바이트 순서로) 
	serv_addr.sin_port = htons(atoi(argv[2])); 
	
	# 서버로 접속 요청 
	# connect() 함수는 매개변수로 sockfd(소켓 디스크립터), *serv_addr(서버주소 정보에 대한 포인터), addrlen(포인터가 가르키는 구조체의 크기)를 가진다.
	# 첫 번째 인자 : sock(클라이언트 소켓), 두 번째 인자 : &serv_addr(서버측 주소), 세 번째 인자: sizeof(serv_addr) (서버 주소 길이)
	# 반환 값 : 성공(-1 이외의 값), 실패(-1) 
	if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
		# 실패 시 에러 메시지 전달  
		error_handling("connect() error"); 
		
	# 쓰레드 생성 : 전송 쓰레드  
	# pthread_create() 함수의 매개변수는 *thread(성공적으로 생성된 쓰레드의 식별자), attr(쓰레드 특성)
	# , start_routine(분기시켜서 실행할 쓰레드 함수), arg(쓰레드 함수의 매개변수)
	# 첫 번째 인자 : &snd_thread(전송 쓰레드), 두 번째 인자 : NULL(기본 특성 사용), 세 번째 인자 : send_msg(사용할 쓰레드 함수), 네 번째 인자 : &sock(send_msg의 인자)
	# 반환 값 : 성공(0), 실패(0이 아닌 값) 
	pthread_create(&snd_thread, NULL, send_msg, (void *)&sock);
	
	# 쓰레드 생성 : 수신 쓰레드  
	pthread_create(&rcv_thread, NULL, recv_msg, (void *)&sock);
	
	#  전송 쓰레드 종료 대기 : 쓰레드가 종료된 이루 다음 진행, join된 쓰레드(종료된 쓰레드)는 모든 자원 반남   
	# pthread_join() 함수는 매개변수로 th(기다릴 쓰레드의 식별자), thread_return(쓰레드의 리턴값)
	# 첫 번째 인자 : snd_thread(전송 쓰레드), 두 번째 인자 : &thread_return(리턴값) 
	# 반환값 : 성공(쓰레드 식별번호를 식별자에 저장 후, 0 반환), 실패(에러 코드 값)  
	pthread_join(snd_thread, &thread_return); 
	
	# 수신 쓰레드 종료 대기  
	pthread_join(rcv_thread, &thread_return); 
	
	# 클라이언트 소켓 연결 종료  
	close(sock); 
	return 0 ; 
}

# 서버에 데이터(메시지) 전송 : 매개변수(데이터(메시지), 데이터의 크기) 
void *send_msg(void *arg)
{
	# 매개변수로 받은 클라이언트 소켓  
	int sock = *((int *)arg);
	
	# 메시지 배열 선언  
	char name_msg[NAME_SIZE + BUF_SIZE]; 
	
	# 무한 루프  
	while(1)
	{  
	    # 메시지 입력  
		fgets(msg, BUF_SIZE, stdin); 
		
		# q 나 Q 입력 시, 소켓 연결 종료  
		if(!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
		{
			close(sock); 
			
			#프로그램 종료 
			exit(0);
		}
		# 메시지 배열에 입력  
		sprintf(name_msg, "%s %s", name, msg);
		
		# 데이터 쓰기 : 메시지 전송 
		# write() 함수의 매개변수는 fd(디스크립터, 소켓 지정 번호), buf(데이터가 저장된 버퍼), count(보낼 데이터의 크기)
		# 첫 번째 인자 : sock(클라이언트 소켓), 두 번째 인자 : name_msg(보낼 데이터),  세 번째 인자 : strlen(name_msg)(데이터의 크기) 
		write(sock, name_msg, strlen(name_msg)); 
	}
	return NULL ; 
}

# 메시지 수신  
void *recv_msg(void *arg)
{
	# 매개변수로 받은 클라이언트 소켓  
	int sock = *((int *)arg); 
	char name_msg[NAME_SIZE + BUF_SIZE]; 
	int str_len ; 
	
	# 무한 루프  
	while(1)
	{
		# 소켓에서 일정 크기만큼의 데이터를 읽어서 저장, 종료 시 0 읽고 종료 
		# read() 함수의 매개변수는 fd(소켓 지정번호, 디스크립터), *buf(읽어들인 데이터가 저장될 버퍼 변수), count(읽어들일 데이터의 count 크기)
		# 첫 번째  인자 : sock(클라이언트 소켓), 두 번째 인자 : name_msg(저장할 버퍼), 세 번째 인자 : NAME_SIZE + BUF_SIZE -1
		# 반환값 : 성공(읽어들인 데이터의 크기 : byte 단위), 실패(-1) 
		str_len = read(sock, name_msg, NAME_SIZE + BUF_SIZE -1); 
		
		# read 실패 시  
		if(str_len == -1)
			return (void *) - 1; 
		name_msg[str_len] = 0 ;
		
		# 콘솔에 출력  
		fputs(name_msg, stdout); 
	}
	return NULL ; 
}

# 에러 표현  
void error_handling(char *msg)
{
	fputs(msg, stderr); 
	fputc('\n', stderr); 
	exit(1); 
}
