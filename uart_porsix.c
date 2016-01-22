#include <termios.h>    
#include <stdio.h>      
#include <unistd.h>      
#include <fcntl.h>      
#include <sys/signal.h>     
#include <sys/types.h>      
      
#define BAUDRATE B19200      
#define MODEMDEVICE "/dev/ttyS1"      

#define _POSIX_SOURCE 1   /* POSIX compliant source */      
#define FALSE 0      
#define TRUE 1      

volatile int STOP=FALSE;
void signal_handler_IO (int status);   /* definition of signal handler */  
                                    // �����źŴ������
int wait_flag=TRUE;                   /* TRUE while no signal received */     
                                   // TRUE ����û���ܵ��źţ����ڵȴ���   

void main()   {        
	int fd,c, res;   
	struct termios oldtio,newtio;  
	struct sigaction saio;         
	/* definition of signal action */       
	// �����źŴ���Ľṹ 

	char buf[255];        

	/* open the device to be non-blocking (read will return immediatly) */     
	// ���÷�����ģʽ���豸 read �������̷��أ���������    
   fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);    
   if (fd <0) {perror(MODEMDEVICE); exit(-1); }        

	/* install the signal handler before making the device asynchronous */    
   // �ڽ����豸�첽����ǰ����װ�źŴ������    
	saio.sa_handler = signal_handler_IO;     
	saio.sa_mask = 0;  
	saio.sa_flags = 0;     
	saio.sa_restorer = NULL;    
	sigaction(SIGIO,&saio,NULL);   

	/* allow the process to receive SIGIO */ 
	// ������̽��� SIGIO �ź�      
	fcntl(fd, F_SETOWN, getpid());    

	   /* Make the file descriptor asynchronous (the manual page says only  
	O_APPEND and O_NONBLOCK, will work with F_SETFL...) */   
	// ���ô��ڵ��ļ�������Ϊ�첽��man��˵��ֻ�� O_APPEND �� O_NONBLOCK ����ʹ��F_SETFL
	fcntl(fd, F_SETFL, FASYNC);        
	tcgetattr(fd,&oldtio); /* save current port settings */   

   /* set new port settings for canonical input processing */   
// �����µĴ���Ϊ��׼����ģʽ      
	newtio.c_cflag = BAUDRATE | CS8;
    newtio.c_cflag |= PARENB;
    newtio.c_cflag &= ~PARODD;
    newtio.c_cflag |=INPCK;
    newtio.c_cflag &= ~CSTOPB;
    newtio.c_iflag = 0;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0; 
	newtio.c_cc[VMIN]=0;    
	newtio.c_cc[VTIME]=0; 
	/**�����շ�����**/
	newtio.c_cc[VINTR] = 0; /* Ctrl-c */
    newtio.c_cc[VQUIT] = 0; /* Ctrl-\ */
    newtio.c_cc[VERASE] = 0; /* del */
    newtio.c_cc[VKILL] = 0; /* @ */
    newtio.c_cc[VEOF] = 4; /* Ctrl-d */
    newtio.c_cc[VSWTC] = 0; /* '\0' */
    newtio.c_cc[VSTART] = 0; /* Ctrl-q */
    newtio.c_cc[VSTOP] = 0; /* Ctrl-s */
    newtio.c_cc[VSUSP] = 0; /* Ctrl-z */
    newtio.c_cc[VEOL] = 0; /* '\0' */
    newtio.c_cc[VREPRINT] = 0; /* Ctrl-r */
    newtio.c_cc[VDISCARD] = 0; /* Ctrl-u */
    newtio.c_cc[VWERASE] = 0; /* Ctrl-w */
    newtio.c_cc[VLNEXT] = 0; /* Ctrl-v */
    newtio.c_cc[VEOL2] = 0; /* '\0' */
	tcflush(fd, TCIFLUSH);     
	tcsetattr(fd,TCSANOW,&newtio);     

	/* loop while waiting for input. normally we would do something      
	useful here ѭ���ȴ����룬ͨ�����ǻ���������Щ���������� */ 
	while (STOP==FALSE) {       
	 printf(".\n");usleep(100000);        

	 /* after receiving SIGIO, wait_flag = FALSE, input is availableand can be read */
	 // ���յ� SIGIO �źź�wait_flag = FALSE, ��ʾ��������������Զ�ȡ�� 
	 if (wait_flag==FALSE) {         
		res = read(fd,buf,255);     
		buf[res]=0;        
		printf(":%s:%d\n", buf, res);  
		if (res==1) STOP=TRUE; /* stop loop if only a CR was input */    
		wait_flag = TRUE;      /* wait for new input �ȴ��µ�����*/   
		}      
	 }       

	/* restore old port settings */     
	tcsetattr(fd,TCSANOW,&oldtio);  
}            

/***************************************************************************    
* signal handler. sets wait_flag to FALSE, to indicate above loop that    *
* characters have been received.                                          * 
***************************************************************************/   

// �źŴ����������� wait_flag Ϊ FALSE, �Ը�֪�����ѭ�����������յ��ַ���  
void signal_handler_IO (int status)   {   
printf("received SIGIO signal.\n");     
wait_flag = FALSE;  
}