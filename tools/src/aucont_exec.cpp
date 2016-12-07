#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <fcntl.h>
#include <sys/wait.h>

#include "helpers.h"

namespace aucont_exec
{    
    void set_namespace(int pid, std::string ns_str)
    {
        std::string ns_path = "/proc/" + std::to_string(pid) + "/ns/" + ns_str;
        int descr = open(ns_path.c_str(), O_RDONLY);
        setns(descr, 0);
        close(descr);
    }

    void set_cgroup(int pid)
    {
        std::string pid_str = std::to_string(pid);
        std::string cpu_dir = "containers/cgroup/cpu";
        if (!is_exist_dir(cpu_dir))
        {
            //mount -t (type) -o old_dir new_nir
            std::string mount_str = "sudo mount -t cgroup -o cpu none " + cpu_dir;
            system(mount_str.c_str());
        }

        std::string add_pid_cmd = "echo " + std::to_string(getpid()) + " >> " + cpu_dir + "/" + pid_str + "/cgroup.procs";
        system(add_pid_cmd.c_str());
    }
    
    void set_environment(int pid)
    {
        set_cgroup(pid);
        set_namespace(pid, "ipc");
        set_namespace(pid, "mnt");
        set_namespace(pid, "net");
        set_namespace(pid, "pid");
        set_namespace(pid, "user");
        set_namespace(pid, "uts");
    }
    
    void execute(std::vector<char*>& args)
    {
        int pid = fork();
        if (pid != 0)
        {
            // parent
            waitpid(pid, NULL, 0);
        }
        else
        {
            // child
            char* command = args[0];
            execvp(command, args.data());
        }
    }
}

// ./aucont_exec PID CMD [ARGS]
int main(int argc, char* argv[])
{ 
    using namespace aucont_exec;
            
    if (argc < 2)
    {
        std::cout << __func__ << ": invalid parameters for aucont_exec" << std::endl;
        exit(1);
    }

    std::vector<char*> args;
    int input_pid = atoi(argv[1]);
    for (int i = 2; i < argc; ++i)
    {
      args.push_back(argv[i]);
    }
    
    set_environment(input_pid);
    execute(args);
    
    return 0;
}

