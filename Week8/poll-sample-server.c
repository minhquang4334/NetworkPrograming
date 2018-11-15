struct poolfd sockets[1024], socknum = 0;

//initiate
for(int i=0; i<1024; i++)
	sockets[i] = -1;

listenfd = socket();
bind(listenfd);
listen(listenfd);
sockets[0].fd = listenfd;
sockets[0].events = POOLIN;

while(1){
	ret = pool(sockets,socknum, INFTIM);
	for(int i=0;i<1024;i++){
		if(sockets[0].revents & POOLIN){ //new client
			connfd = accept();
			if(socknum=1024) printf("too manu client!\n");
			else{
				for(int j=0;j<1024;j++){
					if(sockets[j].fd == -1){
						sockets[j].fd = connfd;
						sockets[j].events = POOLIN;
						socknum++;
					}
				}
			}
		}
		if(sockets[i].fd > -1 && sockets[i].revents && POOLIN){
			// do something
			// errror or client closed
			sockets[i].fd = -1;
			socknum--;
		}
	}
}