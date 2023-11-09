#include <stdio.h>
#include <cstring>      // strcspn
#include <exception> //throw
#include <signal.h>          /* signals */
#include <sys/epoll.h>
#include "drive_communicator.hpp"
#include "storage.hpp"
#include "logger.hpp"


//sudo su- for root permissions

using namespace hrd29;

IDriverCommunicator* ptr_nbd;
const size_t g_diskSize = 128000000;
const size_t g_max_num_event = 10; //2 should be enough?
const size_t g_max_input_size = 10;
/****************************************/
void InitSignals(Logger* logger1);
void WrapperDisconnectNbd(int sig_num);
/****************************************/

/*runing nbd locally(manually- would be in scripts)
first terminal
modprobe nbd
sudo ./a.out


second terminal
sudo su
 mkfs.ext4 /dev/nbd0
mount /dev/nbd0 ./test_mounted_files
nano text.txt


*/

int stop_flag = 1; //TODO: atomic


int main(void)
{
    //NBDDriverCommunicator* nbd = new NBDDriverCommunicator("/dev/ram0", 128000000);
    //ptr_nbd = NBDDriverCommunicator("/dev/ram0", 128000000);


    RAMStorage ramStorage(g_diskSize);
    ptr_nbd = new NBDDriverCommunicator("/dev/nbd1", g_diskSize);
    std::shared_ptr<DriverData> data;
     struct epoll_event events[g_max_num_event]; // 0 for NBD socket, 1 for STDIN
    struct epoll_event nbd_event;
    struct epoll_event stdin_event;
    char input_buffer[g_max_input_size];
    int num_waiting_events = 0;
    int epoll_fd = 0;
    int i = 0;
    int nbd_fd;

    
   ptr_nbd->logger1->Write( hrd29::Logger::Info,__FILE__, __LINE__, __func__, "main master- error level", true );
    

    try
    {
        ptr_nbd->logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "main master- error level", true );
        InitSignals(ptr_nbd->logger1);
        epoll_fd = epoll_create1(0); // Create epoll instance and event array
        if(-1 == epoll_fd)
        {
            throw MyException("fail in epoll_create1\n");
            ptr_nbd->logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "main master- Failed epoll_create1", true );
        }
        nbd_fd = ptr_nbd->GetFD();
        
        // Add NBD socket to epoll
        nbd_event.events = EPOLLIN; // Listen for read events
        nbd_event.data.fd = nbd_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, ptr_nbd->GetFD(), &nbd_event) == -1)
        {
            throw MyException("Failed to add NBD socket to epoll");
            ptr_nbd->logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "main master- Failed to add NBD socket to epoll", true );

        }

        // Add STDIN to epoll
        stdin_event.events = EPOLLIN; // Listen for read events
        stdin_event.data.fd = STDIN_FILENO;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &stdin_event) == -1)
        {
            throw MyException("Failed to add STDIN to epoll");
            ptr_nbd->logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "main master- Failed to add STDIN to epoll", true );
        }   

        while(1)// i change the flag latter
        {
            num_waiting_events = epoll_wait(epoll_fd, events, 2, -1);
            if(-1 == num_waiting_events)
            {
                if(stop_flag == 0)//after flag changed
                {
                    ptr_nbd->Disconnect();//NBD_DISCONNECT
                }
                throw MyException("Failed in epoll_wait() function");
                ptr_nbd->logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "main master- Failed in epoll_wait()", true );
            }

            for(i=0; i<num_waiting_events; ++i)
            {
                if(STDIN_FILENO == events[i].data.fd)
                {
                    fgets(input_buffer, g_max_input_size, stdin);
                    input_buffer[strcspn(input_buffer, "\n")] = '\0'; /* Remove newline character */

                    if (0 == strcmp(input_buffer, "quit")) 
                    {
                        WrapperDisconnectNbd(SIGINT);//changing flag to 0 
                        return 0;
                    }
                }

                else if(nbd_fd == events[i].data.fd)
                {

                    data = ptr_nbd->ReceiveRequest();

                    switch (data->m_action)
                    {
                        case DriverData::READ:
                            ramStorage.Read(data);
                            // printf("case DriverData::READ\n");
                            break;

                        case DriverData::WRITE:
                            ramStorage.Write(data);
                            // printf("case DriverData::WRITE\n");
                            break;

                        // Handle other actions (FLUSH, TRIM) if needed.

                        default:    // Handle unsupported actions or errors.
                            printf("default\n");
                            break;
                    }

                    ptr_nbd->SendReply(data);
                }
            }
        }
     
    }
    

    catch (const MyException& ex)  // Handle MyException
    {
        std::cout << "Gotta Catch 'em All" <<std::endl;
         ptr_nbd->logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, ex.what(), true );
        //std::cerr << "Caught MyException: " << ex.what() << std::endl;//redundency
    }

    catch (...)  // unexpected exceptions-default catch
    {
         ptr_nbd->logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "Caught unknown exception", true );
        //std::cerr << "Caught unknown exception" << std::endl;
    }

        
    
    return 0;
}


void InitSignals(Logger* logger1)
{
    struct sigaction sa_act;
    sa_act.sa_flags = SA_SIGINFO;    //SA_RESTART;
    sa_act.sa_handler = &WrapperDisconnectNbd;    //   disconnect_nbd;  
    if (sigemptyset(&sa_act.sa_mask) != 0 || sigaddset(&sa_act.sa_mask, SIGINT) != 0 ||sigaddset(&sa_act.sa_mask, SIGTERM) != 0) 
    {
        throw MyException("failed to register signal handlers in parent");
        logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "failed to register signal handlers in parent", true );
    }

    if(sigaction(SIGINT, &sa_act, NULL) != 0 ||sigaction(SIGTERM, &sa_act, NULL) != 0) 
    {
        throw MyException("failed to register signal handlers in parent");
        logger1->Write( hrd29::Logger::Error,__FILE__, __LINE__, __func__, "failed to register signal handlers in parent", true );
    }
}


void WrapperDisconnectNbd(int sig_num)
{
    (void) sig_num;

    //ptr_nbd->Disconnect();// DO NOT CALL FUNCTION IN SIGANL HANDLER!
    //change flag instead
    stop_flag = 0;
}

