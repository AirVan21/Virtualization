#include <vector>
#include <iostream>
#include <cstring>
#include <sstream>
#include <thread>

#include <sys/wait.h>
#include <linux/sched.h> // unshare
#include <unistd.h> // deamon, sethostname
#include <sys/mount.h>
#include <sys/syscall.h>
#include <arpa/inet.h> //increment_address
#include <unistd.h>

#include "helpers.h"
#include "input_parameters.h"

namespace aucont_start
{
    #define STACK_SIZE (1024 * 1024 * 4)
    
    char source_stack[STACK_SIZE];
    
    void start_daemon(input_parameters *params)
    {
        // changes work dir to "/"
        int nochdir = 0;
        // input && error to /dev/null
        int noclose = 0;
        if (daemon(nochdir, noclose) < 0)
        {
            // error
            std::cout << __func__ << ": daemon failed!" << std::endl;
            exit(1);
        }
    }
    
    void setup_container_uts()
    {
        std::string name = "container";
        // host name setting
        int ret = sethostname(name.c_str(), name.length());
        if (ret < 0)
        {
            std::cout << __func__ << ": sethostname failed!" << std::endl;
        }
    }
    
    void remount_root(const std::string& path)
    {
        std::string old_root = path + "/old_root";
        create_dir(old_root);
        mount(path.c_str(), path.c_str(), "bind", MS_BIND | MS_REC, NULL);
        syscall(SYS_pivot_root, path.c_str(), old_root.c_str());
        chdir("/");
        umount2("/old_root", MNT_DETACH); 
    }
    
    void setup_container_fs(const std::string& path)
    {
        unsigned long flags =  MS_NOSUID | MS_NODEV | MS_NOEXEC;
        
        std::string proc_dir = path + "/proc";
        create_dir(proc_dir);
        mount(NULL, proc_dir.c_str(), "auto", flags, NULL);

        std::string sys = path + "/sys";
        create_dir(sys);
        mount(NULL, sys.c_str(), "auto", flags, NULL);
        
        std::string shm = path + "/dev/shm";
        create_dir(shm);
        mount("/dev/shm", shm.c_str(), "auto", flags, NULL);
        
        // message
        std::string mqueue = path + "/dev/mqueue";
        create_dir(mqueue);
        mount("/dev/mqueue", mqueue.c_str(), "auto", flags, NULL);
        
        
        std::string zero = path + "/dev/zero";
        mknod(zero.c_str(), 0777, 0);
        mount("/dev/zero", zero.c_str(), "auto", flags, NULL);


        std::string null = path + "/dev/null";
        mknod(null.c_str(), 0777, 0);
        mount("/dev/null", null.c_str(), "auto", flags, NULL);
        
        remount_root(path);
    }
        
    int initialize_container(void *argument)
    {
        input_parameters *parameters = (input_parameters*) argument;
        
        if (parameters->m_is_deamon)
        {
            start_daemon(parameters);
        }
        
        // disassociate execution context
        unshare(CLONE_NEWPID);
        
        int pid = fork();
        if (pid > 0)
        {
            // writes container pid
            parameters->write_snd(pid);
            waitpid(pid, NULL, 0);
            // 
            exit(0);
        }
        
        // SYNC_POINT
        parameters->read_fst();

        // configure
        setup_container_uts();
        setup_container_fs(parameters->m_image_path);
        
        // SYNC_POINT
        parameters->write_snd(1);
        parameters->exec_command();
            
        return 0;
    }
    
    void setup_user(int pid)
    {
        std::string proc_pid = "/proc/" + std::to_string(pid);
        // uid 0 
        std::string uid_cmd = "echo \'0 " + std::to_string(getuid())  + " 1\' > " + proc_pid + "/uid_map";
        std::string setgroups_cmd = "echo deny > " + proc_pid + "/setgroups";
        // gid 0
        std::string gid_cmd = "echo \'0 " + std::to_string(getgid()) + " 1\' > " + proc_pid + "/gid_map";

        system(uid_cmd.c_str());
        system(setgroups_cmd.c_str());
        system(gid_cmd .c_str());   
    }
    
    void setup_cpu(int pid, const input_parameters& parms)
    {
        std::string cpu_dir = "containers/cgroup/cpu";
        std::string mount_cgroups_cmd = "sudo mount -t cgroup -o cpu none " + cpu_dir;
        if (!is_exist_dir(cpu_dir))
        {
            system(mount_cgroups_cmd.c_str()); 
        }

        std::string str_pid = std::to_string(pid);
        
        // creates dir
        std::string pid_dir = cpu_dir + "/" +str_pid;
        std::string mkdir_cmd = "sudo mkdir -p " + pid_dir;
        system(mkdir_cmd.c_str());
        
        // recursively change fileowner and group
        std::string chown_cmd = "sudo chown -R " + std::to_string(getuid()) + ":" + std::to_string(getgid()) + " " + pid_dir;
        system(chown_cmd.c_str());

        // 1 sec
        std::string period_cmd = "echo 1000000 >> " + pid_dir + "/cpu.cfs_period_us";
        // how much we allow access for cgroup in 1 sec
        int quota = 1000000 * parms.m_cpu_load / 100;
        std::string quota_cmd = "echo " + std::to_string(quota) + " >> " + pid_dir + "/cpu.cfs_quota_us";
        std::string pid_cmd = "echo " + str_pid + " >> " + pid_dir + "/cgroup.procs";

        system(period_cmd.c_str());
        system(quota_cmd.c_str());
        system(pid_cmd.c_str());
    }
} 

int main(int argc, char* argv[])
{
    using namespace aucont_start;

    input_parameters parameters = input_parameters::parse_input_parameters(argc, argv);
    if (parameters.is_empty())
    {
        std::cout << __func__ << ": empty parameters!" << std::endl;
        return 1;
    }
    if (parameters.set_pipes())
    {
        std::cout << __func__ << ": failed to create pipes!" << std::endl;
        return 1;
    }
    
    create_container_list();
    int namespace_flags = CLONE_NEWUSER | CLONE_NEWNET | CLONE_NEWIPC | CLONE_NEWNS | CLONE_NEWUTS | SIGCHLD;
    if (clone(initialize_container, source_stack + STACK_SIZE, namespace_flags, &parameters) < 0)
    {
        std::cout << __func__ << ": clone failed!" << std::endl;
    }
    
    // SYNC_POINT
    int created_pid = parameters.read_snd();
    
    setup_user(created_pid);
    setup_cpu(created_pid, parameters);

    // SYNC_POINT
    parameters.write_fst(42);
    parameters.read_snd();

    add_to_list(created_pid);
    std::cout << created_pid << std::endl;

    if (!parameters.m_is_deamon)
    {
        wait(NULL);
        remove_from_list(created_pid);
    }
    
    return 0;
}