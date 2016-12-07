#include <iostream>
#include <cstdlib>
#include <csignal>
#include <cerrno>

#include "helpers.h"

// ./aucont_stop PID [SIG_NUM]
int main(int argc, char* argv[])
{
    if (argc <  2)
    {
      std::cout << __func__ << ": wrong input for aucont_stop!" << std::endl;
      return 1;
    }

    // gets PID
    int pid = atoi(argv[1]);
    if (pid == 0)
    {
        std::cout << __func__ << ": invalid PID in argument" << std::endl;
        return 1;
    }
    
    // send_signal
    if (exist_in_list(pid))
    {
        remove_from_list(pid);
        
        int signal = SIGTERM;
        if (argc > 3 && std::atoi(argv[2]) != 0)
        {
            signal = std::atoi(argv[2]);
            
        }
        kill(pid, signal);
    }

    return 0;
}

