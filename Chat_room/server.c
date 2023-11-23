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

#define SERVER_PORT 7888 //端口号不能发生冲突,不常用的端口号通常大于 5000
#define MAXMEM 10           // 等待连接队列能够达到的最大值 用于listen()函数
#define BUFFSIZE 1024
 
//#define DEBUG_PRINT 1         // 宏定义 调试开关
#ifdef DEBUG_PRINT
#define DEBUG(format, ...) printf("FILE: "__FILE__", LINE: %d: "format"\n", __LINE__, ##__VA_ARGS__)
#else
#define DEBUG(format, ...)
#endif

void quit();
void rcv_snd(int p);

int sock_fd,connect_fd[MAXMEM];    // 创建服务器socket套接字、以及连接后创建的连接socket套接字
//pthread_t thread[MAXMEM];

int main(int argc, char** argv)
{
                     
    struct sockaddr_in server_addr = {0};       // 服务器端socket结构体
    struct sockaddr_in  client_addr = {0};      // 客户端socket结构体
    char ip_str[20] = {0};                      // IP地址
    int addrlen = sizeof(client_addr);          // 客户端socket结构体大小
    char recvbuf[512];                          // 接受缓冲区
    pthread_t thread;
    time_t time_ticks;
    int ret,i;

    printf("running...\n(Prompt: enter command ""quit"" to exit server)\n");
    
    // socket 创建服务器端的监听套接字
    sock_fd = socket(AF_INET,SOCK_STREAM,0);    // AF_INET 代表通信协议是TCP/IP
    if(0>sock_fd){
        perror("socket errror");
        exit(-1);
    }

    // 初始化填充服务端地址结构
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    //htonl、htons 宏定义，避免大小端模式
    server_addr.sin_port = htons(SERVER_PORT);

    // bind 将套接字与填充好的地址结构进行绑定
    if (0 > bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))){
        perror("bind error");
        close(sock_fd);
        exit(-1);
    }

    // listen 将主动连接套接字变为被动倾听套接字 让服务器进程进入监听状态，等待客户端的连接请球
    ret = listen(sock_fd,MAXMEM);
    if (0 > ret) {
        perror("listen error");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    /* === 创建一个线程，对服务器程序进行管理，调用quit函数 === */
    pthread_create(&thread, NULL, (void *)(quit), NULL);
    //
    // 将套接字描述符数组初始化为-1，表示空闲
    for(i=0; i<MAXMEM; i++)
        connect_fd[i] = -1;

    while(1){
        int len;// = sizeof(cli_addr);
        for(i=0; i<MAXMEM; i++)
        {
            if(connect_fd[i] == -1)
                break;
        }

        //accept从listen 接受的连接队列中取得一个连接(会陷入阻塞)
        connect_fd[i] = accept(sock_fd,(struct sockaddr *)&client_addr, &addrlen);
        if(connect_fd[i] < 0)
        {
            perror("fail to accept");
            exit(-1);
        }
        time_ticks = time(NULL);
        printf("有客户端接入...\n");
        inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip_str,sizeof(ip_str));
        printf("客户端主机的 IP 地址: %s\n", ip_str);
        printf("客户端进程的端口号: %d\n", client_addr.sin_port);

        
        /* === 针对当前套接字创建一个线程，对当前套接字的消息进行处理 === */
        //每次连接一个新用户，都会创建一个对此连接的消息处理线程。
        pthread_create(malloc(sizeof(pthread_t)), NULL, (void *)(&rcv_snd), (void *)i);
    }
    return 0;
}

/*
*函数名称：退出函数
*函数功能：当输入quit时，服务器程序终止。
*/
void quit()
{
    char msg[10];
    while(1)
    {
        scanf("%s", msg);       // scanf 不同于fgets, 它不会读入最后输入的换行符
        if(strcmp(msg, "quit") == 0)
        {
            printf("Byebye... \n");
            close(sock_fd);
            exit(0);        //通过此函数退出函数，会导致整个进程停止。
        }
    }
}

void rcv_snd(int n)
{
    int len, i;
    char name[32], mytime[32], buf[BUFFSIZE];
    time_t time_ticks;
    int ret;

    //线程分离，当线程终止时会自动回收线程资源
    if(pthread_detach(pthread_self())){
        perror("pthread_detach error");
        return 0;
    }

    write(connect_fd[n], "your name: ", strlen("your name: "));     //把 “yourname: ”发送给客户端
    len = read(connect_fd[n], name, 32);
    if(len > 0)
        name[len-1] = '\0';     // 去除换行符
    
    strcpy(buf, name);
    strcat(buf, "\tjoin in\n\0"); // 把当前用户的加入 告知所有用户

    for(i=0; i<MAXMEM; i++)
    {
        if(connect_fd[i] != -1)
            write(connect_fd[i],buf,sizeof(buf));
    }   

    while(1)
    {
        char temp [BUFFSIZE];
        if(len = read(connect_fd[i],temp,BUFFSIZE)>0)
        {
            temp[len-1] = '\0';
            if (strcmp(temp,"bey")==0)
            {
                close(connect_fd[n]);
                connect_fd[n] = -1;
                pthread_exit(&ret);             //ret表示整个线程的退出码
            }
            time_ticks = time(NULL);
            sprintf(mytime,"%.24s\r\n",ctime(&time_ticks));
            strcpy(buf,name);
            strcat(buf, "\t");
            strcat(buf, mytime);
            strcat(buf, "\r\t");
            strcat(buf, temp);
            strcat(buf, "\n");

            //发送给所有连接服务器的用户
            for(i=0; i<MAXMEM; i++)
            {
                if(connect_fd[i] != -1)
                    write(connect_fd[i], buf, strlen(buf));
            }
        }
    }

}
    // connect_fd = accept(sock_fd,(struct sockaddr *)&client_addr, &addrlen);
    // if (0 > connect_fd){
    //     perror("accept error");
    //     close(sock_fd);
    //     exit(-1);
    // }

    // printf("有客户端接入...\n");
    // inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, ip_str,sizeof(ip_str));
    // printf("客户端主机的 IP 地址: %s\n", ip_str);
    // printf("客户端进程的端口号: %d\n", client_addr.sin_port);

    // /* 接收客户端发送过来的数据 */
    // for ( ; ; ) {
    //         // 接收缓冲区清零
    //         memset(recvbuf, 0x0, sizeof(recvbuf));
    //         // 读数据
    //         ret = recv(connect_fd, recvbuf, sizeof(recvbuf), 0);
    //         if(0 >= ret) {
    //             perror("recv error");
    //             close(connect_fd);
    //             break;
    //         }
    //         // 将读取到的数据以字符串形式打印出来
    //         printf("from client: %s\n", recvbuf);
    //         // 如果读取到"exit"则关闭套接字退出程序
    //         if (0 == strncmp("exit", recvbuf, 4)) {
    //             printf("server exit...\n");
    //             close(connect_fd);
    //             break;
    //         }
    // }

    /* 关闭套接字 */
