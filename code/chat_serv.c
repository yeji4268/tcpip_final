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

void *handle_clnt(void *arg); 
void send_msg(char *msg, int len); 
void error_handling(char *msg); 

# Ŭ���̾�Ʈ ���� �ʱⰪ : 0  
int clnt_cnt = 0 ; 

# Ŭ���̾�Ʈ ����  
int clnt_socks[MAX_CLNT]; 

# mutex ��ü  
pthread_mutex_t mutx ; 

int main(int argc, char &argv[])
{
	# ���� �� ����, Ŭ���̾�Ʈ �� ���� 
	int serv_sock, clnt_sock; 
	
	# ���� �� �ּ�, Ŭ���̾�Ʈ �� �ּ�  
	struct sockaddr_in serv_adr, clnt_adr ; 
	
	# Ŭ���̾�Ʈ �ּ� ������  
	int clnt_adr_sz ; 
	
	# ������ �ĺ���  
	pthread_t t_id ;
	
	# main �Լ��� ���޵Ǵ� ������ ����(argc)�� 2����  �ƴ� ��� 
	# ��, ���� �� ������(���� ���� �̸�)�� ��Ʈ ��ȣ�� �Էµ��� ���� ��� �ùٸ� ������ �ȳ��Ѵ�.  
	if(argc != 2){
		printf("Usage : %s <port> \n", argv[0]); 
		
		# ���α׷� ����  
		exit(1); 
	}
	
	# mutext ��ü �ʱ�ȭ 
	# pthread_mutex_init() �Լ��� ù��° �Ű������� �ʱ�ȭ ��ų mutex ��ü, �ι�° �Ű������� ���ؽ��� Ư¡�̴�. 
	# ù ��° ���� : mutx, �� ��° ���� : NULL (�⺻ ��)
	# ��ȯ �� : ����(0), ����(-1)   
	pthread_mutex_init(&mutx, NULL); 
	
	# ���� �� ���� ����
	# socket() �Լ��� �Ű������� domain(���ͳ� ���/ ���� �ý��� �� ��� ����), type (�������� ���� ����)
	# , protocol(�������� ����)�� ����Ѵ�. 
	# ù ��° ���� : PF_INET(IPv4 ���ͳ� �������� ���), �� ��° ���� : SOCK_STREAM(TCP/IP �������� ���)
	# �� ��° ���� : 0 (�ڵ����� type���� ����)
	# ��ȯ �� : ����(0 �̻��� ��), ����(-1)  
	serv_sock = socket(PF_INET, SOCK_STREAM, 0); 
	
	# �޸� ���� : ������ ���� �ʱ�ȭ ���� (������ �� ����) 
	# memset() �Լ��� �Ű������� *ptr(�����ϰ��� �ϴ� �޸��� ���� �ּ�), value(�޸𸮿� �����ϰ��� �ϴ� ��)
	# , num(����, ���� sizeof(������)�� ���·� �ۼ�)�� ����Ѵ�. 
	# ù ��° ���� : &serv_sock(���� �� ���Ͽ���), �� ��° ���� : 0 (0����), �� ��° ���� : size(serv_adr) (���� �� �ּ��� ���̸�ŭ ����)
	# ��ȯ �� : ����(ptr), ����(NULL) 
	memset(&serv_adr, 0, sizeof(serv_adr)); 
	
	# ���� �ּ� ü�� : AF_INET(IPv4) 
	serv_adr.sin_family = AF_INET ; 
	
	# ���� IP �ּ� ���� : htonl(INADDR_ANY)
	# INADDR_ANY : IP �ּҸ� �ڵ����� ã�Ƽ� ���� (�ش� ��Ʈ�� �������� �ϴ� ��� ���� ��û�� ���� �ش� ���α׷����� ó��) 
	# htonl : ȣ��Ʈ ����Ʈ ������ ������ �����͸� ��Ʈ��ũ ����Ʈ ������ ��ȯ(long integer�� ��Ʈ��ũ ����Ʈ ������)  
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY); 
	
	# ��Ʈ ���� : htons(atoi(argv[1]))
	# argv[1] : ���� �� �Էµ� ���� �� �� ��° ��(��Ʈ��ȣ) 
	# atoi : ���ڿ� ���·� �Էµ� ��Ʈ ��ȣ�� ������ ��ȯ  
	# htons :  ȣ��Ʈ ����Ʈ ������ ������ �����͸� ��Ʈ��ũ ����Ʈ ������ ��ȯ(short integer�� ��Ʈ��ũ ����Ʈ ������) 
	serv_adr.sin_port = htons(atoi(argv[1])); 
	
	# ���ϰ� ������ ������ ���ε� 
	# bind() �Լ��� sockfd(���� ��ũ����), *myaddr(������ IP �ּ�), addrlen(�ּ��� ����)�� �Ű������� ����Ѵ�. 
	# ù ��° ���� : serv_sock(���� ����), �� ��° ���� : (struct sockaddr *)&serv_adr(������ IP �ּ�), �� ��° ���� : sizeof(serv_adr)(���� �ּ��� ����)
	# ��ȯ �� : ����(0), ����(-1) 
	if(bind(serv_sock, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) ==  -1)
		# ���� �� ���� �޽��� ����  
		error_handling("bind() error");
		
	# Ŭ���̾�Ʈ�� ���� ��û�� ��ٸ����� ���� 
	# listen() �Լ��� �Ű������� sock(���� �ĺ���/ ���� ��ũ����), backlog(���� ��û�� ����ų ����)
	# ù ��° ���� : serv_sock(���� ����), �� ��° ���� : 5 (ť�� ũ�⸦ 5�� ����, Ŭ���̾�Ʈ�� ���� ��û�� 5������ ����Ŵ)
	# ��ȯ �� :  ����(0), ����(-1) 
	if(listen(serv_sock, 5) == -1)
		# ���� �� ���� �޽��� ����  
		error_handling("listen() error"); 
	
	
	while(1)
	{
		# Ŭ���̾�Ʈ �ּ� ������  
		clnt_adr_sz = sizeof(clnt_adr);
		
		# ��û�� ��ٸ��� ���� ���Ͽ� ���� ��û�� ���� �� ���� ����
		# �ش� ���������� ���� ��û�� ���� ���, Ŭ���̾�Ʈ�� ������ ��û�� ������ ������ ��� �����ϸ鼭 ��� ���� ����  
		# accept() �Լ��� �Ű������� sockfd(���� ��ũ����), *serv_addr(���� �ּ� ���� ������), *addlen(����ü�� ũ��)
		# ù ��° ���� :  serv_sock(���� ����), �� ��°���� : &clnt_adr(Ŭ���̾�Ʈ �ּ�), �� ���� ���� : Ŭ���̾�Ʈ �ּ��� ���� 
		# ��ȯ �� : ����(-1 ���� ���� ��ũ����), ����(-1)  
		clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz); 
		
		# mutex ��ü(mutx)�� mutex�� ���� �Ӱ豸�� ���� ��û
		# ������ �����尡 ���ٸ�(�ֱ� ���°� unlocked) : lock ���°� �ǰ� �Ӱ豸�� ���� 
		# ������ �����尡 �ִٸ�(�ֱ� ���°� locked) : ������ �����尡 unlock���� �Ӱ豸���� ���������� ������ ��� 
		pthread_mutex_lock(&mutx); 
		
		# �Ӱ� ����  
		# Ŭ���̾�Ʈ ���� ���� ��ũ���� ����  
		clnt_socks[clnt_cnt ++] = clnt_sock ; 
		 
		# mutex ��ü(mutx)�� mutex�� ���� �Ӱ豸���� �������´�. �ֱ� ���¸� unlocked�� ����   
		pthread_mutex_unlock(&mutx); 
		
		# ������ ���� 
		# pthread_create() �Լ��� �Ű������� *thread(���������� ������ �������� �ĺ���), attr(������ Ư�� ����)
		# , start_routine(�б���Ѽ� ������ ������ �Լ�), arg(������ �Լ��� �Ű�����)
		# ù ��° ���� : &t_id(�ĺ���), �� ��° ���� : NULL(�⺻ Ư�� ����), �� ��° ���� : handle_clnt(����� ������ �Լ�), �� ��° ���� : &clnt_sock(handle_clnt�� ����)
		# ��ȯ�� : ����(0), ����(0 �� �ƴ� ��) 
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock); 
		
		# �����忡 ���� ��� �ڿ� ����(�����带 ���ξ����忡�� �и�)
		# pthread_detach() �Լ��� �Ű������� th(�и���ų ������ �ĺ���) 
		# ��ȯ�� : ����(0), ����(0�� �ƴ� ��) 
		pthread_detach(t_id); 
		
		# ����� Ŭ���̾�Ʈ IP ���
		# inet_ntoa(clnt_adr.sin_addr) : ��Ʈ��ũ ����Ʈ ���� ����� 4����Ʈ ������ IPv4 �ּ�(clnt_adr.sin_addr)�� ���ڿ� �ּҷ� ��ȯ  
		printf("Connected client IP : %s \n", inet_ntoa(clnt_adr.sin_addr)); 
	}
	# ������ ���� ���� ����  
	close(serv_sock); 
	return 0; 
}

# ������ �Լ� : handle_clnt, �Ű������� ���� ���·� ����  
void *handle_clnt(void *arg)
{
	# �Ű������� ���� ����  
	int clnt_sock = *((int*)arg); 
	int str_len = 0, i ; 
	char msg[BUF_SIZE]; 
	
	# ���Ͽ��� ���� ũ�⸸ŭ�� �����͸� �о msg�� ����, Ŭ���̾�Ʈ���� ���� ��, 0 �а� ����  
	# read() �Լ��� �Ű������� fd(���� ������ȣ, ��ũ����), *buf(�о���� �����Ͱ� ����� ���� ����), count(�о���� �������� count ũ��)
	# ù ���� ���� : clnt_sock(Ŭ���̾�Ʈ ����), �� ��° ���� : msg(�о���� �����͸� ������ ����), �� ��° ���� : sizeof(msg)(������ ũ��)
	# ��ȯ�� : ����(�о���� �������� ũ�� : byte ����), ����(-1)  
	while((str_len = read(clnt_sock, msg, sizeof(msg))) != 0)
		# ��� Ŭ���̾�Ʈ�� ������(�޽���) ���� : ����(������(�޽���), �������� ũ��(����)) 
		send_msg(msg, str_len); 
	
	# ������ �������ٸ� ����  
	# mutex�� ���� ��û
	# ���°� locked�� ��� : unlocked�� �� ��(�������� �����尡 ���� ��)���� ���
	# ���°� unlocked�� ��� : ����  
	pthread_mutex_lock(&mutx);
	
	# �Ӱ� ���� ����  
	# ���� ����� Ŭ���̾�Ʈ ����  
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
	# �Ӱ� ���� �� 
	
	# mutx(mutex ��ü)�� ���¸� unlocked�� 
	pthread_mutex_unlock(&mutx); 
	
	# Ŭ���̾�Ʈ�� ���� ���� ����  
	close(clnt_sock);
	return NULL ; 
}

# ��� Ŭ���̾�Ʈ�� ������ ���� : �Ű�����(������(�޽���), �������� ũ��) 
void send_msg(char *msg, int len)
{
	int i ; 
	
	# ���� ��û, locked�� �ƴ� ���( ���� ���� �����尡 ���� ���) ���� : ���´� locked�� �ٲ�  
	pthread_mutex_lock(&mutx); 
	
	# �Ӱ� ����
	# ��� Ŭ���̾�Ʈ���� ������(�޽���) ����  
	for(i = 0 ; i < clnt_cnt ; i ++)
		# ������ ����: ���ۿ� �ʿ��� ũ�⸸ŭfd�� �ۼ� 
		# write() �Լ��� �Ű������� fd(��ũ����, ���� ���� ��ȣ), buf(�����Ͱ� ����� ����), count(���� �������� ũ��)
		# ù ��° ���� : clnt_socks[i](i��° ����), �� ��° ���� : ���� ������(�޽���), �� ��° ���� : ������(�޽���)�� ũ�� 
		write(clnt_socks[i], msg, len); 
	
	# ���� ����, ���¸� unlocked��  
	pthread_mutex_unlock(&mutx); 
}

# ���� ǥ�� : �Ű�����(���� �޽���) 
void error_handling(char *msg)
{
	fputs(msg, stderr); 
	fputs('\n', stderr); 
	exit(1); 
}
