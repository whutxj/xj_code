
#ifndef TASK_FTP_H
#define TASK_FTP_H

/*	tfp for lwip free rtos and ucos move by peatree 2015.6.17 
*	get file test ok
*	I think is easy to put file and other !!~~
*/

#include <limits.h>
//#include <inttypes.h>
#define  GLOBALREF  extern
/* FtpAccess() type codes */
#define FTPLIB_DIR 1
#define FTPLIB_DIR_VERBOSE 2
#define FTPLIB_FILE_READ 3
#define FTPLIB_FILE_WRITE 4

/* FtpAccess() mode codes */
#define FTPLIB_ASCII 'A'		//asc
#define FTPLIB_IMAGE 'I'		//bin file
#define FTPLIB_TEXT FTPLIB_ASCII
#define FTPLIB_BINARY FTPLIB_IMAGE

/* connection modes */
#define FTPLIB_PASSIVE 1
#define FTPLIB_PORT 2

/* connection option names */
#define FTPLIB_CONNMODE 1
#define FTPLIB_CALLBACK 2
#define FTPLIB_IDLETIME 3
#define FTPLIB_CALLBACKARG 4
#define FTPLIB_CALLBACKBYTES 5

//#define dbg_print   do{}while(0);

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__UINT64_MAX)
typedef uint64_t fsz_t;
#else
typedef unsigned int fsz_t;
#endif
	
typedef struct
{
	char ftpuser[20];    // 用户名
	char ftppsw[20];     // 密码
	char ftppath[30];    // 文件路径
	char ftpIp[20];      // 服务器IP
	int  ftpport;        // 服务器端口
}FTP_PARA_INFO;

typedef struct NetBuf netbuf;
typedef int (*FtpCallback)(netbuf *nControl, fsz_t xfered, void *arg);

typedef struct FtpCallbackOptions {
    FtpCallback cbFunc;			/* function to call */
    void *cbArg;				/* argument to pass to function */
    unsigned int bytesXferred;	/* callback if this number of bytes transferred */
    unsigned int idleTime;		/* callback if this many milliseconds have elapsed */
} FtpCallbackOptions;

GLOBALREF int ftplib_debug;
GLOBALREF void FtpInit(void);
GLOBALREF char *FtpLastResponse(netbuf *nControl);
GLOBALREF int FtpConnect(const char *host, int port,netbuf **nControl);
GLOBALREF int FtpOptions(int opt, long val, netbuf *nControl);
GLOBALREF int FtpSetCallback(const FtpCallbackOptions *opt, netbuf *nControl);
GLOBALREF int FtpClearCallback(netbuf *nControl);
GLOBALREF int FtpLogin(const char *user, const char *pass, netbuf *nControl);
GLOBALREF int FtpAccess(const char *path, int typ, int mode, netbuf *nControl,
    netbuf **nData);
GLOBALREF int FtpRead(void *buf, int max, netbuf *nData);
GLOBALREF int FtpWrite(const void *buf, int len, netbuf *nData);
GLOBALREF int FtpClose(netbuf *nData);
GLOBALREF int FtpSite(const char *cmd, netbuf *nControl);
GLOBALREF int FtpSysType(char *buf, int max, netbuf *nControl);
GLOBALREF int FtpMkdir(const char *path, netbuf *nControl);
GLOBALREF int FtpChdir(const char *path, netbuf *nControl);
GLOBALREF int FtpCDUp(netbuf *nControl);
GLOBALREF int FtpRmdir(const char *path, netbuf *nControl);
GLOBALREF int FtpPwd(char *path, int max, netbuf *nControl);
GLOBALREF int FtpNlst(const char *output, const char *path, netbuf *nControl);
GLOBALREF int FtpDir(const char *output, const char *path, netbuf *nControl);
GLOBALREF int FtpSize(const char *path, unsigned int *size, char mode, netbuf *nControl);
#if defined(__UINT64_MAX)
GLOBALREF int FtpSizeLong(const char *path, fsz_t *size, char mode, netbuf *nControl);
#endif
GLOBALREF int FtpModDate(const char *path, char *dt, int max, netbuf *nControl);
GLOBALREF int FtpGet(const char *output, const char *path, char mode,
	netbuf *nControl);
GLOBALREF int FtpPut(const char *input, const char *path, char mode,
	netbuf *nControl);
GLOBALREF int FtpRename(const char *src, const char *dst, netbuf *nControl);
GLOBALREF int FtpDelete(const char *fnm, netbuf *nControl);
GLOBALREF void FtpQuit(netbuf *nControl);

int ftp_connect(const char *fip,int fport,const char *fuser,const char *fpsw);
int connect_serv(char *host,int port);

void ftp_Update(void const *pdata);

#ifdef __cplusplus
};
#endif

#endif /* __FTPLIB_H */
