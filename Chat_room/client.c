#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

#define SERVER_PORT 7888 //服务器的端口号
#define SERVER_IP "192.168.137.118" //服务器的 IP 地址
#define BUFFSIZE 1024

int sock_fd;
void snd();

int main()
{
    pthread_t thread;
    struct sockaddr_in server_addr = {0};
    char ip_str[20] = {0};
    char sendbuf[BUFFSIZE];
    int ret;

    // 创建客户端套接字
    sock_fd = socket(AF_INET,SOCK_STREAM,0);
    if(0>sock_fd){
        perror("socket errror");
        exit(-1);
    }

    // 初始化填充服务端地址结构
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);//将点分十进制表示的字符串形式转换成二进制 Ipv4 或 Ipv6 地址

    ret = connect(sock_fd,(struct sockaddr *)&server_addr, sizeof(server_addr));
    if(0>ret){
        perror("connect error");
        close(sock_fd);
        exit(-1);
    }
    printf("connected\n");
    /* === 从此处开始 程序分做两个线程 === */
    // 创建发送消息的线程，调用发送消息的函数snd
    pthread_create(&thread, NULL, (void *)(&snd), NULL);    // pthread_create
    // 接收消息的线程
    while(1)
    {
            int len;
            if((len=read(sock_fd, sendbuf, BUFFSIZE)) > 0)       // read 读取通信套接字
            {
                sendbuf[len] = '\0';        // 添加结束符，避免显示缓冲区中残留的内容
                printf("\n%s", sendbuf);
                fflush(stdout);         // fflush 冲洗标准输出，确保内容及时显示
            }
    }
    return 0;
}
//发送消息的函数（线程）
void snd()
{
    char name[32], buf[BUFFSIZE];

    // //线程分离，当线程终止时会自动回收线程资源
    // if(pthread_detach(pthread_self())){
    //     perror("pthread_detach error");
    //     return -1;
    // }
    fgets(name, 32, stdin); // fgets 会读取输入字符串后的换行符
    write(sock_fd, name, strlen(name));      // write 写入通信套接字
    while(1)
    {
        fgets(buf, BUFFSIZE, stdin);
        write(sock_fd, buf, strlen(buf));
        if(strcmp(buf, "bye\n") == 0)   // 注意此处的\n
            exit(0);
    }

}
