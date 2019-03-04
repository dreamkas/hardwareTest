#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <syslog.h>
#include <stdlib.h>
#include <stdarg.h>


int TestLCD(void)
{
 int fd;

 printf("Test LCD\n");

 fd=open("/sys/class/gpio/export",O_WRONLY);
 if (fd < 0)
 {
  printf("Error GPIO export.\n");
  return -1;
 }
 write(fd,"206",4);
 close(fd);

 fd=open("/sys/class/gpio/gpio206/direction",O_RDWR);
 if (fd < 0)
 {
  printf("Error GPIO direction.\n");
  return -1;
 }
 write(fd,"out",4);
 close(fd);

 fd=open("/sys/class/gpio/gpio206/value",O_RDWR);
 if (fd < 0)
 {
  printf("Error GPIO set value.\n");
  return -1;
 }
 printf("Turn off.\n");
 write(fd,"0",2);
 sleep(2);
 printf("Turn on.\n");
 write(fd,"1",2);
 close(fd);

 printf("Test LCD ok.\n");
 return 1;
}

int TestBeeper(void)
{
 int fd;

 printf("Test beeper.\n");
 fd=open("/sys/class/gpio/export",O_WRONLY);
 if (fd < 0)
 {
  printf("Error GPIO export.\n");
  return -1;
 }
 write(fd,"205",4);
 close(fd);

 fd=open("/sys/class/gpio/gpio205/direction",O_RDWR);
 if (fd < 0)
 {
  printf("Error GPIO direction.\n");
  return -1;
 }
 write(fd,"out",4);
 close(fd);

 fd=open("/sys/class/gpio/gpio205/value",O_RDWR);
 if (fd < 0)
 {
  printf("Error GPIO set value.\n");
  return -1;
 }
 write(fd,"1",2);
 sleep(3);
 write(fd,"0",2);
 close(fd);
 printf("Test beeper ok.\n");
 return 1;
}

int TestDrawer(void)
{
 int fd;

 printf("Test drawer.\n");
 fd=open("/sys/class/gpio/export",O_WRONLY);
 if (fd < 0)
 {
  printf("Error GPIO export.\n");
  return -1;
 }
 write(fd,"204",4);
 close(fd);

 fd=open("/sys/class/gpio/gpio204/direction",O_RDWR);
 if (fd < 0)
 {
  printf("Error GPIO direction.\n");
  return -1;
 }
 write(fd,"out",4);
 close(fd);

 fd=open("/sys/class/gpio/gpio204/value",O_RDWR);
 if (fd < 0)
 {
  printf("Error GPIO set value.\n");
  return -1;
 }
 write(fd,"1",2);
 usleep(100000);
 write(fd,"0",2);
 close(fd);
 printf("Test drawer ok.\n");
 return 1;
}

void clcCrc16(unsigned char *data, unsigned short len)
{
    unsigned char x;
    unsigned short tmp;

    int crc16 = 0xFFFF;
    unsigned char *ptr = data + 1;
    tmp = len - 3;
    while (tmp--)
    {
        x = crc16 >> 8 ^ *ptr++;
        x ^= x >> 4;
        crc16 = (crc16 << 8) ^ ((unsigned short) (x << 12)) ^ ((unsigned short) (x << 5)) ^ ((unsigned short) x);
    }
    *(data + len - 2) = crc16 & 0xff;
    *(data + len - 1) = ((crc16 & 0xff00) >> 8);
}

int TestFN(void)
{
 int i,cnt,fd;
 unsigned char buff[100];
 unsigned char getst[6] = {0x04, 0x01, 0x00, 0x20, 0x00, 0x00};
 clcCrc16(getst, sizeof(getst)); 
 
 printf("Test FN.\n");
 fd=open("/dev/i2c-0",O_RDWR);
 if (fd < 0)
 {
  printf("Error I2C open.\n");
  return -1;
 }
 i=2;
 if (ioctl(fd,I2C_SLAVE,i) < 0)
 {
  printf("Error slave select.\n");
  return -1;
 }
 if (ioctl(fd,I2C_TIMEOUT,100000) < 0)
 {
  printf("Error timeout.\n");
  return -1;
 }
 if (ioctl(fd,I2C_RETRIES,2000000) < 0)
 {
  printf("Error retries.\n");
  return -1;
 }
 memcpy(buff,getst,sizeof(getst));
 for (cnt=0; cnt < 30; cnt++)
 {
  if ((i=write(fd,buff,sizeof(getst))) == sizeof(getst)) break;
 }
 if (i != sizeof(getst))
 {
  printf("Error write. Writed=%d\n",i);
  return -1;
 }
 usleep(100000);
 if ((i=read(fd,buff,4)) != 4)
 if (i < 4)
 {

  printf("Error read. Readed=%d\n",i);
  return -1;
 }
 cnt=buff[1];
 if ((i=read(fd,&buff[4],cnt)) != cnt)
 if (i < cnt)
 {
  printf("Error read. Readed=%d\n",i);
  return -1;
 }

 for (i=0; i < 4+cnt; i++) printf("%02x ",buff[i]);
 printf("\n");
 close(fd);

 printf("Test FN ok.\n");
 return 1;
}

int mygetch(void)
{
  int ch;
  struct termios oldt, newt;

  tcgetattr ( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr ( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr ( STDIN_FILENO, TCSANOW, &oldt );

  return ch;
}


int SetRes(int on)
{
 int fd;

 fd=open("/sys/class/gpio/gpio43/value",O_RDWR);
 if (fd < 0)
 {
  printf("Error GPIO set value.\n");
  return -1;
 }
 if (on) write(fd,"1",2);
 else write(fd,"0",2);
 close(fd);
 return 1;
}

int CheckReady(void)
{
 int fd,res,rd;

 fd=open("/sys/class/gpio/gpio42/value",O_RDONLY);
 if (fd < 0)
 {
  printf("Error GPIO set value.\n");
  return -1;
 }
 res=read(fd,&rd,1);
 if (res == 1) res=rd;
 close(fd);
 return res;
}

int SPI_Write(int fds, unsigned char c)
{
 int res;
 struct timeval stv,tv;

 gettimeofday(&stv,NULL);
 stv.tv_sec+=2; // 2 secs
 while (1)
 {
  if ((res=CheckReady()) < 0) return -1;
  if (res) break;
  gettimeofday(&tv,NULL);
  if (tv.tv_sec > stv.tv_sec)
  {
   printf("Error SPI write.\n");
   return -1;
  }
 }
 if ((res=write(fds,&c,1)) != 1)
 {
  printf("Error SPI write.\n");
  return -1;
 }
 usleep(3);
 return 1;
}

int SPI_Read(int fds, unsigned char *c)
{
 int res,i;
 struct timeval stv,tv;

 gettimeofday(&stv,NULL);
 stv.tv_sec+=2; // 2 secs
 while (1)
 {
  if ((res=CheckReady()) < 0) return -1;
  if (res) break;
  gettimeofday(&tv,NULL);
  if (tv.tv_sec > stv.tv_sec)
  {
   printf("Error SPI read.\n");
   return -1;
  }
 }
 i=0;
 res=read(fds,&i,1);
 *c=(unsigned char)i;
 if (res != 1)
 {
  printf("Error SPI read.\n");
  return -1;
 }
 usleep(3);
 return 1;
}

int SPI_Reset(int fds)
{
 int res;
 unsigned char c;

 res=SPI_Write(fds,0xA1);
 if (res)
 {
  res=SPI_Read(fds,&c);
  if (c == 0x5A) return 1;
 }
 SetRes(0);
 usleep(100000);
 SetRes(1);
 usleep(500000);
 if (SPI_Write(fds,0xA1) < 0) return -1;
 if (SPI_Read(fds,&c) < 0) return -1;
 if (c != 0x5A)
 {
  printf("Error SPI reset.\n");
  return -1;
 }
 return 1;
}

int TestSPI(void)
{
 int i,count,fd,fds;
 unsigned char c;

 printf("Test SPI with FM.\n");
 fds=open("/dev/spidev0.0",O_RDWR);
 if (fds < 0)
 {
  printf("Error SPI device open.\n");
  return -1;
 }

 fd=open("/sys/class/gpio/export",O_WRONLY);
 if (fd < 0)
 {
  printf("Error GPIO export.\n");
  return -1;
 }
 write(fd,"42",3);
 close(fd);

 fd=open("/sys/class/gpio/gpio42/direction",O_RDWR);
 if (fd < 0)
 {
  printf("Error GPIO direction.\n");
  return -1;
 }
 write(fd,"in",3);
 close(fd);

 fd=open("/sys/class/gpio/export",O_WRONLY);
 if (fd < 0)
 {
  printf("Error GPIO export.\n");
  return -1;
 }
 write(fd,"43",3);
 close(fd);

 fd=open("/sys/class/gpio/gpio43/direction",O_RDWR);
 if (fd < 0)
 {
  printf("Error GPIO direction.\n");
  return -1;
 }
 write(fd,"out",4);
 close(fd);
 fd=open("/sys/class/gpio/gpio43/value",O_RDWR);
 if (fd < 0)
 {
  printf("Error GPIO value.\n");
  return -1;
 }
 write(fd,"2",2);
 close(fd);
 count=0x100;
 if (SPI_Reset(fds) < 0) return -1;
 for (i=0; i < count; i++)
 {
  if (SPI_Write(fds,0x32) < 0) return -1;
  if (SPI_Write(fds,i) < 0) return -1;
  if (SPI_Write(fds,i >> 8) < 0) return -1;
  if (SPI_Write(fds,i >> 16) < 0) return -1;
  if (SPI_Write(fds,1) < 0) return -1;

  if ((i & 0x0F) == 0) printf("\nOffs:%04x  ",i);
  if (SPI_Read(fds,&c) < 0) return -1;
  printf("%02x ",c);
 }
 printf("\n");
 printf("Test SPI ok.\n");
 return 1;
}

#define ReadID_STR "AT+CCID\r"
int TestModem(void)
{
 int i,count,fd;
 char tmp,buf[200],*p,*p1;
 struct termios tm;

 printf("Test modem.\n");

 fd=open("/sys/class/gpio/export",O_WRONLY);
 if (fd < 0)
 {
  printf("Error GPIO export.\n");
  return -1;
 }
 write(fd,"96",3);
 close(fd);

 fd=open("/sys/class/gpio/gpio96/direction",O_RDWR);
 if (fd < 0)
 {
  printf("Error GPIO direction.\n");
  return -1;
 }
 write(fd,"out",4);
 close(fd);

 fd=open("/sys/class/gpio/gpio96/value",O_RDWR);
 if (fd < 0)
 {
  printf("Error GPIO value.\n");
  return -1;
 }
 printf("Resetting modem...\n");
 write(fd,"1",2);
 sleep(1);
 write(fd,"0",2);
 close(fd);
 sleep(3);
 printf("Reset ok.\n");

 fd=open("/dev/ttyS6",O_RDWR);
 if (fd < 0)
 {
  printf("Error open ttyS6.\n");
  return -1;
 }

 tm.c_cflag=0;
 cfmakeraw(&tm);
 cfsetispeed(&tm,B115200);
 cfsetospeed(&tm,B115200);

 tm.c_cflag=tm.c_cflag | CLOCAL | CREAD; // local mode+enable receiver
 tm.c_cflag=tm.c_cflag & ~(CSTOPB | PARENB | PARODD | CRTSCTS);
 tm.c_cflag=tm.c_cflag | CS8;
 tm.c_cc[VMIN]=0;
 tm.c_cc[VTIME]=1;
 tcsetattr(fd,TCSANOW,&tm);
 tcflush(fd,TCIFLUSH);
 strcpy(buf,ReadID_STR);
 write(fd,buf,strlen(buf));

 count=0;
 for (i=0; i < 150; i++)
 {
  if (read(fd,&tmp,1) == 1) buf[count++]=tmp;
  else break;
 }
 close(fd);

 buf[count]=0;
 p=strstr(buf,ReadID_STR);
 if (p == 0) return -1;
 if (strlen(p) < strlen(ReadID_STR)+4) return -1;
 p+=strlen(ReadID_STR)+2;
 p1=strchr(p,'\r');
 if (p1 == 0) return -1;
 *p1=0;
 printf("ID=%s\n",p);
 p=strstr(p1+1,"OK");
 if (p == 0) return -1;
 printf("Test modem ok.\n");
 return 1;
}

int main (int argc, char *argv[]) 
{
 int res=0,
     mode=0;

 if (argc > 1)
 {
  mode=strtol(argv[1], (char **)NULL, 10);
 }
 switch (mode)
 {
  case 1:  res=TestBeeper();
           break;

  case 2:  
       for (int i = 0; i < 5000; ++i)
	   {
	   	printf("Try count:\t%d\n", i);
	   	res=TestFN();
	   }
	   break;

  case 3:  res=TestDrawer();
           break;

  case 4:  res=TestSPI();
           break;

  case 5:  res=TestModem();
           break;


case 100:  res=TestLCD();
           if (res != 1) break;
           res=TestBeeper();
           if (res != 1) break;
           res=TestFN();
           if (res != 1) break;
           res=TestDrawer();
           if (res != 1) break;
           res=TestSPI();
           break;

  default: res=TestLCD();
           break;
 }
 res=(res == 1) ? 0 : 1;
 return res;
}
