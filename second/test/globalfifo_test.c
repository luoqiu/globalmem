
void aio_completion_handler(int signo, siginfo_t* info, void* context)
{

}

void set_io()
{
	int fd = open("/dev/globalfifo", O_RDWR);

	struct sigaction sig_act;
	struct aiocb my_aiocb;

	sigemptyset(sig_act);
	sig_act.sa_flags = SA_SIGINFO;
	sig_act.sa_sigactoin = aio_completion_handler;

	bzero((char*)&my_aiocb, sizeof(my_aiocb));

	my_aiocb.aio_fildes = fd;
	my_aiocb.aio_buf = malloc(256);


}