#include <iostream> 
#include <fstream>
#include <string>
#include <memory> //shared_ptr
#include <vector> 

#include <cstddef>
#include <sys/socket.h> /*socketpair*/
#include <stdio.h> /*io*/
#include <fcntl.h>/*open*/
#include <bits/stdc++.h> 
#include <sys/ioctl.h>/*ioctl*/
#include <linux/nbd.h>/*nbd*/
#include <arpa/inet.h>/*htons*/
#include <signal.h>
#include <cstdint>
#include <csignal>
#include <atomic>

#include "drive_communicator.hpp"
#include "driver_data.hpp"
#include "storage.hpp"

using namespace hrd29;


//std::atomic<bool>ShouldDisconnectFlag(false); //0
int g_nbd_dev_to_disconnect;
IDriverCommunicator *driver ;

static int serve_nbd(int sk,IDriverCommunicator *driver,  IStorage *storage);
void SignalHandlerFather();
int SetSigaction(int sig, const struct sigaction * act);
void DisconnectNbd(int signal);


int main(int argc, char const *argv[])
{
	try
	{
			
	size_t nbd_size = 128000000;
	size_t storage_size = 128000000;//check 128 megabyte
	//char *device = "/dev/nbd0" ;
	IStorage *storage = new RAMStorage(storage_size);
 	driver = new NBDDriverCommunicator( "/dev/nbd0", nbd_size); 

	//call ndb_serve with socket, iDrivercomminocator object *base, istore *store
	//build the ndb function and understand what args i need for this function 

	puts("in main");
	int sk = driver->GetSocketFather();//TODO:CHECK if the right socket
	puts("sk ok ");
	serve_nbd(sk,driver, storage);
	puts("after serve nbd ");

	}
	
	catch(const hrd29::Exception& ex)
	{
		std::cerr<< "caught exception: "<<ex.what()<<std::endl;
	}
	catch(...)
	{
		std::cerr<< "caught unknown exception: " <<std::endl;
	}
	
 
	return 0; 
}


 //implement ndb_serve


//sk- socket_o- child
 static int serve_nbd(int sk,IDriverCommunicator *driver,  IStorage *storage) 
 {

	
	
		//call to signal Father
		puts("before signal in servernbd");
		SignalHandlerFather();
		puts("after signal in servernbd");

		//do epoll - I/O listener witout busy waiting
		//epoll with struct epoll event

		while (1)
		{
			
			std::shared_ptr<DriverData>p_data = driver->ReceiveRequest();
			printf("after reciverequest in serve_nbd");

			switch(p_data->m_action) 
			{

				case DriverData::READ:
				{
					storage->Read(p_data);
					driver->SendReply(p_data);
					
					break;
				}

				case DriverData::WRITE:
				{
					//READ ALL- check 
					read_all(sk, &(*p_data->m_buffer.begin()), p_data->m_len);

					storage->Write(p_data);
					driver->SendReply(p_data);
					
					break;
				}



				default:
				{
					puts("default\n");
					break;
				}
				return EXIT_SUCCESS;
			}
		}

		
}
	




void SignalHandlerFather()
{
  struct sigaction act; //action for signal
  act.sa_handler = DisconnectNbd;//this function would be called when a signal is recieved
  
  act.sa_flags = SA_RESTART;

  /*set signal mask currently- blocked, so signal handler wont get interrupted*/
  /*sigemptyset(&act.sa_mask)- empty signal mask,then add to mask SIGINT, SIGTERM */
  if(sigemptyset(&act.sa_mask) != 0 || sigaddset(&act.sa_mask, SIGINT) != 0 ||sigaddset(&act.sa_mask, SIGTERM) != 0) 
  {
    throw Exception("sigemptyset or sigaddset fail\n");
  }
  /*register the act- check that signals set ok\catch\throw error*/
  if (SetSigaction(SIGINT, &act) != 0 || SetSigaction(SIGTERM, &act) != 0) 
  {
   /*throw or try catch if couldnt initialize mask */
   throw Exception("SetSigaction fail\n");
  }

}


/************************SetSigaction*****************************/

/* Sets signal action like regular sigaction but is suspicious.  take to main*/
int SetSigaction(int sig, const struct sigaction * act) 
{
  struct sigaction oact;
  int r = sigaction(sig, act, &oact);
  if (r == 0 && oact.sa_handler != SIG_DFL) 
  {
    //try catch throw error
    throw Exception("oact.sa_handler fail\n");
  }
  return r;

  /*take to signal handler function*/
}

/************************SIGNAL HANDLER*****************************/

void DisconnectNbd(int signal) //should be signal handler
{
  (void)signal;
  g_nbd_dev_to_disconnect = driver->GetFD();
  
  if(g_nbd_dev_to_disconnect != -1)
  {
    //call function disconnect from comminucator
	driver->Disconnect();
  }
}



