#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <assert.h>
//define constant value
#define NUMBEROFDOTSINIPV4 3 //number dots in ipv4
#define NUMBEROFDOTSINIPV6 5 //number dots in ipv6
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

// global value
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
    if(!strcmp(value, "0")) {
        return 1;
    }
    return (atoi(value) > 0) && (atoi(value) <= 255);
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
        if((i-1) == NUMBEROFDOTSINIPV4) {
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
void showInfoFromIP(char *ip) {
    int i;
    inet_pton(AF_INET, ip, &ipv4addr);
    host = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);
    if (host != NULL)
    {
        printf("Official name: %s\n", host->h_name);
        printf("Alias name : \n");
        for (i = 0; host->h_aliases[i] != NULL; i++)
        {
            printf("%s \n", host->h_aliases[i]);
        }
    }
    else
    {
        printf("Not found information Of IP Address [%s]\n", ip);
    }
}
/*
* Show info Host From Domain Address
* @param char* domain
* @return void
*/
void showInfoFromDomain(char* domain) {
    int i;
    host = gethostbyname(domain);
    if (host != NULL) {
        addr_list = (struct in_addr **)host->h_addr_list;
        printf("Official IP: %s\n", inet_ntoa(*addr_list[0]));
        printf("Alias IP: \n");
        for (i = 1; addr_list[i] != NULL; i++)
        {
            printf("%s \n", inet_ntoa(*addr_list[i]));
        }
    }
    else {
        printf("Not found information of Domain[%s]\n", domain);
    }
}
/*
* Main function
* @param char** argv
* @param int argc
* @return int
*/
int main(int argc, char **argv)
{
    if (argc < 2)
        printf("Please input parameter \n");
    else if(argc > 2) {
        printf("Please input one parameter \n");
    }
    else {
        char *temp = malloc(sizeof(argv[1]) * strlen(argv[1]));
        strcpy(temp, argv[1]);
        if (checkIP(argv[1]))
        {
            showInfoFromIP(temp);
        } 
        else {
            showInfoFromDomain(temp);
        }
    }
    return 1;
}
