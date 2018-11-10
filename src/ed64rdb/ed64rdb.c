#include <config.h>
#include <swap.h>

#ifdef _WIN32
# include <Winsock2.h>
# include <ws2tcpip.h>
# define ioctl        ioctlsocket
# define close        closesocket
# define SEAGAIN      WSAEWOULDBLOCK
# define SEWOULDBLOCK WSAEWOULDBLOCK
typedef int socklen_t;
#else
# include <fcntl.h>
# include <netdb.h>
# include <sys/ioctl.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <termios.h>
# include <unistd.h>
# if defined(HAVE_PTY_H)
#  include <pty.h>
# elif defined(HAVE_UTIL_H)
#  include <util.h>
# elif defined(HAVE_LIBUTIL_H)
#  include <libutil.h>
# endif
# define SEAGAIN      EAGAIN
# define SEWOULDBLOCK EWOULDBLOCK
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

union fifo_pkt
{
  char        mem[512];
  struct
  {
    uint16_t  size;
    char      data[510];
  };
};

#ifdef _WIN32
static HANDLE hComm     = INVALID_HANDLE_VALUE;
static SOCKET sock      = INVALID_SOCKET;
static SOCKET cl        = INVALID_SOCKET;
#else
static int    sv        = -1;
static char  *pty_link  = NULL;
static int    pty_s     = -1;
static int    sock      = -1;
static int    cl        = -1;
#endif
static char  *cl_addr   = NULL;

static void stop_sig(int sig)
{
  static _Bool fatal;
  if (!fatal) {
    fatal = 1;
#ifdef _WIN32
    if (hComm != INVALID_HANDLE_VALUE)
      CloseHandle(hComm);
    if (cl != INVALID_SOCKET)
      close(cl);
    if (sock != INVALID_SOCKET)
      close(sock);
#else
    if (sv != -1)
      close(sv);
    if (pty_link)
      unlink(pty_link);
    if (pty_s != -1)
      close(pty_s);
    if (cl != -1)
      close(cl);
    if (sock != -1)
      close(sock);
#endif
    if (cl_addr)
      free(cl_addr);
  }
  signal(sig, SIG_DFL);
  raise(sig);
}

#ifdef _WIN32
static int makeraw(HANDLE hComm)
{
  DCB dcb;
  dcb.DCBlength = sizeof(dcb);
  if (!GetCommState(hComm, &dcb))
    return -1;
  dcb.fParity = FALSE;
  dcb.fOutX = FALSE;
  dcb.fInX = FALSE;
  dcb.fErrorChar = FALSE;
  dcb.fNull = FALSE;
  dcb.ByteSize = 8;
  if (!SetCommState(hComm, &dcb))
    return -1;
  return 0;
}
#else
static int makeraw(int fd)
{
  struct termios t;
  if (tcgetattr(fd, &t) == -1)
    return -1;
  t.c_iflag = 0;
  t.c_oflag = 0;
  t.c_lflag = 0;
  t.c_cflag = CS8;
  t.c_cc[VMIN] = 0;
  t.c_cc[VTIME] = 0;
  if (tcsetattr(fd, TCSAFLUSH, &t) == -1)
    return -1;
  return 0;
}
#endif

static int sockerr(void)
{
#ifdef _WIN32
  return WSAGetLastError();
#else
  return errno;
#endif
}

static void die(const char *note, int errtype)
{
#ifdef _WIN32
  char s[1024];
#endif
  switch (errtype) {
    case 0: /* generic error */
      fprintf(stderr, "%s\n", note);
      break;
    case 1: /* posix error */
      fprintf(stderr, "%s: %s\n", note, strerror(errno));
      break;
    case 2: /* getaddrinfo error */
      fprintf(stderr, "%s: %s\n", note, gai_strerror(errno));
      break;
    case 3: /* windows error */
#ifdef _WIN32
      if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 0,
                        s, sizeof(s), NULL))
      {
        fprintf(stderr, "%s: %s\n", note, s);
      }
#endif
      break;
    case 4: /* socket error */
#ifdef _WIN32
      if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), 0,
                        s, sizeof(s), NULL))
      {
        fprintf(stderr, "%s: %s\n", note, s);
      }
#else
      fprintf(stderr, "%s: %s\n", note, strerror(errno));
#endif
      break;
  }
  raise(SIGTERM);
}

int main(int argc, char *argv[])
{
  if (argc != 3) {
    fprintf(stderr, "%s",
            "ed64rdb - ed64 remote debugging packet translator\n"
            "usage: `ed64rdb <usb-tty> "
            "< pty:<pty-link> | tcp:[host]:<port> | udp:[host]:<port> >`\n");
    return 0;
  }

  int cl_if = 0;
  if (strncmp(argv[2], "pty:", 4) == 0)
    cl_if = 1;
  else if (strncmp(argv[2], "tcp:", 4) == 0)
    cl_if = 2;
  else if (strncmp(argv[2], "udp:", 4) == 0)
    cl_if = 3;
  else
    die("unknown client interface", 0);
  cl_addr = malloc(strlen(&argv[2][4]) + 1);
  if (!cl_addr) {
    errno = ENOMEM;
    die("malloc()", 1);
  }
  strcpy(cl_addr, &argv[2][4]);

#ifndef _WIN32
  signal(SIGPIPE, stop_sig);
#endif
  signal(SIGINT, stop_sig);
  signal(SIGTERM, stop_sig);

#ifdef _WIN32
  HANDLE hComm = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE, 0, NULL,
                            OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
  if (hComm == INVALID_HANDLE_VALUE)
    die("CreateFile(usb-tty)", 3);
  if (makeraw(hComm))
    die("makeraw(sv)", 3);
#else
  sv = open(argv[1], O_RDWR | O_NONBLOCK);
  if (sv == -1)
    die("open(usb-tty)", 1);
  if (makeraw(sv))
    die("makeraw(sv)", 1);
#endif

  struct sockaddr cl_sockaddr;
  socklen_t cl_socklen;
  cl_socklen = sizeof(cl_sockaddr);
  memset(&cl_sockaddr, 0, cl_socklen);

  if (cl_if == 1) {
#ifdef _WIN32
    die("this platform does not support pseudoterminals", 0);
#else
    char cl_name[1024];
    if (openpty(&cl, &pty_s, cl_name, NULL, NULL))
      die("openpty()", 1);
    if (makeraw(cl))
      die("makeraw(cl)", 1);

    unlink(cl_addr);
    if (symlink(cl_name, cl_addr))
      die("symlink(pty_link)", 1);
    pty_link = cl_addr;

    fprintf(stderr, "RDB PTY opened on %s, linked at %s\n", cl_name, cl_addr);
#endif
  }
  else {
    /* parse address */
    char *host = cl_addr;
    char *port = strchr(host, ':');
    if (!port)
      die("no port specified", 0);
    if (host == port)
      host = NULL;
    *port++ = 0;
    /* resolve address */
#ifdef _WIN32
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2, 2), &wsadata))
      die("WSAStartup()", 4);
#endif
    struct addrinfo hints;
    struct addrinfo *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = (cl_if == 2 ? SOCK_STREAM : SOCK_DGRAM);
    hints.ai_flags = 0;
    hints.ai_protocol = 0;
    errno = getaddrinfo(host, port, &hints, &res);
    if (errno) {
#ifdef _WIN32
      die("getaddrinfo()", 2);
#else
      if (errno == EAI_SYSTEM)
        die("getaddrinfo()", 1);
      else
        die("getaddrinfo()", 2);
#endif
    }
    /* create socket and bind */
    for (struct addrinfo *rp = res; rp; rp = rp->ai_next) {
      sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (sock == -1)
        continue;
      if (bind(sock, rp->ai_addr, rp->ai_addrlen) == 0)
        break;
      close(sock);
      sock = -1;
    }
    if (res)
      freeaddrinfo(res);
    if (sock == -1)
      die("bind()", 4);
    /* wait for connection (tcp) */
    if (cl_if == 2) {
      if (listen(sock, 1))
        die("listen()", 4);
      fprintf(stderr, "RDB listening on %s:%s\n",
              host ? host : cl_addr, port);
      cl = accept(sock, &cl_sockaddr, &cl_socklen);
      if (cl == -1)
        die("accept()", 4);
      close(sock);
      fprintf(stderr, "RDB connected\n");
    }
    else {
      cl = sock;
      fprintf(stderr, "RDB bound to %s:%s\n",
              host ? host : cl_addr, port);
    }
    sock = -1;
  }

  /* disable blocking on client */
  u_long nbio = 1;
  if (ioctl(cl, FIONBIO, &nbio))
    die("ioctl(cl)", 4);

  union fifo_pkt sv_pkt;
  union fifo_pkt cl_pkt;

#ifdef _WIN32
  OVERLAPPED OverlappedRead;
  memset(&OverlappedRead, 0, sizeof(OverlappedRead));
  BOOL ReadPending = 0;
#endif

  while (1) {
    int rd = 0;
    /* check for server packet */
#ifdef _WIN32
    if (!ReadPending) {
      if (!ReadFile(hComm, &sv_pkt, sizeof(sv_pkt), NULL, &OverlappedRead)
          && GetLastError() != ERROR_IO_PENDING)
      {
        die("ReadFile(sv)", 3);
      }
      ReadPending = 1;
    }
    DWORD BytesRead;
    if (GetOverlappedResult(hComm, &OverlappedRead, &BytesRead, FALSE)) {
      ReadPending = 0;
      rd = BytesRead;
    }
    else if (GetLastError() != ERROR_IO_INCOMPLETE)
      die("GetOverlappedResult(sv)", 3);
#else
    rd = read(sv, &sv_pkt.mem[0], sizeof(sv_pkt.mem));
    if (rd == -1)
      die("read(sv)", 1);
    else if (rd != 0) {
      int total = rd;
      while (total != sizeof(sv_pkt.mem)) {
        rd = read(sv, &sv_pkt.mem[total], sizeof(sv_pkt.mem) - total);
        if (rd == -1)
          die("read(sv)", 1);
        total += rd;
      }
      rd = total;
    }
#endif
    if (rd != 0) {
      /* forward to client */
      int size = btoh16(sv_pkt.size);
      if (size > sizeof(sv_pkt.data)) {
        fprintf(stderr, "packet too big (%i), truncated\n", size);
        size = sizeof(sv_pkt.data);
      }
      int wr;
#ifndef _WIN32
      if (cl_if == 1)
        wr = write(cl, sv_pkt.data, size);
      else
        wr = sendto(cl, sv_pkt.data, size, 0, &cl_sockaddr, cl_socklen);
#else
      wr = sendto(cl, sv_pkt.data, size, 0, &cl_sockaddr, cl_socklen);
#endif
      if (wr == -1)
        die("sendto(cl)", 4);
      printf("RDB SEND  %.*s\n", size, sv_pkt.data);
    }
    /* check for client packet */
#ifndef _WIN32
    if (cl_if == 1)
      rd = read(cl, cl_pkt.data, sizeof(cl_pkt.data));
    else {
      rd = recvfrom(cl, cl_pkt.data, sizeof(cl_pkt.data), 0,
                    &cl_sockaddr, &cl_socklen);
    }
#else
    rd = recvfrom(cl, cl_pkt.data, sizeof(cl_pkt.data), 0,
                  &cl_sockaddr, &cl_socklen);
#endif
    if (rd == -1) {
      if (sockerr() == SEAGAIN || sockerr() == SEWOULDBLOCK)
        rd = 0;
      else
        die("recvfrom(cl)", 4);
    }
    if (rd != 0) {
      /* forward to server */
      cl_pkt.size = htob16(rd);
#ifdef _WIN32
      OVERLAPPED OverlappedWrite;
      memset(&OverlappedWrite, 0, sizeof(OverlappedWrite));
      DWORD BytesWritten;
      if (!WriteFile(hComm, &cl_pkt, sizeof(cl_pkt), NULL, &OverlappedWrite)
          && GetLastError() != ERROR_IO_PENDING)
      {
        die("WriteFile(sv)", 3);
      }
      if (!GetOverlappedResult(hComm, &OverlappedWrite, &BytesWritten, TRUE))
        die("GetOverlappedResult(sv)", 3);
#else
      if (write(sv, &cl_pkt, sizeof(cl_pkt)) == -1)
        die("write(sv)", 1);
#endif
      printf("RDB RECV  %.*s\n", rd, cl_pkt.data);
    }
  }

  return 0;
}
