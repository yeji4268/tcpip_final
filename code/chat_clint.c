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

# �̸� ����, �ʱ�ȭ  
char name[NAME_SIZE] = "[DEFALUT]"; 
char msg[BUF_SIZE]; 

int main(int argc, char *argv[])
{
	# ����  
	int sock ;  
	# �ּ�  
	struct sockaddr_in serv_addr ;
	# ���� ������, ���� ������(�ĺ���)  
	pthread_t snd_thread, rcv_thread ;
    # ���ϰ�  
	void *thread_return ; 
	
	# main �Լ��� ���޵Ǵ� ������ ����(argc)�� 4���� �ƴ� ��� 
	# ���� �� ������(���� ���� �̸�), IP, ��Ʈ ��ȣ, �̸��� �Էµ��� ���� ��� �ùٸ� ������ �ȳ��Ѵ�.  
	if(argc != 4){
		printf("Usage : %s <IP> <port> <name> \n", argv[0])
		
		# ���α׷� ���� 
		exit(1); 
	}
	
	# �̸� �Է�  
	sprintf(name, "[%s]", argv[3]);
	
	# ���� ���� : �� ������, sock�� Ŭ���̾�Ʈ ����  
	# socket() �Լ��� �Ű������� domain(���ͳ� ���/ ���� �ý��� �� ��� ����), type(�������� ���� ����)
	# , protocol(�������� ����)�� ����Ѵ�. 
	# ù ��° ���� : PF_INET(IPv4 ���ͳ� �������� ���), �� ��° ���� : SOCK_STREAM(TCP/IP �������� ���)
	# �� ��° ���� : 0(�ڵ����� type���� ����)
	# ��ȯ �� : ����(0 �̻��� ��), ����(-1) 
	sock = socket(PF_INET, SOCK_STREAM, 0);
	
	# �޸� ���� : ������ ���� �ʱ�ȭ ����(������ �� ����) 
	# memset() �Լ��� �Ű������� *ptr(�����ϰ��� �ϴ� �޸��� ���� �ּ�), value(�޸𸮿� �����ϰ��� �ϴ� ��)
	# , num(����, ���� sizeof(������)�� ���·� �ۼ�)�� ����Ѵ�. 
	# ù ��° ���� : &serv_addr(���� �ּҿ���), �� ��° ���� : 0 (0����), �� ��° ���� : size(serv_adr) (���� �� �ּ��� ���̸�ŭ ����)
	# ��ȯ �� : ����(ptr), ����(NULL) 
	memset(&serv_addr, 0, sizeof(serv_addr)); 
	
	# ���� �ּ� ü�� : AF_INET(IPv4) 
	serv_addr.sin_family = AF_INET ;
	
	# ���� IP �ּ� ���� : argv[1](���� �� ���ڷ� ���� IP �ּ�, ���ڿ� ����)�� ��Ʈ��ũ ����Ʈ ���� ������� ��ȯ   
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]); 
	
	# ��Ʈ ���� : htons(atoi(argv[2]))
	# argv[2] : ���� �� �Էµ� ���� �� �� ��° ��(��Ʈ ��ȣ)
	# atoi : ���ڿ� ���·� �Էµ� ��Ʈ ��ȣ�� ������ ��ȯ 
	# htons : ȣ��Ʈ ����Ʈ ������ ������ �����͸� ��Ʈ��ũ ����Ʈ ������ ��ȯ(short integer�� ��Ʈ��ũ ����Ʈ ������) 
	serv_addr.sin_port = htons(atoi(argv[2])); 
	
	# ������ ���� ��û 
	# connect() �Լ��� �Ű������� sockfd(���� ��ũ����), *serv_addr(�����ּ� ������ ���� ������), addrlen(�����Ͱ� ����Ű�� ����ü�� ũ��)�� ������.
	# ù ��° ���� : sock(Ŭ���̾�Ʈ ����), �� ��° ���� : &serv_addr(������ �ּ�), �� ��° ����: sizeof(serv_addr) (���� �ּ� ����)
	# ��ȯ �� : ����(-1 �̿��� ��), ����(-1) 
	if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
		# ���� �� ���� �޽��� ����  
		error_handling("connect() error"); 
		
	# ������ ���� : ���� ������  
	# pthread_create() �Լ��� �Ű������� *thread(���������� ������ �������� �ĺ���), attr(������ Ư��)
	# , start_routine(�б���Ѽ� ������ ������ �Լ�), arg(������ �Լ��� �Ű�����)
	# ù ��° ���� : &snd_thread(���� ������), �� ��° ���� : NULL(�⺻ Ư�� ���), �� ��° ���� : send_msg(����� ������ �Լ�), �� ���� ���� : &sock(send_msg�� ����)
	# ��ȯ �� : ����(0), ����(0�� �ƴ� ��) 
	pthread_create(&snd_thread, NULL, send_msg, (void *)&sock);
	
	# ������ ���� : ���� ������  
	pthread_create(&rcv_thread, NULL, recv_msg, (void *)&sock);
	
	#  ���� ������ ���� ��� : �����尡 ����� �̷� ���� ����, join�� ������(����� ������)�� ��� �ڿ� �ݳ�   
	# pthread_join() �Լ��� �Ű������� th(��ٸ� �������� �ĺ���), thread_return(�������� ���ϰ�)
	# ù ��° ���� : snd_thread(���� ������), �� ��° ���� : &thread_return(���ϰ�) 
	# ��ȯ�� : ����(������ �ĺ���ȣ�� �ĺ��ڿ� ���� ��, 0 ��ȯ), ����(���� �ڵ� ��)  
	pthread_join(snd_thread, &thread_return); 
	
	# ���� ������ ���� ���  
	pthread_join(rcv_thread, &thread_return); 
	
	# Ŭ���̾�Ʈ ���� ���� ����  
	close(sock); 
	return 0 ; 
}

# ������ ������(�޽���) ���� : �Ű�����(������(�޽���), �������� ũ��) 
void *send_msg(void *arg)
{
	# �Ű������� ���� Ŭ���̾�Ʈ ����  
	int sock = *((int *)arg);
	
	# �޽��� �迭 ����  
	char name_msg[NAME_SIZE + BUF_SIZE]; 
	
	# ���� ����  
	while(1)
	{  
	    # �޽��� �Է�  
		fgets(msg, BUF_SIZE, stdin); 
		
		# q �� Q �Է� ��, ���� ���� ����  
		if(!strcmp(msg, "q\n") || !strcmp(msg, "Q\n"))
		{
			close(sock); 
			
			#���α׷� ���� 
			exit(0);
		}
		# �޽��� �迭�� �Է�  
		sprintf(name_msg, "%s %s", name, msg);
		
		# ������ ���� : �޽��� ���� 
		# write() �Լ��� �Ű������� fd(��ũ����, ���� ���� ��ȣ), buf(�����Ͱ� ����� ����), count(���� �������� ũ��)
		# ù ��° ���� : sock(Ŭ���̾�Ʈ ����), �� ���� ���� : name_msg(���� ������),  �� ��° ���� : strlen(name_msg)(�������� ũ��) 
		write(sock, name_msg, strlen(name_msg)); 
	}
	return NULL ; 
}

# �޽��� ����  
void *recv_msg(void *arg)
{
	# �Ű������� ���� Ŭ���̾�Ʈ ����  
	int sock = *((int *)arg); 
	char name_msg[NAME_SIZE + BUF_SIZE]; 
	int str_len ; 
	
	# ���� ����  
	while(1)
	{
		# ���Ͽ��� ���� ũ�⸸ŭ�� �����͸� �о ����, ���� �� 0 �а� ���� 
		# read() �Լ��� �Ű������� fd(���� ������ȣ, ��ũ����), *buf(�о���� �����Ͱ� ����� ���� ����), count(�о���� �������� count ũ��)
		# ù ��°  ���� : sock(Ŭ���̾�Ʈ ����), �� ��° ���� : name_msg(������ ����), �� ��° ���� : NAME_SIZE + BUF_SIZE -1
		# ��ȯ�� : ����(�о���� �������� ũ�� : byte ����), ����(-1) 
		str_len = read(sock, name_msg, NAME_SIZE + BUF_SIZE -1); 
		
		# read ���� ��  
		if(str_len == -1)
			return (void *) - 1; 
		name_msg[str_len] = 0 ;
		
		# �ֿܼ� ���  
		fputs(name_msg, stdout); 
	}
	return NULL ; 
}

# ���� ǥ��  
void error_handling(char *msg)
{
	fputs(msg, stderr); 
	fputc('\n', stderr); 
	exit(1); 
}
