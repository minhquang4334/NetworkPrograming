#include <stdio.h>          
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#define	QSIZE	   8		/* size of input queue */
#define	MAXDG	4096		/* max datagram size */
#define SERV_PORT 5500
#define BUFF_SIZE 1024
#define NUMBEROFDOTSINIPV4 3 //number dots in ipv4
#define NUMBEROFDOTSINIPV6 5 //number dots in ipv6

typedef struct {
  void		*dg_data;		/* ptr to actual datagram */
  size_t	dg_len;			/* length of datagram */
  struct sockaddr  *dg_sa;	/* ptr to sockaddr{} w/client's address */
  socklen_t	dg_salen;		/* length of sockaddr{} */
} DG;
static DG	dg[QSIZE];			/* queue of datagrams to process */

static int	iget;		/* next one for main loop to process */
static int	iput;		/* next one for signal handler to read into */
static int	nqueue;		/* # on queue for main loop to process */
static socklen_t clilen;/* max length of sockaddr{} */
static int		sockfd;

static void	sig_io(int);

char **tokens;
struct in_addr ipv4addr;
struct hostent *host;
struct in_addr **addr_list;
struct in_addr **alias_list;

/*
* Check valid number in range 0 -> 255
* @param char* value
* @return boolean
*/
int validNumber(char *value)
{
    return (atoi(value) >= 0) && (atoi(value) <= 255);
}

/*
* Split string
* @param char* string
* @param const char a_delim
* @return boolean
* source https://stackoverflow.com/questions/9210528/split-string-with-delimiters-in-c
*/
char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}
/*
* Check dots in string equals dots in ip address
* @param char* string
* @return boolean(0,1)
*/
int checkDots(char *str)
{
    tokens = str_split(str, '.');
    if (tokens)
    {
        int i;
        for (i = 0; *(tokens + i); i++)
        {
            // count number elements in array
        }
        if((i-1) == NUMBEROFDOTSINIPV4 || (i-1) == 2) {
            return 1;
        }
    }
    return 0;
}
/*
* Check valid Ip
* @param char* string
* @return 1 if valid ip, 0 if invalid ip
*/
int checkIP(char *str)
{
    if(checkDots(str)) {
        if (tokens) {
            int i;
            for (i = 0; *(tokens + i); i++)
            {
                if(!validNumber(*(tokens + i))) {
                    return 0;
                }
                free(*(tokens + i));
            }
            free(tokens);
            return 1;
        }
    }
    return 0;
}
/*
* Show info Host From Ip Address
* @param char* ip
* @return void
*/
void showInfoFromIP(char *ip, char* str) {
    int i;
    inet_pton(AF_INET, ip, &ipv4addr);
    host = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);
    if (host != NULL)
    {
    	strcat(str, "Official Name: ");
    	strcat(str, host->h_name);
        strcat(str, "\nAlias Name: ");
       
        for (i = 0; host->h_aliases[i] != NULL; i++)
        {
        	strcat(str, host->h_aliases[i]);
        	strcat(str, "\n\0");
        }
    }
    else
    {
    	strcpy(str, "Not found information Of IP Address!\n");
    }
}
/*
* Show info Host From Domain Address
* @param char* domain
* @return void
*/
void showInfoFromDomain(char* domain, char* str) {
    int i;
    host = gethostbyname(domain);
    if (host != NULL) {
    	strcat(str, "Official IP: ");
        addr_list = (struct in_addr **)host->h_addr_list;
        strcat(str, inet_ntoa(*addr_list[0]));
        strcat(str, "\nAlias IP: ");
        for (i = 1; addr_list[i] != NULL; i++)
        {
        	strcat(str, inet_ntoa(*addr_list[i]));
        	strcat(str, "\n\0");
        }
    }
    else {
    	strcpy(str, "Not found information of domain!\n");
    }
}
/*
* Check valid port number
* @param int port
* @return 1 if valid port number, else return 0
*/
int validPortNumber(int port) {
	return (port > 0) && (port <= 65535);
}
/*
* Main function
* @param int argc, char** argv
* @return 1 if valid ip, 0 if invalid ip
*/
int main(int argc, char **argv)
{
	int port_number;
 	if(argc != 2) {
 		perror(" Error Parameter! Please input only port number\n ");
 		exit(0);
 	}
 	if((port_number = atoi(argv[1])) == 0) {
 		perror(" Please input port number\n");
 		exit(0);
 	}
 	if(!validPortNumber(port_number)) {
 		perror("Invalid Port Number!\n");
 		exit(0);
 	}
	int			i;
	const int	on = 1;
	sigset_t	zeromask, newmask, oldmask;
	
	struct sockaddr_in	servaddr, cliaddr;
	
	if ((sockfd=socket(AF_INET, SOCK_DGRAM, 0)) == -1 ){  /* calls socket() */
		perror("socket() error\n");
		return 0;
	}
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port_number);
	if(bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr))==-1){ /* calls bind() */
		perror("bind() error\n");
		return 0;
	}
		
	clilen = sizeof(cliaddr);
	
	for (i = 0; i < QSIZE; i++) {	/* init queue of buffers */
		dg[i].dg_data = malloc(MAXDG);
		dg[i].dg_sa = malloc(clilen);
		dg[i].dg_salen = clilen;
	}
	iget = iput = nqueue = 0;
	
	/* Signal handlers are established for SIGIO. The socket owner is
	 * set using fcntl and the signal-driven and non-blocking I/O flags are set using ioctl
	 */
	signal(SIGIO, sig_io);
	fcntl(sockfd, F_SETOWN, getpid());
	ioctl(sockfd, FIOASYNC, &on);
	ioctl(sockfd, FIONBIO, &on);
	
	/* Three signal sets are initialized: zeromask (which never changes),
	 * oldmask (which contains the old signal mask when we block SIGIO), and newmask.
	 */
	sigemptyset(&zeromask);		
	sigemptyset(&oldmask);
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGIO);	/* signal we want to block */
	
	/* Stores the current signal mask of the process in oldmask and then
	 * logically ORs newmask into the current signal mask. This blocks SIGIO
	 * and returns the current signal mask. We need SIGIO blocked when we test
	 * nqueue at the top of the loop
	 */
	sigprocmask(SIG_BLOCK, &newmask, &oldmask);
	
	for ( ; ; ) {
		while (nqueue == 0)
			sigsuspend(&zeromask);	/* wait for datagram to process */

		/* unblock SIGIO by calling sigprocmask to set the signal mask of
		 * the process to the value that was saved earlier (oldmask).
		 * The reply is then sent by sendto.
		 */
		sigprocmask(SIG_SETMASK, &oldmask, NULL);

		sendto(sockfd, dg[iget].dg_data, dg[iget].dg_len, 0,
			   dg[iget].dg_sa, dg[iget].dg_salen);

		if (++iget >= QSIZE)
			iget = 0;

		/* SIGIO is blocked and the value of nqueue is decremented.
		 * We must block the signal while modifying this variable since
		 * it is shared between the main loop and the signal handler.
		 */
		sigprocmask(SIG_BLOCK, &newmask, &oldmask);
		nqueue--;
	}
}

static void sig_io(int signo)
{
	ssize_t		len;
	DG			*ptr;
	for (; ; ) {
		if (nqueue >= QSIZE){
			perror("receive overflow");
			break;
		}

		ptr = &dg[iput];
		ptr->dg_salen = clilen;
		len = recvfrom(sockfd, ptr->dg_data, MAXDG, 0,
					   ptr->dg_sa, &ptr->dg_salen);
		if (len < 0) {
			if (errno == EWOULDBLOCK)
				break;		/* all done; no more queued to read */
			else{
				perror("recvfrom error");
				break;
			}
		}
		char temp[50];
		strcpy(temp, ptr->dg_data);
		char response[500] = "";
		if (checkIP(temp)) {
		    showInfoFromIP(ptr->dg_data, response); // save info host of ip in response
		} 
		else {
		    showInfoFromDomain(ptr->dg_data, response); // save info host of domain in reponse
		}
		ptr->dg_len = strlen(response);
		memcpy(ptr->dg_data, response, strlen(response));
		nqueue++;
		if (++iput >= QSIZE)
			iput = 0;

	}	
}
