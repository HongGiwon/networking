## Download on Demand system
다수의 client로부터 자료요청을 받아, 해당 자료를 2개의 file server로부터 DoD server에 저장하고 client에게 전송하는 시스템.
<br>TCP 기반으로 소켓프로그래밍과 멀티스레딩을 통해 구현하였으며, 다수의 client의 요청을 동시에 처리할 수 있다. 

### Development environment
OS: Ubuntu 14.04.3 LTS
<br>Complier: gcc 4.8.4

### Complie/build 
gcc –o fileserver fileserver.c
<br>gcc –o fileserver2 fileserver2.c
<br>gcc –o dodserver dodserver.c –lpthread
<br>gcc –o client client.c

### RUN
-Create and fill up files that are listed in source code. (They are included in each file server folder in executable files folder)
<br>(File list: "file01.dat", "file02.dat", "file03.dat", "txt01.txt","code01.txt","file01.dat","file02.dat",
<br>"file03.dat", "txt01.txt", "code02.txt")
<br>./fileserver
<br>./fileserver2
<br>./dodserver
<br>-Enter IP addresses of file servers.
<br>./client (repeat 3 times)
<br>-Enter IP address of DoD server.

fileserver와 fileserver2의 차이는 port number이다. fileserver는 5001, fileserver2는 5002을 사용한다.
