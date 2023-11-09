
#include <iostream> 
#include <fstream>
#include <string>
#include <memory> //shared_ptr
#include <vector> 
#include "drive_communicator.hpp"
#include "driver_data.hpp"
#include <cstddef>
#include <sys/socket.h> /*socketpair*/
#include <stdio.h> /*io*/
#include <fcntl.h>/*open*/
#include <bits/stdc++.h> 
#include <thread>
#include <pthread.h> /*pthread_sigmask*/
#include <sys/ioctl.h>/*ioctl*/
#include <linux/nbd.h>/*nbd*/
#include <arpa/inet.h>/*htons*/
#include <signal.h>
#include <cstdint>


//int nbd_dev_to_disconnect = -1; /*disconnect flag*/ //atomic

namespace hrd29
{
    /*this abstract class is used for driver polymorphisem 
    i do not know which driver the user ill use(NBD\driver\else)
    must of the functions are pure virtual- there is no need for implementation 
    except the GetFD()   
    */


/*for knowing if the computer system is big or little*/
  #ifdef WORDS_BIGENDIAN
  u_int64_t ntohll(u_int64_t a) {
    return a;
  }
  #else
  u_int64_t ntohll(u_int64_t a) {
    u_int32_t lo = a & 0xffffffff;
    u_int32_t hi = a >> 32U;
    lo = ntohl(lo);
    hi = ntohl(hi);
    return ((u_int64_t) lo) << 32U | hi;
  }
  #endif
  #define htonll ntohll


/**********************************************************NBDDriverCommunicator class*********************************************/

/*both constructor should initialize int socket_fd, and threar *childThread*/

NBDDriverCommunicator::NBDDriverCommunicator(const std::string& device_path, size_t nbd_size):m_device_path(device_path),m_nbd_size(nbd_size)
{
 
   puts("enter Ctor NBD\n");
  int sp[2];
  //int nbd, sk, err, flags;

  if(-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, sp))
  {
    throw Exception("socketpair fail\n");
  }

  nbd = open(device_path.c_str(), O_RDWR);/*dev/nbd0*/
  if(-1 == nbd)
  {
    throw Exception("open fail\n");
  }
  
  //parent- process in loop do the request then close the child and exit
  
  socket_fd_0 = sp[0];
  //move to thread
  /*if(-1 == ioctl(nbd, NBD_SET_SIZE, nbd_size))
  {
    throw Exception("ioctl - NBD_SET_SIZE fail\n");
  }*/

  //thread function child is initializing ioctl

  /*if(-1 == close(sp[0]))
  {
    throw Exception("close fail\n");
  }*/
  socket_fd_1 = sp[1]; /*sp[1] is  sockect was sk*/
   /*clear socket*/
   /*move to thread*/
  /*if(-1 == ioctl(nbd, NBD_CLEAR_SOCK))
  {
    throw Exception("ioctl - NBD_CLEAR_SOCK fail\n");
  }*/
  
  child_Thread = std::thread(&ThreadFunc,nbd_size, nbd, socket_fd_1);
  child_Thread.detach();/*not sure if here or in disconnect*/
  //close(socket_fd_1); 
   puts("finish Ctor NBD\n");
 
}

//second constructor will call first constructor with initialization list
NBDDriverCommunicator::NBDDriverCommunicator(const std::string& device_path, size_t block_size,size_t num_blocks)
:NBDDriverCommunicator(device_path, num_blocks*block_size)
{}


NBDDriverCommunicator:: ~NBDDriverCommunicator() /*call disconnect!*/
{
  NBDDriverCommunicator::Disconnect();
  close(socket_fd_0);
  close(socket_fd_1);
}

int  NBDDriverCommunicator::GetFD() const
{
  return nbd;
    
} 

std::shared_ptr<DriverData>NBDDriverCommunicator::ReceiveRequest() 
{
//create make share in size of data
 //shared_ptr<DriverData> sh_ptr = make_shared<DriverData>();
  
  puts("enter ReceiveRequest\n");
  
  struct nbd_request request;
  u_int32_t len;
  ssize_t bytes_read = sizeof(request);
  std::shared_ptr<DriverData> data = NULL;
 puts("ReceiveRequest 174\n");
 if(bytes_read ==read(socket_fd_0, &request, sizeof(request)))//0 <=
  {
    //throw Exception("read fail 1\n");
    
    puts("ReceiveRequest 175\n");
    len = ntohl(request.len);
    assert(request.magic == htonl(NBD_REQUEST_MAGIC));
    puts("ReceiveRequest 178\n");
    //requestData = std::make_shared<DriverData>(len);
    puts("ReceiveRequest 180\n");

    data = std::make_shared<DriverData>(len);
    puts("ReceiveRequest 181\n");
    memcpy(&(data->m_handle), request.handle, sizeof(request.handle));
    puts("ReceiveRequest 182\n");
    data->m_action = static_cast<DriverData::Action>(ntohl(request.type));
    std::cout <<"data action is"<< data->m_action <<std::endl;
  //data->m_len = static_cast<size_t>(ntohl(request.len));
    data->m_offset = ntohll(request.from);
    
    if(DriverData::Action::WRITE == data->m_action)
    {
      puts("ReceiveRequest 184\n");
      read_all(socket_fd_0, data->m_buffer.data(), len);//len
       
    }
puts("ReceiveRequest 183\n");
    return data;
   
  }

  puts("yara\n");
 throw Exception("failed to read - ReceiveRequest function\n");


}


void NBDDriverCommunicator::SendReply(std::shared_ptr<DriverData> data_)
{
  //struct nbd_request request;
  struct nbd_reply reply;
  

  reply.magic = htonl(NBD_REPLY_MAGIC);
  reply.error = htonl(0);
  memcpy(reply.handle, reinterpret_cast<char*>(&data_->m_handle), sizeof(reply.handle));
  reply.error = htonl(0);

  if(0!=write_all(socket_fd_0, (char*)&reply, sizeof(struct nbd_reply)))
  {
    throw Exception("write_all fail\n");

  }

    //add handling the buffer for message
  //using "get" shared ptr built in function

  if(DriverData::Action::READ == data_.get()->m_action)
    {
        if(0 != write_all(socket_fd_0, &data_.get()->m_buffer[0], data_.get()->m_len))
        {
          throw Exception("nbd request write to storage failed\n");

        }
    }

}

void NBDDriverCommunicator::Disconnect() /*clean sockets, join threads*/
{
 
  if(-1 ==ioctl(nbd, NBD_DISCONNECT))//put ndb
  {
    throw Exception("ioctl -nbd_dev_to_disconnect  failed\n");
  } 
//check if clean something else
}


void ThreadFunc(size_t size,int nbd, int socket)
{

   if(-1 == ioctl(nbd, NBD_SET_SIZE, size))
  {
    throw Exception("ioctl - NBD_SET_SIZE fail\n");
  }

  if(-1 == ioctl(nbd, NBD_CLEAR_SOCK))
  {
    throw Exception("ioctl - NBD_CLEAR_SOCK fail\n");
  }


  /*do blocking*/
  sigset_t sigset;
  static int nbd_dev_to_disconnect = -1;
  
  /*if(0!=sigfillset(&sigset))
  {
   throw Exception("sigfillset  failed\n"); 
  }

  //sigprocmask(SIG_SETMASK, &sigset, NULL) != 0;//find THREAD SAVE func-pthread_sigmask
  if(0!= pthread_sigmask(SIG_SETMASK, &sigset, NULL))
  {
    throw Exception("pthread_sigmask  failed\n"); 
  }*/

   if ( (0 != sigfillset(&sigset)) || (0 != sigprocmask(SIG_SETMASK, &sigset, nullptr)) )
    {
        throw std::runtime_error("Failed to block signals in child");
    }

  /*flags NBD_FLAG_SEND_TRIM or NBD_FLAG_SEND_FLUSH*/
  /*if(-1 ==ioctl(nbd, NBD_SET_FLAGS, NBD_FLAG_SEND_TRIM | NBD_FLAG_SEND_FLUSH))
  {
    throw Exception("ioctl- NBD_SET_FLAGS failed\n");
  }*/
  /*blocking signals- sigfill*/

  /*if (sigfillset(&sigset) != 0 || sigprocmask(SIG_SETMASK, &sigset, NULL) != 0)
  {
    throw Exception("sigfillset/sigprocmask failed\n");
  }*/

  puts("333 in thnreadfunc");
  /*set the socket for the child*/
  if(-1 == ioctl(nbd, NBD_SET_SOCK, socket))
  {
    throw Exception("ioctl- NBD_SET_SOCK failed\n");
  }
  //puts("355 in threadfunction");
  /*main function- DO IT*/
 else
  {
    #if defined NBD_SET_FLAGS
    int flags = 1; // Changed from 0
    // #if defined NBD_FLAG_SEND_TRIM
    // flags |= NBD_FLAG_SEND_TRIM;
    // #endif
    #if defined NBD_FLAG_SEND_FLUSH
    flags |= NBD_FLAG_SEND_FLUSH;
    #endif
    if (flags && (-1 == ioctl(nbd, NBD_SET_FLAGS, flags)) )
    {
        throw Exception("ioctl(nbd, NBD_SET_FLAGS, " + std::to_string(flags) + ") failed");
    }
    #endif

    // if (BUSE_DEBUG) std::cerr << "nbd device terminated with code " << err << std::endl;
                    std::cout << "before NBD_DO_IT" << std::endl;            
    if (-1 == ioctl(nbd, NBD_DO_IT))
    {
        throw Exception("NBD_DO_IT terminated with error\n");
    }
  }

/*basic close up- close the sockcet and clear socket*/
  //ioctl(nbd_dev_to_disconnect, NBD_DISCONNECT); /*should be in discoconet function- should it be here too?*/
  if(-1==ioctl(nbd, NBD_CLEAR_QUE)|| (-1 == ioctl(nbd, NBD_CLEAR_SOCK)))
  {
    throw Exception("ioctl- NBD_CLEAR_QUE failed\n");
  }

  if(-1==ioctl(nbd, NBD_CLEAR_SOCK))
  {
    throw Exception("ioctl- NBD_CLEAR_SOCK failed\n");
  } 

  exit(EXIT_SUCCESS);

}


int read_all(int fd, char* buf, size_t count)//copy paste this
{
  int bytes_read;

  while (count > 0) {
    bytes_read = read(fd, buf, count);//save in loop so it wont be lost

    if(-1 == bytes_read)
    {
      throw Exception("read fail 2\n");
    }

    assert(bytes_read > 0);
    buf += bytes_read;
    count -= bytes_read;
  }
  assert(count == 0);

  return 0;
}

int write_all(int fd, char* buf, size_t count)
{
  int bytes_written;

  while (count > 0) {
    bytes_written = write(fd, buf, count);
    if (-1  == bytes_written)
    {
       throw Exception("write fail\n");
    }
    assert(bytes_written > 0);
    buf += bytes_written;
    count -= bytes_written;
  }
  assert(count == 0);

  return 0;
}

int NBDDriverCommunicator::GetSocketChild()//0
{
  return socket_fd_0;
}

int NBDDriverCommunicator::GetSocketFather()//1
{
  return socket_fd_1;

}


};