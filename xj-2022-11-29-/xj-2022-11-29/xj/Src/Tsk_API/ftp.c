/*

ftp for lwip freeRtos and ucosII

*/

#include "lwip/sockets.h"
#include <stdio.h>
#include "lwip/netdb.h"
#include <string.h>
#include "lwip/api.h"
#include "ftp.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include "lwip/dns.h"
#include "Utility.h"

#if !defined FTPLIB_DEFMODE
#define FTPLIB_DEFMODE FTPLIB_PASSIVE
#endif

#if defined(_WIN32)
#define SETSOCKOPT_OPTVAL_TYPE (const char *)
#else
#define SETSOCKOPT_OPTVAL_TYPE (void *)
#endif

#define FTPLIB_BUFSIZ 	256

#define RESPONSE_BUFSIZ 128
#define TMP_BUFSIZ      256
#define ACCEPT_TIMEOUT 5000

#define FTPLIB_CONTROL 0
#define FTPLIB_READ 1
#define FTPLIB_WRITE 2


struct NetBuf {
    char *cput,*cget;
    int handle;
    int cavail,cleft;
    char *buf;
    int dir;
    netbuf *ctrl;
    netbuf *data;
    int cmode;
    struct timeval idletime;
    FtpCallback idlecb;
    void *idlearg;
    unsigned long int xfered;
    unsigned long int cbbytes;
    unsigned long int xfered1;
    char response[RESPONSE_BUFSIZ];
};

//static char *version =
//    "ftp Release 4.0 07-Jun-2013, copyright 1996-2003, 2013 Thomas Pfau";
int ftplib_debug = 0;

#define net_read(x,y,z) read(x,y,z)
#define net_write(x,y,z) write(x,y,z)
#define net_close closesocket

FTP_PARA_INFO  ftp_paraInfo;    // ftp配置信息


/*
 * socket_wait - wait for socket to receive or flush data
 *
 * return 1 if no user callback, otherwise, return value returned by
 * user callback
 */
static int socket_wait(netbuf *ctl)
{
    fd_set fd,*rfd = NULL,*wfd = NULL;
    struct timeval tv;
    int rv = 0;
    if ((ctl->dir == FTPLIB_CONTROL) || (ctl->idlecb == NULL))
	return 1;
    if (ctl->dir == FTPLIB_WRITE)
	wfd = &fd;
    else
	rfd = &fd;
    FD_ZERO(&fd);
    do
    {
	FD_SET(ctl->handle,&fd);
	tv = ctl->idletime;
	rv = select(ctl->handle+1, rfd, wfd, NULL, &tv);
	if (rv == -1)
	{
	    rv = 0;
	    strncpy(ctl->ctrl->response, strerror(errno),
                    sizeof(ctl->ctrl->response));
	    break;
	}
	else if (rv > 0)
	{
	    rv = 1;
	    break;
	}
    }
    while ((rv = ctl->idlecb(ctl, ctl->xfered, ctl->idlearg)));
    return rv;
}


void* memccpy(void *dst,const void *src,char c,int count)
{
	while(count)
	{
		*(char*)dst = *(char *)src;
		dst = (char*)dst+1;
		if(*(char*)src == (char)c)
		{
			break;
		}
		src = (char*)src + 1;
		count--;
	}
	return (count?dst:NULL);
}

char *strdup(const char *str)
{
	int len = 0;
	char *dst ;
	if(str ==NULL)
		return NULL;
	len = strlen(str);

	dst = malloc(len + 1);

	strcpy(dst,str);

	return dst;
}

/*
 * read a line of text
 *
 * return -1 on error or bytecount
 */
static int readline(char *buf,int max,netbuf *ctl)
{
    int x,retval = 0;
    char *end,*bp=buf;
    int eof = 0;

    if ((ctl->dir != FTPLIB_CONTROL) && (ctl->dir != FTPLIB_READ))
	return -1;
    if (max == 0)
	return 0;
    do
    {
    	if (ctl->cavail > 0)
    	{
	    x = (max >= ctl->cavail) ? ctl->cavail : max-1;
	    end = memccpy(bp,ctl->cget,'\n',x);
	    if (end != NULL)
		x = end - bp;
	    retval += x;
	    bp += x;
	    *bp = '\0';
	    max -= x;
	    ctl->cget += x;
	    ctl->cavail -= x;
	    if (end != NULL)
	    {
		bp -= 2;
		if (strcmp(bp,"\r\n") == 0)
		{
		    *bp++ = '\n';
		    *bp++ = '\0';
		    --retval;
		}
	    	break;
	    }
    	}
    	if (max == 1)
    	{
	    *buf = '\0';
	    break;
    	}
    	if (ctl->cput == ctl->cget)
    	{
	    ctl->cput = ctl->cget = ctl->buf;
	    ctl->cavail = 0;
	    ctl->cleft = FTPLIB_BUFSIZ;
    	}
	if (eof)
	{
	    if (retval == 0)
		retval = -1;
	    break;
	}
	if (!socket_wait(ctl))
	    return retval;
    	if ((x = net_read(ctl->handle,ctl->cput,ctl->cleft)) == -1)
    	{
	    if (ftplib_debug)
		perror("read");
	    retval = -1;
	    break;
    	}
	if (x == 0)
	    eof = 1;
    	ctl->cleft -= x;
    	ctl->cavail += x;
    	ctl->cput += x;
    }
    while (1);
    return retval;
}


/*
 * write lines of text
 *
 * return -1 on error or bytecount
 */
static int writeline(const char *buf, int len, netbuf *nData)
{
    int x, nb=0, w;
    const char *ubp = buf;
    char *nbp;
    char lc=0;

    if (nData->dir != FTPLIB_WRITE)
	return -1;
    nbp = nData->buf;
    for (x=0; x < len; x++)
    {
	if ((*ubp == '\n') && (lc != '\r'))
	{
	    if (nb == FTPLIB_BUFSIZ)
	    {
		if (!socket_wait(nData))
		    return x;
		w = net_write(nData->handle, nbp, FTPLIB_BUFSIZ);
		if (w != FTPLIB_BUFSIZ)
		{
		    if (ftplib_debug)
			printf("net_write(1) returned %d, errno = %d\n", w, errno);
		    return(-1);
		}
		nb = 0;
	    }
	    nbp[nb++] = '\r';
	}
	if (nb == FTPLIB_BUFSIZ)
	{
	    if (!socket_wait(nData))
		return x;
	    w = net_write(nData->handle, nbp, FTPLIB_BUFSIZ);
	    if (w != FTPLIB_BUFSIZ)
	    {
		if (ftplib_debug)
		    printf("net_write(2) returned %d, errno = %d\n", w, errno);
		return(-1);
	    }
	    nb = 0;
	}
	nbp[nb++] = lc = *ubp++;
    }
    if (nb)
    {
	if (!socket_wait(nData))
	    return x;
	w = net_write(nData->handle, nbp, nb);
	if (w != nb)
	{
	    if (ftplib_debug)
		printf("net_write(3) returned %d, errno = %d\n", w, errno);
	    return(-1);
	}
    }
    return len;
}

/*
 * read a response from the server
 *
 * return 0 if first char doesn't match
 * return 1 if first char matches
 */
static int readresp(char c, netbuf *nControl)
{
    char match[5];
    if (readline(nControl->response,RESPONSE_BUFSIZ,nControl) == -1)
    {
	if (ftplib_debug)
	    perror("Control socket read failed");

	return 0;
    }
    if (ftplib_debug > 1)
	fprintf(stderr,"%s",nControl->response);
    if (nControl->response[3] == '-')
    {
	strncpy(match,nControl->response,3);
	match[3] = ' ';
	match[4] = '\0';
	do
	{
	    if (readline(nControl->response,RESPONSE_BUFSIZ,nControl) == -1)
	    {
		if (ftplib_debug)
		    perror("Control socket read failed");
		return 0;
	    }
	    if (ftplib_debug > 1)
		fprintf(stderr,"%s",nControl->response);
	}
	while (strncmp(nControl->response,match,4));
    }
    if (nControl->response[0] == c)
		return 1;
    return 0;
}
/*
 * FtpInit
*/

void FtpInit(void)
{
//	memset(&ftp_paraInfo,0,sizeof(ftp_paraInfo));
//
//	strcpy(ftp_paraInfo.ftpIp,"192.168.88.11");
//	ftp_paraInfo.ftpport = 21;
//
//	strcpy(ftp_paraInfo.ftpuser,"whq");
//	strcpy(ftp_paraInfo.ftppsw,"whq");
//
//	strcpy(ftp_paraInfo.ftppath,"./EVD_app.bin");
}


/*
 * FtpLastResponse - return a pointer to the last response received
 */
char *FtpLastResponse(netbuf *nControl)
{
    if ((nControl) && (nControl->dir == FTPLIB_CONTROL))
    	return nControl->response;
    return NULL;
}


/*
 * FtpConnect - connect to remote server
 *
 * return 1 if connected, 0 if not
 */
int FtpConnect(const char *host,int port, netbuf **nControl)
{
    int sControl;
    struct sockaddr_in sin;
//    int on=1;
    netbuf *ctrl;
//    char *lhost;
//    char *pnum;
    //struct hostent *pURL = NULL;

//add
    memset(&sin,0,sizeof(sin));
	sin.sin_family=AF_INET;
	sin.sin_addr.s_addr= 0xAE723772;//inet_addr(host);
	sin.sin_port=htons(port);

//    memset(&sin,0,sizeof(sin));
//    pURL = gethostbyname(host);
//
//	sin.sin_family=AF_INET;
//    sin.sin_addr.s_addr = *((unsigned long*) pURL->h_addr_list[0]);
//	sin.sin_port=htons(port);

    sControl = socket(PF_INET, SOCK_STREAM,0 );//IPPROTO_TCP
    if (sControl == -1)
    {
		if (ftplib_debug)
			perror("socket");
		return 0;
    }

//    if (setsockopt(sControl,SOL_SOCKET,SO_REUSEADDR,
//		   SETSOCKOPT_OPTVAL_TYPE &on, sizeof(on)) == -1)
//    {
//		if (ftplib_debug)
//			perror("setsockopt");
//		net_close(sControl);
//		return 0;
//    }

    if (connect(sControl, (struct sockaddr *)&sin, sizeof(sin)) == -1)
    {
		if (ftplib_debug)
			perror("connect");
		net_close(sControl);
		return 0;
    }
    ctrl = calloc(1,sizeof(netbuf));
    if (ctrl == NULL)
    {
		if (ftplib_debug)
			perror("calloc");
		net_close(sControl);
		return 0;
    }
    ctrl->buf = malloc(FTPLIB_BUFSIZ);
    if (ctrl->buf == NULL)
    {
		if (ftplib_debug)
			perror("calloc");
		net_close(sControl);
		free(ctrl);
		return 0;
    }
    ctrl->handle = sControl;
    ctrl->dir = FTPLIB_CONTROL;
    ctrl->ctrl = NULL;
    ctrl->cmode = FTPLIB_DEFMODE;
    ctrl->idlecb = NULL;
    ctrl->idletime.tv_sec = ctrl->idletime.tv_usec = 0;
    ctrl->idlearg = NULL;
    ctrl->xfered = 0;
    ctrl->xfered1 = 0;
    ctrl->cbbytes = 0;
	osDelay(500);
    if (readresp('2', ctrl) == 0)
    {
	net_close(sControl);
	free(ctrl->buf);
	free(ctrl);
	return 0;
    }
    *nControl = ctrl;
    return 1;
}
int FtpSetCallback(const FtpCallbackOptions *opt, netbuf *nControl)
{
    nControl->idlecb = opt->cbFunc;
    nControl->idlearg = opt->cbArg;
    nControl->idletime.tv_sec = opt->idleTime / 1000;
    nControl->idletime.tv_usec = (opt->idleTime % 1000) * 1000;
    nControl->cbbytes = opt->bytesXferred;
    return 1;
}
int FtpClearCallback(netbuf *nControl)
{
    nControl->idlecb = NULL;
    nControl->idlearg = NULL;
    nControl->idletime.tv_sec = 0;
    nControl->idletime.tv_usec = 0;
    nControl->cbbytes = 0;
    return 1;
}
/*
 * FtpOptions - change connection options
 *
 * returns 1 if successful, 0 on error
 */
int FtpOptions(int opt, long val, netbuf *nControl)
{
    int v,rv=0;
    switch (opt)
    {
      case FTPLIB_CONNMODE:
	v = (int) val;
	if ((v == FTPLIB_PASSIVE) || (v == FTPLIB_PORT))
	{
	    nControl->cmode = v;
	    rv = 1;
	}
	break;
      case FTPLIB_CALLBACK:
	nControl->idlecb = (FtpCallback) val;
	rv = 1;
	break;
      case FTPLIB_IDLETIME:
	v = (int) val;
	rv = 1;
	nControl->idletime.tv_sec = v / 1000;
	nControl->idletime.tv_usec = (v % 1000) * 1000;
	break;
      case FTPLIB_CALLBACKARG:
	rv = 1;
	nControl->idlearg = (void *) val;
	break;
      case FTPLIB_CALLBACKBYTES:
        rv = 1;
        nControl->cbbytes = (int) val;
        break;
    }
    return rv;
}

/*
 * FtpSendCmd - send a command and wait for expected response
 *
 * return 1 if proper response received, 0 otherwise
 */
static int FtpSendCmd(const char *cmd, char expresp, netbuf *nControl)
{
	char buf[TMP_BUFSIZ];
	if (nControl->dir != FTPLIB_CONTROL)
	return 0;
	if (ftplib_debug > 2)
	fprintf(stderr,"%s\n",cmd);
	if ((strlen(cmd) + 3) > sizeof(buf))
		return 0;
	sprintf(buf,"%s\r\n",cmd);
	if (net_write(nControl->handle,buf,strlen(buf)) <= 0)
	{
	if (ftplib_debug)
		perror("write");
	return 0;
	}
	return readresp(expresp, nControl);
}

/*
 * FtpLogin - log in to remote server
 *
 * return 1 if logged in, 0 otherwise
 */
int FtpLogin(const char *user, const char *pass, netbuf *nControl)
{
    char tempbuf[64];

    if (((strlen(user) + 7) > sizeof(tempbuf)) ||
        ((strlen(pass) + 7) > sizeof(tempbuf)))
        return 0;
    sprintf(tempbuf,"USER %s",user);
    if (!FtpSendCmd(tempbuf,'3',nControl))
    {
	if (nControl->response[0] == '2')
	    return 1;
	return 0;
    }
    sprintf(tempbuf,"PASS %s",pass);
    return FtpSendCmd(tempbuf,'2',nControl);
}

/*
 * FtpOpenPort - set up data connection
 *
 * return 1 if successful, 0 otherwise
 */
static int FtpOpenPort(netbuf *nControl, netbuf **nData, int mode, int dir)
{
    int sData;
    union {
	struct sockaddr sa;
	struct sockaddr_in in;
    } sin;
//    struct linger lng = { 0, 0 };
    unsigned int l;
    //int on=1;
    netbuf *ctrl;
    char *cp;
    unsigned int v[6];
    char buf[TMP_BUFSIZ];

    if (nControl->dir != FTPLIB_CONTROL)
		return -1;
    if ((dir != FTPLIB_READ) && (dir != FTPLIB_WRITE))
    {
		sprintf(nControl->response, "Invalid direction %d\n", dir);
		return -1;
    }
    if ((mode != FTPLIB_ASCII) && (mode != FTPLIB_IMAGE))
    {
		sprintf(nControl->response, "Invalid mode %c\n", mode);
		return -1;
		}
		l = sizeof(sin);
		if (nControl->cmode == FTPLIB_PASSIVE)
		{
		memset(&sin, 0, l);
		sin.in.sin_family = AF_INET;
		if (!FtpSendCmd("PASV",'2',nControl))
			return -1;
		cp = strchr(nControl->response,'(');
		if (cp == NULL)
			return -1;
		cp++;
		sscanf(cp,"%u,%u,%u,%u,%u,%u",&v[2],&v[3],&v[4],&v[5],&v[0],&v[1]);
		sin.sa.sa_data[2] = v[2];
		sin.sa.sa_data[3] = v[3];
		sin.sa.sa_data[4] = v[4];
		sin.sa.sa_data[5] = v[5];
		sin.sa.sa_data[0] = v[0];
		sin.sa.sa_data[1] = v[1];
    }
    else
    {
	if (getsockname(nControl->handle, &sin.sa, (socklen_t *)&l) < 0)
	{
	    if (ftplib_debug)
		perror("getsockname");
	    return -1;
	}
    }
    sData = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (sData == -1)
    {
		if (ftplib_debug)
			perror("socket");
		return -1;
    }
//    if (setsockopt(sData,SOL_SOCKET,SO_REUSEADDR,
//		   SETSOCKOPT_OPTVAL_TYPE &on,sizeof(on)) == -1)
//    {
//	if (ftplib_debug)
//	    perror("setsockopt");
//	net_close(sData);
//	return -1;
//    }
//    if (setsockopt(sData,SOL_SOCKET,SO_LINGER,
//		   SETSOCKOPT_OPTVAL_TYPE &lng,sizeof(lng)) == -1)
//    {
//	if (ftplib_debug)
//	    perror("setsockopt");
//	net_close(sData);
//	return -1;
//    }
    if (nControl->cmode == FTPLIB_PASSIVE)
    {
		if (connect(sData, &sin.sa, sizeof(sin.sa)) == -1)
		{
			if (ftplib_debug)
			perror("connect");
			net_close(sData);
			return -1;
		}
    }
    else
    {
		sin.in.sin_port = 0;
		if (bind(sData, &sin.sa, sizeof(sin)) == -1)
		{
			if (ftplib_debug)
			perror("bind");
			net_close(sData);
			return -1;
		}
		if (listen(sData, 1) < 0)
		{
			if (ftplib_debug)
			perror("listen");
			net_close(sData);
			return -1;
		}
		if (getsockname(sData, &sin.sa, (socklen_t *)&l) < 0)
			return -1;
		sprintf(buf, "PORT %d,%d,%d,%d,%d,%d",
			(unsigned char) sin.sa.sa_data[2],
			(unsigned char) sin.sa.sa_data[3],
			(unsigned char) sin.sa.sa_data[4],
			(unsigned char) sin.sa.sa_data[5],
			(unsigned char) sin.sa.sa_data[0],
			(unsigned char) sin.sa.sa_data[1]);
		if (!FtpSendCmd(buf,'2',nControl))
		{
			net_close(sData);
			return -1;
		}
    }
    ctrl = calloc(1,sizeof(netbuf));

    if (ctrl == NULL)
    {
		if (ftplib_debug)
			perror("calloc");
		net_close(sData);
		return -1;
    }
    if ((mode == 'A') && ((ctrl->buf = malloc(FTPLIB_BUFSIZ)) == NULL))
    {
		if (ftplib_debug)
			perror("calloc");
		net_close(sData);
		free(ctrl);
		return -1;
    }
    ctrl->handle = sData;
    ctrl->dir = dir;
    ctrl->idletime = nControl->idletime;
    ctrl->idlearg = nControl->idlearg;
    ctrl->xfered = 0;
    ctrl->xfered1 = 0;
    ctrl->cbbytes = nControl->cbbytes;
    if (ctrl->idletime.tv_sec || ctrl->idletime.tv_usec || ctrl->cbbytes)
		ctrl->idlecb = nControl->idlecb;
    else
		ctrl->idlecb = NULL;
    *nData = ctrl;
    return 1;
}

/*
 * FtpAcceptConnection - accept connection from server
 *
 * return 1 if successful, 0 otherwise
 */
static int FtpAcceptConnection(netbuf *nData, netbuf *nControl)
{
    int sData;
    struct sockaddr addr;
    unsigned int l;
    int i;
    struct timeval tv;
    fd_set mask;
    int rv;

    FD_ZERO(&mask);
    FD_SET(nControl->handle, &mask);
    FD_SET(nData->handle, &mask);
    tv.tv_usec = 0;
    tv.tv_sec = ACCEPT_TIMEOUT;
    i = nControl->handle;
    if (i < nData->handle)
	i = nData->handle;
    i = select(i+1, &mask, NULL, NULL, &tv);
    if (i == -1)
    {
        strncpy(nControl->response, strerror(errno),
                sizeof(nControl->response));
        net_close(nData->handle);
        nData->handle = 0;
        rv = 0;
    }
    else if (i == 0)
    {
		strcpy(nControl->response, "timed out waiting for connection");
		net_close(nData->handle);
		nData->handle = 0;
		rv = 0;
    }
    else
    {
		if (FD_ISSET(nData->handle, &mask))
		{
			l = sizeof(addr);
			sData = accept(nData->handle, &addr, (socklen_t *)&l);
			i = errno;
			net_close(nData->handle);
			if (sData > 0)
			{
			rv = 1;
			nData->handle = sData;
			}
			else
			{
			strncpy(nControl->response, strerror(i),
							sizeof(nControl->response));
			nData->handle = 0;
			rv = 0;
			}
		}
		else if (FD_ISSET(nControl->handle, &mask))
		{
			net_close(nData->handle);
			nData->handle = 0;
			readresp('2', nControl);
			rv = 0;
		}
    }
    return rv;
}

/*
 * FtpAccess - return a handle for a data stream
 *
 * return 1 if successful, 0 otherwise
 */
int FtpAccess(const char *path, int typ, int mode, netbuf *nControl,
    netbuf **nData)
{
    char buf[TMP_BUFSIZ];
    int dir;
    if ((path == NULL) &&
        ((typ == FTPLIB_FILE_WRITE) || (typ == FTPLIB_FILE_READ)))
    {
	sprintf(nControl->response,
                "Missing path argument for file transfer\n");
	return 0;
    }
    sprintf(buf, "TYPE %c", mode);
    if (!FtpSendCmd(buf, '2', nControl))
	return 0;
    switch (typ)
    {
      case FTPLIB_DIR:
	strcpy(buf,"NLST");
	dir = FTPLIB_READ;
	break;
      case FTPLIB_DIR_VERBOSE:
	strcpy(buf,"LIST");
	dir = FTPLIB_READ;
	break;
      case FTPLIB_FILE_READ:
	strcpy(buf,"RETR");
	dir = FTPLIB_READ;
	break;
      case FTPLIB_FILE_WRITE:
	strcpy(buf,"STOR");
	dir = FTPLIB_WRITE;
	break;
      default:
	sprintf(nControl->response, "Invalid open type %d\n", typ);
	return 0;
    }
    if (path != NULL)
    {
        int i = strlen(buf);
        buf[i++] = ' ';
        if ((strlen(path) + i + 1) >= sizeof(buf))
            return 0;
        strcpy(&buf[i],path);
    }
    if (FtpOpenPort(nControl, nData, mode, dir) == -1)
	return 0;
    if (!FtpSendCmd(buf, '1', nControl))
    {
	FtpClose(*nData);
	*nData = NULL;
	return 0;
    }
    (*nData)->ctrl = nControl;
    nControl->data = *nData;
    if (nControl->cmode == FTPLIB_PORT)
    {
	if (!FtpAcceptConnection(*nData,nControl))
	{
	    FtpClose(*nData);
	    *nData = NULL;
	    nControl->data = NULL;
	    return 0;
	}
    }
    return 1;
}
/*
 * FtpRead - read from a data connection
 */
int FtpRead(void *buf, int max, netbuf *nData)
{
    int i;
    if (nData->dir != FTPLIB_READ)
	return 0;
    if (nData->buf)
        i = readline(buf, max, nData);
    else
    {
        i = socket_wait(nData);
	if (i != 1)
	    return 0;
        i = net_read(nData->handle, buf, max);
    }
    if (i == -1)
	return 0;
    nData->xfered += i;
    if (nData->idlecb && nData->cbbytes)
    {
        nData->xfered1 += i;
        if (nData->xfered1 > nData->cbbytes)
        {
	    if (nData->idlecb(nData, nData->xfered, nData->idlearg) == 0)
		return 0;
            nData->xfered1 = 0;
        }
    }
    return i;
}

/*
 * FtpWrite - write to a data connection
 */
int FtpWrite(const void *buf, int len, netbuf *nData)
{
    int i;
    if (nData->dir != FTPLIB_WRITE)
	return 0;
    if (nData->buf)
    	i = writeline(buf, len, nData);
    else
    {
        socket_wait(nData);
        i = net_write(nData->handle, buf, len);
    }
    if (i == -1)
	return 0;
    nData->xfered += i;
    if (nData->idlecb && nData->cbbytes)
    {
        nData->xfered1 += i;
        if (nData->xfered1 > nData->cbbytes)
        {
            nData->idlecb(nData, nData->xfered, nData->idlearg);
            nData->xfered1 = 0;
        }
    }
    return i;
}

/*
 * FtpClose - close a data connection
 */
int FtpClose(netbuf *nData)
{
    netbuf *ctrl;
    switch (nData->dir)
    {
      case FTPLIB_WRITE:
	/* potential problem - if buffer flush fails, how to notify user? */
	if (nData->buf != NULL)
	    writeline(NULL, 0, nData);
      case FTPLIB_READ:
	if (nData->buf)
	    free(nData->buf);
	shutdown(nData->handle,2);
	net_close(nData->handle);
	ctrl = nData->ctrl;
    ctrl = ctrl;//add
	free(nData);
//	ctrl->data = NULL;
//	if (ctrl && ctrl->response[0] != '4' && ctrl->response[0] != 5)
//	{
//	    return(readresp('2', ctrl));
//	}
	return 1;
      case FTPLIB_CONTROL:
	if (nData->data)
	{
	    nData->ctrl = NULL;
	    FtpClose(nData->data);
	}
	net_close(nData->handle);
	free(nData);
	return 0;
    }
    return 1;
}


/*
 * FtpSite - send a SITE command
 *
 * return 1 if command successful, 0 otherwise
 */
 int FtpSite(const char *cmd, netbuf *nControl)
{
    char buf[TMP_BUFSIZ];

    if ((strlen(cmd) + 7) > sizeof(buf))
        return 0;
    sprintf(buf,"SITE %s",cmd);
    if (!FtpSendCmd(buf,'2',nControl))
	return 0;
    return 1;
}

/*
 * FtpSysType - send a SYST command
 *
 * Fills in the user buffer with the remote system type.  If more
 * information from the response is required, the user can parse
 * it out of the response buffer returned by FtpLastResponse().
 *
 * return 1 if command successful, 0 otherwise
 */
 int FtpSysType(char *buf, int max, netbuf *nControl)
{
    int l = max;
    char *b = buf;
    char *s;
    if (!FtpSendCmd("SYST",'2',nControl))
	return 0;
    s = &nControl->response[4];
    while ((--l) && (*s != ' '))
	*b++ = *s++;
    *b++ = '\0';
    return 1;
}

/*
 * FtpMkdir - create a directory at server
 *
 * return 1 if successful, 0 otherwise
 */
 int FtpMkdir(const char *path, netbuf *nControl)
{
    char buf[TMP_BUFSIZ];

    if ((strlen(path) + 6) > sizeof(buf))
        return 0;
    sprintf(buf,"MKD %s",path);
    if (!FtpSendCmd(buf,'2', nControl))
	return 0;
    return 1;
}

/*
 * FtpChdir - change path at remote
 *
 * return 1 if successful, 0 otherwise
 */
 int FtpChdir(const char *path, netbuf *nControl)
{
    char buf[TMP_BUFSIZ];

    if ((strlen(path) + 6) > sizeof(buf))
        return 0;
    sprintf(buf,"CWD %s",path);
    if (!FtpSendCmd(buf,'2',nControl))
	return 0;
    return 1;
}

/*
 * FtpCDUp - move to parent directory at remote
 *
 * return 1 if successful, 0 otherwise
 */
 int FtpCDUp(netbuf *nControl)
{
    if (!FtpSendCmd("CDUP",'2',nControl))
	return 0;
    return 1;
}

/*
 * FtpRmdir - remove directory at remote
 *
 * return 1 if successful, 0 otherwise
 */
 int FtpRmdir(const char *path, netbuf *nControl)
{
    char buf[TMP_BUFSIZ];

    if ((strlen(path) + 6) > sizeof(buf))
        return 0;
    sprintf(buf,"RMD %s",path);
    if (!FtpSendCmd(buf,'2',nControl))
	return 0;
    return 1;
}

/*
 * FtpPwd - get working directory at remote
 *
 * return 1 if successful, 0 otherwise
 */
 int FtpPwd(char *path, int max, netbuf *nControl)
{
    int l = max;
    char *b = path;
    char *s;
    if (!FtpSendCmd("PWD",'2',nControl))
	return 0;
    s = strchr(nControl->response, '"');
    if (s == NULL)
	return 0;
    s++;
    while ((--l) && (*s) && (*s != '"'))
	*b++ = *s++;
    *b++ = '\0';
    return 1;
}

/*
 * FtpXfer - issue a command and transfer data
 *
 * return 1 if successful, 0 otherwise
 */
static int FtpXfer(const char *localfile, const char *path,
	netbuf *nControl, int typ, int mode)
{
    int l,c;
    char *dbuf;
    FILE *local = NULL;
    netbuf *nData;
    int rv=1;

    if (localfile != NULL)
    {
	char ac[4];
	memset( ac, 0, sizeof(ac) );
	if (typ == FTPLIB_FILE_WRITE)
	    ac[0] = 'r';
	else
	    ac[0] = 'w';
	if (mode == FTPLIB_IMAGE)
	    ac[1] = 'b';
	local = fopen(localfile, ac);
	if (local == NULL)
	{
	    strncpy(nControl->response, strerror(errno),
                    sizeof(nControl->response));
	    return 0;
	}
    }
    if (local == NULL)
	local = (typ == FTPLIB_FILE_WRITE) ? stdin : stdout;
    if (!FtpAccess(path, typ, mode, nControl, &nData))
    {
	if (localfile)
	{
	    fclose(local);
	    if ( typ == FTPLIB_FILE_WRITE )
			;
		//unlink(localfile);
	}
	return 0;
    }
    dbuf = malloc(FTPLIB_BUFSIZ);
    if (typ == FTPLIB_FILE_WRITE)
    {
	while ((l = fread(dbuf, 1, FTPLIB_BUFSIZ, local)) > 0)
	{
	    if ((c = FtpWrite(dbuf, l, nData)) < l)
	    {
		printf("short write: passed %d, wrote %d\n", l, c);
		rv = 0;
		break;
	    }
	}
    }
    else
    {
    	while ((l = FtpRead(dbuf, FTPLIB_BUFSIZ, nData)) > 0)
	{
	    if (fwrite(dbuf, 1, l, local) == 0)
	    {
		if (ftplib_debug)
		    perror("localfile write");
		rv = 0;
		break;
	    }
	}
    }
    free(dbuf);
    fflush(local);
    if (localfile != NULL)
	fclose(local);
    FtpClose(nData);
    return rv;
}

/*
 * FtpNlst - issue an NLST command and write response to output
 *
 * return 1 if successful, 0 otherwise
 */
 int FtpNlst(const char *outputfile, const char *path,
	netbuf *nControl)
{
    return FtpXfer(outputfile, path, nControl, FTPLIB_DIR, FTPLIB_ASCII);
}

/*
 * FtpDir - issue a LIST command and write response to output
 *
 * return 1 if successful, 0 otherwise
 */
 int FtpDir(const char *outputfile, const char *path, netbuf *nControl)
{
    return FtpXfer(outputfile, path, nControl, FTPLIB_DIR_VERBOSE, FTPLIB_ASCII);
}

/*
 * FtpSize - determine the size of a remote file
 *
 * return 1 if successful, 0 otherwise
 */
 int FtpSize(const char *path, unsigned int *size, char mode, netbuf *nControl)
{

	char cmd[TMP_BUFSIZ];
    int resp,rv=1;
    unsigned int sz;

    if ((strlen(path) + 7) > sizeof(cmd))
        return 0;
    sprintf(cmd, "TYPE %c", mode);
    if (!FtpSendCmd(cmd, '2', nControl))
	return 0;
    sprintf(cmd,"SIZE %s",path);
    if (!FtpSendCmd(cmd,'2',nControl))
	rv = 0;
    else
    {
	if (sscanf(nControl->response, "%d %u", &resp, &sz) == 2)
	    *size = sz;
	else
	    rv = 0;
    }
    return rv;
}

#if defined(__UINT64_MAX)
/*
 * FtpSizeLong - determine the size of a remote file
 *
 * return 1 if successful, 0 otherwise
 */
 int FtpSizeLong(const char *path, fsz_t *size, char mode, netbuf *nControl)
{
    char cmd[TMP_BUFSIZ];
    int resp,rv=1;
    fsz_t sz;

    if ((strlen(path) + 7) > sizeof(cmd))
        return 0;
    sprintf(cmd, "TYPE %c", mode);
    if (!FtpSendCmd(cmd, '2', nControl))
	return 0;
    sprintf(cmd,"SIZE %s",path);
    if (!FtpSendCmd(cmd,'2',nControl))
	rv = 0;
    else
    {
	if (sscanf(nControl->response, "%d %" PRIu64 "", &resp, &sz) == 2)
	    *size = sz;
	else
	    rv = 0;
    }
    return rv;
}
#endif

/*
 * FtpModDate - determine the modification date of a remote file
 *
 * return 1 if successful, 0 otherwise
 */
 int FtpModDate(const char *path, char *dt, int max, netbuf *nControl)
{
    char buf[TMP_BUFSIZ];
    int rv = 1;

    if ((strlen(path) + 7) > sizeof(buf))
        return 0;
    sprintf(buf,"MDTM %s",path);
    if (!FtpSendCmd(buf,'2',nControl))
	rv = 0;
    else
	strncpy(dt, &nControl->response[4], max);
    return rv;
}

/*
 * FtpGet - issue a GET command and write received data to output
 *
 * return 1 if successful, 0 otherwise
 */
 int FtpGet(const char *outputfile, const char *path,
	char mode, netbuf *nControl)
{
    return FtpXfer(outputfile, path, nControl, FTPLIB_FILE_READ, mode);
}

/*
 * FtpPut - issue a PUT command and send data from input
 *
 * return 1 if successful, 0 otherwise
 */
 int FtpPut(const char *inputfile, const char *path, char mode,
	netbuf *nControl)
{
    return FtpXfer(inputfile, path, nControl, FTPLIB_FILE_WRITE, mode);
}

/*
 * FtpRename - rename a file at remote
 *
 * return 1 if successful, 0 otherwise
 */
 int FtpRename(const char *src, const char *dst, netbuf *nControl)
{
    char cmd[TMP_BUFSIZ];

    if (((strlen(src) + 7) > sizeof(cmd)) ||
        ((strlen(dst) + 7) > sizeof(cmd)))
        return 0;
    sprintf(cmd,"RNFR %s",src);
    if (!FtpSendCmd(cmd,'3',nControl))
	return 0;
    sprintf(cmd,"RNTO %s",dst);
    if (!FtpSendCmd(cmd,'2',nControl))
	return 0;
    return 1;
}

/*
 * FtpDelete - delete a file at remote
 *
 * return 1 if successful, 0 otherwise
 */
 int FtpDelete(const char *fnm, netbuf *nControl)
{
    char cmd[TMP_BUFSIZ];

    if ((strlen(fnm) + 7) > sizeof(cmd))
        return 0;
    sprintf(cmd,"DELE %s",fnm);
    if (!FtpSendCmd(cmd,'2', nControl))
	return 0;
    return 1;
}

/*
 * FtpQuit - disconnect from remote
 *
 * return 1 if successful, 0 otherwise
 */
 void FtpQuit(netbuf *nControl)
{
    if (nControl->dir != FTPLIB_CONTROL)
	return;
    FtpSendCmd("QUIT",'2',nControl);
    net_close(nControl->handle);
    free(nControl->buf);
    free(nControl);
}

//static int logged_in = 0;
//static char *host = NULL;
//static char *user = NULL;
//static char *pass = NULL;
//static char mode = 'I';
//static int action = 0;
//static char *invocation;

//#include "includes.h"
//#include "flash_userProgram.h"
//// 写文件之前擦除相关区域
//void Erase_Rom_data(void)
//{
//	  osDelay(2000);
//    if (user_flash_erase(BAK_ADDR,BACK_SECTOR_NUM)!=1)   //擦除备份区
//    {

//    }

//    if (user_flash_erase(PARA2_ADDR,PARA2_SECTOR_NUM)!=1)    //擦除参数区2
//    {

//    }
//}


int ftp_put_file(const char *localfile, const char *path,
	netbuf *nControl,int typ,int mode)
{
  int l,c;
	int out = 0;
//	int cnt = 0;
    char *dbuf;
//    FILE *local = NULL;
    netbuf *nData;
    int rv=1;
	typ = FTPLIB_FILE_WRITE;
    if (!FtpAccess(path, typ, mode, nControl, &nData))
    {
		dbg_printf(1,"call ftpaccess fila ");
		return 0;
    }

    dbuf = malloc(FTPLIB_BUFSIZ);

	while(1)
	{
		int i = 0;
		for(i = 0; i< FTPLIB_BUFSIZ;i++)
		{
			dbuf[i] = (i % 10 ) + 0x30;
		}
		if ((c = FtpWrite(dbuf, FTPLIB_BUFSIZ, nData)) < l)
		{
			dbg_printf(1,"short write: passed %d, wrote %d\n", l, c);
			rv = 0;
			break;
		}
		else
		{
			out++;
			if(out >= 10)
				break;
		}

	}

	dbg_printf(1,"put end!!!! data");

    free(dbuf);
    FtpClose(nData);
    return rv;
}

//typedef struct
//{
//    char 		name[64];        //查询域名，这是一个大小在0到63之间的字符串
//    char  		type;					   //查询类型，大约有20个不同的类型
//    char		classes;				 //查询类，通常是A类既查询IP地址
//}DnsQuery;


//extern _UPDATE gUpdateInfo;

//int ftp_updata_program(const char *localfile, const char *path,
//	netbuf *nControl,int typ,int mode)
//{
////    uint8_t paraCnt=0;
//
////    uint16_t  len;
//    uint32_t  adr=BAK_ADDR;
//	uint8_t   ret=1;
///**/
//    int read_len;
//	int err_cnt = 0;
//    uint8 *dbuf;
//    netbuf *nData;
//
//    if (!FtpAccess(path, typ, mode, nControl, &nData))
//    {
//		dbg_printf(1,"call ftpaccess fila ");
//		return 0;
//    }
//
//    dbuf = malloc(FTPLIB_BUFSIZ);
//
//    gUpdateInfo.fileLen=0;
//    gUpdateInfo.fileChecksum=0;

//	osDelay(2000);
//
//    if (user_flash_erase(BAK_ADDR,BACK_SECTOR_NUM)!=1)   //擦除备份区
//    {

//    }

//    if (user_flash_erase(PARA2_ADDR,PARA2_SECTOR_NUM)!=1)    //擦除参数区2
//    {

//    }
//    else
//    {

//    }
//
//    dbuf = malloc(FTPLIB_BUFSIZ);

//    while (1)
//    {
//        read_len = FtpRead(dbuf, FTPLIB_BUFSIZ, nData);
//
//        if(read_len > 0)
//        {
//
//            user_flash_write(adr,dbuf,read_len,TYPEPROGRAM_BYTE);   //保存文件内容
//
//            adr +=read_len;
//            gUpdateInfo.fileLen +=read_len;
//            gUpdateInfo.fileChecksum += updata_programm_checksum(dbuf,read_len);  //计算文件较验
//            if (read_len !=FTPLIB_BUFSIZ) //结束
//            {
//                ret=1;
//                //break;
//            }
//            osDelay(10);
//            err_cnt = 0;
//        }
//        else
//        {
//            //
//            err_cnt++;
//            if(err_cnt > 10)
//            {
//                if(gUpdateInfo.fileLen <= 100 *1024)
//                    ret = 0;
//                else
//                    ret = 1;
//                break;
//            }
//            osDelay(10);
//        }
//    }
//
//    user_flash_write(UPDATE_FILE_CHCKSUM_ADR,(uint8*)&gUpdateInfo.fileChecksum,4,TYPEPROGRAM_BYTE); //保存文件校验
//    user_flash_write(UPDATE_FILE_LEN_ADR,(uint8*)&gUpdateInfo.fileLen,4,TYPEPROGRAM_BYTE) ;   //保存文件长度
//
//    free(dbuf);
//    return ret;
//}


//int ftp_updata_proc(char *ip,int port,char *usr,char *psword,char *path)
//{
//    int ret = 0;
//    FtpInit();
//    if(ftp_connect(ip,port,usr,psword) == 0)
//    {
//		if (conn)
//			FtpClose(conn);
//    }

//
//    FtpOptions(FTPLIB_CALLBACK, (long) NULL, conn);
//
//    ret = ftp_updata_program(NULL,path,conn,FTPLIB_FILE_READ,'I');
//
//    if(ret)
//    {
//        uint16 tmp32=0x55AA;
//        if (user_flash_write(UPDATE_FLAG_ADR,(uint8*)&tmp32,4,TYPEPROGRAM_BYTE)!= 1) //保存升级标志
//                ;
//        osDelay(3000);
//        HAL_NVIC_SystemReset();       //软件复位
//    }
//    FtpClose(conn);
//
//    return ret;
//}

int ftp_get_file(const char *localfile, const char *path,
	netbuf *nControl,int typ,int mode)
{
    int l;
	int out = 0;
	int cnt = 0;
    char *dbuf;
    //FILE *local = NULL;
    netbuf *nData;
    int rv=1;

    if (!FtpAccess(path, typ, mode, nControl, &nData))
    {
		dbg_printf(1,"call ftpaccess fila ");
		return 0;
    }

    dbuf = malloc(FTPLIB_BUFSIZ);

	while(1)
	{
		l = FtpRead(dbuf, FTPLIB_BUFSIZ, nData);

		if(l)
		{
			out = 0;
			cnt += l;
			dbg_printf(1,"rcv %d data %d len pack",l,cnt);
		}
		else
		{
			out++;
			dbg_printf(1,"err times %d",out);
			if(out > 10)
				break;

		}
	}

	dbg_printf(1,"rcv end!!!! data");

    free(dbuf);
    FtpClose(nData);
    return rv;
}
