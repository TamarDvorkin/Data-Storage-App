#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <unistd.h>         /* close */
#include <sys/types.h>      /* socket(),  */
#include <sys/socket.h>     /* socket interface functions - socket(),   */
#include <netinet/in.h>     /*internet address structures */
#include <linux/nbd.h>      /* NBD protocol */
#include <fcntl.h>
#include <sys/ioctl.h>      /* ioctl() */
#include <signal.h>          /* signals */
#include <assert.h>
#include <cstring>
#include <arpa/inet.h>      /* htonl */


#include "drive_communicator.hpp"


namespace hrd29
{   
    void ThreadFunc(size_t size, int nbd_fd, int socket,Logger* logger1);
    int ReadAll(int fd, char* buf, size_t count);
    int WriteAll(int fd, char* buf, size_t count);
    void WrapperThread (size_t size, int nbd_fd, int socket, Logger* logger1);
    

    /***********************IDriverCommunicator***********************/
    

    /*********************NBDDriverCommunicator*********************/
    /*
    These helper functions were taken from cliserv.h in the nbd distribution.
    */
    #ifdef WORDS_BIGENDIAN
    u_int64_t ntohll(u_int64_t a) {
    return a;
    }
    #else
    u_int64_t ntohll(u_int64_t a) 
    {
        u_int32_t lo = a & 0xffffffff;
        u_int32_t hi = a >> 32U;
        lo = ntohl(lo);
        hi = ntohl(hi);
        return ((u_int64_t) lo) << 32U | hi;
    }
    #endif
    #define htonll ntohll
    

    NBDDriverCommunicator::NBDDriverCommunicator(const std::string &device_path, size_t nbd_size)
    {
        // size = nbd_size;
        logger1= Singleton<Logger>::GetInstance();
        puts("enter Ctor NBD\n");
        /***** SOCKETS ******/
        int sock_pair[2]; /* sock_pair[0] is the NBD driver socket and sock_pair[1] is the socket we can use for communication. */
        if(-1 ==  socketpair(AF_UNIX, SOCK_STREAM, 0, sock_pair) )
        {
            logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "driver communicator- Failed to create socketpair", true );
            throw MyException("Failed to create socketpair\n");

        }

        m_socket_fd = sock_pair[0]; 

        /****** NBD protocol ******/
        int nbd_fd = open(device_path.c_str(), O_RDWR);
        if(-1 == nbd_fd)
        {
            logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "driver communicator- Failed to open device_path", true );
            throw MyException("Failed to open device_path\n");
        }

        
        background_thread = std::thread(&WrapperThread, nbd_size, nbd_fd, sock_pair[1],logger1);//&ThreadFunc
        background_thread.detach();
        m_main_fd =  sock_pair[1]; 

        
        puts("finish Ctor NBD\n");

    }

    void WrapperThread (size_t size, int nbd_fd, int socket,Logger* logger1)
    {
        try
        {
            ThreadFunc(size, nbd_fd,socket, logger1);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }

        return;
        
    }

    void ThreadFunc(size_t size, int nbd_fd, int socket, Logger* logger1)
    {
        if(-1 == ioctl(nbd_fd, NBD_SET_SIZE, size)) /* Set block size for the NBD device */
        {
            logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "driver communicator- Set socket for NBD device", true );
           throw MyException("Failed to Set socket for NBD device\n");

        }

        if (-1 == ioctl(nbd_fd, NBD_CLEAR_SOCK)) /* Clear socket for the NBD device */
        {
            logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "driver communicator- clear socket for NBD device", true );
            throw std::runtime_error("Failed to clear socket for NBD device");
            
        }

        sigset_t sigset;
        /* Block all signals to not get interrupted in ioctl(NBD_DO_IT) */
        if ( (0 != sigfillset(&sigset)) || (0 != pthread_sigmask(SIG_BLOCK, &sigset, nullptr)) )
        {
            logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "driver communicator- Failed to block signals in child", true );
            throw std::runtime_error("Failed to block signals in child");
            
        }

        if (-1 == ioctl(nbd_fd, NBD_SET_SOCK, socket))
        {
            logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "driver communicator- NBD_SET_SOCK failed", true );
            throw std::runtime_error("ioctl(nbd_fd, NBD_SET_SOCK, sk) failed.");
        }

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
            if (flags && (-1 == ioctl(nbd_fd, NBD_SET_FLAGS, flags)))
            {
                
                logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "driver communicator- NBD_SET_FLAGS failed", true );
                throw MyException("ioctl(nbd, NBD_SET_FLAGS, " + std::to_string(flags) + ") failed");
            }
            #endif

            // if (BUSE_DEBUG) std::cerr << "nbd device terminated with code " << err << std::endl;
            std::cout << "before NBD_DO_IT" << std::endl;            
            if (-1 == ioctl(nbd_fd, NBD_DO_IT))
            {
                logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "driver communicator- NBD_DO_IT failed", true );
                throw MyException("NBD_DO_IT terminated with error\n");
            
            }
        }
    
        if ( (-1 == ioctl(nbd_fd, NBD_CLEAR_QUE)) || (-1 == ioctl(nbd_fd, NBD_CLEAR_SOCK)) )
        {
            
            logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "driver communicator- Failed to perform nbd cleanup actions", true );
            throw MyException("Failed to perform nbd cleanup actions\n");
        }

        exit(EXIT_SUCCESS);    
    }
    
    NBDDriverCommunicator::NBDDriverCommunicator(const std::string &device_path, size_t block_size, size_t num_blocks)
                         : NBDDriverCommunicator(device_path, block_size * num_blocks)
    {}


    NBDDriverCommunicator::~NBDDriverCommunicator() noexcept
    {
        Disconnect();
    
    }


    std::shared_ptr<DriverData> NBDDriverCommunicator::ReceiveRequest()
    {
        //puts("  enter ReceiveRequest\n");
        struct nbd_request request;
        u_int32_t len;
        ssize_t bytes_read = sizeof(request);
        std::shared_ptr<DriverData> requestData = NULL;


        if (bytes_read == read(m_socket_fd, &request, sizeof(request)))
        {           
            len = ntohl(request.len);
            assert(request.magic == htonl(NBD_REQUEST_MAGIC));
            requestData = std::make_shared<DriverData>(len);
            memcpy(reinterpret_cast<char*>(&(requestData->m_handle)), request.handle, sizeof(request.handle));   
            requestData->m_action = static_cast<DriverData::Action>(ntohl(request.type));    
            requestData->m_offset = ntohll(request.from);   
            if(DriverData::Action::WRITE == requestData->m_action)
            {  
                ReadAll(m_socket_fd, requestData->m_buffer.data(), len);
                std::cout <<requestData->m_buffer.data() <<std::endl;
            }
            return requestData;
        }

        
        logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "driver communicator- failed to read - ReceiveRequest function", true );
        throw MyException("failed to read - ReceiveRequest function\n");
        //puts("  finish ReceiveRequest\n");
        logger1->Write( hrd29::Logger::Info,__FILE__, __LINE__, __func__, "finish ReceiveRequest", true );
    }


    void NBDDriverCommunicator::SendReply(std::shared_ptr<DriverData> data_)
    {
        //puts("  enter SendReply\n");
        
        struct nbd_reply reply;

        reply.magic = htonl(NBD_REPLY_MAGIC);
        reply.error = htonl(0);

        memcpy(reply.handle, reinterpret_cast<char*>(&(data_->m_handle)), sizeof(reply.handle));//or std::copy
        WriteAll(m_socket_fd, reinterpret_cast<char*>(&reply), sizeof(reply));
        if(DriverData::Action::READ == data_->m_action)
        {    
            WriteAll(m_socket_fd, data_->m_buffer.data(), data_->m_buffer.size());
        }
        //puts("  finish SendReply\n");
        logger1->Write( hrd29::Logger::Info,__FILE__, __LINE__, __func__, "finish SendReply", true );
    }


    void NBDDriverCommunicator::Disconnect()
    {
        static int nbd_dev_to_disconnect = -1;

        if (nbd_dev_to_disconnect != -1) 
        {
            if(-1 == ioctl(nbd_dev_to_disconnect, NBD_DISCONNECT) ) 
            {
                logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "driver communicator-  NBD_DISCONNECT failed", true );
                throw MyException("Failed to request disconnect on nbd device\n");
            } 
            
            else 
            {
                nbd_dev_to_disconnect = -1;
                fprintf(stderr, "successfully requested disconnect on nbd device\n");
            }
        }

        close(m_socket_fd);
        close(m_main_fd);

    }
 
    int NBDDriverCommunicator::GetFD() const
    {
        return m_socket_fd;
    }

/********************************************************************************/

    int ReadAll(int fd, char* buf, size_t count)
    {
        int bytes_read;

        while (count > 0) 
        {
            bytes_read = read(fd, buf, count);
            assert(bytes_read > 0);
            buf += bytes_read;
            count -= bytes_read;
        }
        assert(count == 0);

        return 0;
    }


    int WriteAll(int fd, char* buf, size_t count)
    {
        int bytes_written;

        while (count > 0) 
        {
            bytes_written = write(fd, buf, count);
            assert(bytes_written > 0);
            buf += bytes_written;
            count -= bytes_written;
        }
        assert(count == 0);

        return 0;
    }



}

