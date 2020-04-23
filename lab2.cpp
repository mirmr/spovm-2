#include <iostream>
#include <thread>
#include <vector>
#include <wait.h>
#include <unistd.h>
//#include <bits/sigaction.h>
#include <signal.h>
#include <termios.h>

class BufferToggle
{
private:
    struct termios t;
public:
    void off()
    {
        tcgetattr(STDIN_FILENO, &t); //get the current terminal I/O structure
        t.c_lflag &= ~ICANON; //Manipulate the flag bits to do what you want it to do
        tcsetattr(STDIN_FILENO, TCSANOW, &t); //Apply the new settings
    }
    void on()
    {
        tcgetattr(STDIN_FILENO, &t);
        t.c_lflag |= ICANON;
        tcsetattr(STDIN_FILENO, TCSANOW, &t);
    }
};

enum signals {
    io_int = SIGUSR1, launched = SIGUSR2
};

void log(const std::string& message = std::string{});

void slave();
void input(char& in, bool& ended);
void process_control();
__pid_t create_process();
void output_by_char(const std::string& str);

bool to_exit;
bool to_write;
bool point_to_next;
bool started;

void signal_handle(int signal);

int main() {
    BufferToggle bt;
    bt.off();

    struct sigaction buf;
    buf.sa_flags = SA_RESTART;
    sigemptyset(&buf.sa_mask);
    buf.sa_handler = signal_handle;
    if (sigaction(SIGUSR1, &buf, nullptr) == -1) {
        switch(errno) {
            case EINVAL:
                std::cout << "EINVAL" << std::endl;
                break;
            case ENOTSUP:
                std::cout << "ENOTSUP" << std::endl;
                break;
        }
    }
    if (sigaction(SIGUSR2, &buf, nullptr) == -1) {
        switch(errno) {
            case EINVAL:
                std::cout << "EINVAL" << std::endl;
                break;
            case ENOTSUP:
                std::cout << "ENOTSUP" << std::endl;
                break;
        }
    }
    log("started");
    process_control();
    bt.on();
    return 0;
}

void log(const std::string& message) {
    static __pid_t main_id = getpid();
    if (getpid() == main_id) {
        output_by_char("main process: ");
    }
    else {
        output_by_char("process " + std::to_string(getpid()) + ": ");
    }
    output_by_char(message);
    if (message.size())
        output_by_char("\n");
}

void slave() {
    to_exit = false;
    kill(getppid(), launched);

    while(!to_exit) {
        if (to_write) {
            to_write = false;
            log("my line");
            kill(getppid(), io_int);
        }
    }
}

void input(char& in, bool& ended) {
    bool legit = false;
    while(!legit) {
        in = std::getchar();
        if (in == '-') {
            legit = true;
        } else if (in == '+') {
            legit = true;
        } else if (in == 'q') {
            legit = true;
        }
    }
    ended = true;
}

void process_control() {
    std::vector<__pid_t> ids;
    size_t current = 0;
    size_t max = 0;
    char in;
    bool entered_value;
    while(!to_exit) {
        {
            entered_value = false;
            std::thread input_thr{input, std::ref(in), std::ref(entered_value)};
            while(!entered_value) {
                if (point_to_next && ids.size()) {
                    ++current;
                    if (current >= ids.size()) {
                        current = 0;
                    }
                    point_to_next = false;
                    kill(ids[current], io_int);
                }
            }
            input_thr.join();
        }

        switch (in) {
            case '+':
                started = false;
                ids.push_back(create_process());
                if (ids.size() > max) {
                    max = ids.size();
                }
                if (ids.size() == 1) {
                    point_to_next = true;
                }
                break;
            case '-':
                if (ids.size() == 0) {
                    break;
                }
                while(!point_to_next);
                kill(ids[ids.size() - 1], SIGKILL);
                waitpid(ids[ids.size() - 1], 0, NULL);
                ids.pop_back();
                break;
            case 'q':
                to_exit = true;
        }
    }
    while(!point_to_next && ids.size());
    while(ids.size()) {
        kill(ids[ids.size() - 1], SIGKILL);
        waitpid(ids[ids.size() - 1], 0, NULL);
        ids.pop_back();
    }
    log("max number of child processes = " + std::to_string(max));
}

__pid_t create_process() {
    started = false;
    __pid_t res = fork();
    switch(res) {
        case -1:
            throw -1;
        case 0:
            slave();
            exit(0);
        default:
            while(!started);
            return res;
    }
}

void signal_handle(int signal) {
    switch(signal) {
        case io_int:
            to_write = true;
            point_to_next = true;
            break;
        case launched:
            started = true;
            break;
    }
}

void output_by_char(const std::string& str) {
    for (auto i : str) {
        std::cout << i;
    }
}
