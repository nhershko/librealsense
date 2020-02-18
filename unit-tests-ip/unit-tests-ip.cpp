#include <unistd.h>

#include <iostream>
#include <chrono>
#include <thread>

#include <librealsense2/rs.hpp>             // Include RealSense Cross Platform API
#include <librealsense2/hpp/rs_internal.hpp>

#include <ethernet/ip_device.hh>

#include <gtest/gtest.h>

#define SLEEP_FOR(X) std::this_thread::sleep_for(std::chrono::seconds(X))

void exec_cmd(char* const argv[]) {
    pid_t pid = fork();
    if (pid) {
        // parent
        std::cout << "[CAMOE_TEST] Executing command: '";
        int i = 0;
        do {
            if (argv[i]) std::cout << argv[i] << " ";
            else break;
            i++;
        } while (1);
        std::cout << "'\n";

        std::thread* threadObj = new std::thread([]() {
            int status  = {0};
            int timeout = 1000;

            while (0 == waitpid(-1 , &status , WNOHANG)) {
                    if ( --timeout < 0 ) {
                            perror("timeout");
                            return;
                    }
                    sleep(1);
            }
        });
    } else {
        // child
        const char* fname = argv[0];
        if (execve(fname, argv, NULL) == -1) {
            std::cout << "[CAMOE_TEST] Cannot execute: " << std::strerror(errno);
            exit(1);
        }
    }
}

class Streaming: public ::testing::Test { 
public: 
    Streaming( ) { 
        // initialization code here
    } 

    void SetUp( ) override { 
        // code here will execute just before the test ensues 
        // start server
        std::cout << "[CAMOE_TEST] Starting server\n";
        exec_cmd(cmd_start);
        SLEEP_FOR(3);
        std::cout << "[CAMOE_TEST] Server started\n";
    }

    void TearDown( ) override { 
        // code here will be called just after the test completes
        // ok to through exceptions from here if need be
        std::cout << "[CAMOE_TEST] Stopping server\n";
        exec_cmd(cmd_stop);
        SLEEP_FOR(1);
        std::cout << "[CAMOE_TEST] Server stopped\n";
    }

    ~Streaming( )  { 
        // cleanup any pending stuff, but no exceptions allowed
    }

   // put in any custom data members that you need 
public:

// #define SERVER_ADDRESS   "10.12.145.38"
// #define SERVER_USER      "apuzhevi"
// #define SERVER_ADDRESS  "10.12.144.71"
// #define SERVER_USER     "user"
#define SERVER_ADDRESS  "10.12.145.41"
#define SERVER_USER     "user"


    const int TIME_HEATUP = 5;  // let's give things to settle for two seconds
    const int TIME_TEST   = 10; // general time for streaming test
    const int TEST_FPS    = 30; // FPS value

    const std::string serv_addr = SERVER_ADDRESS;   // server address
    const std::string serv_port = "8554";           // server port

    char* const cmd_start[64] = { "/usr/bin/ssh", SERVER_USER"@"SERVER_ADDRESS, "-C", "/tmp/rs-server" }; 
    char* const cmd_stop[64]  = { "/usr/bin/ssh", SERVER_USER"@"SERVER_ADDRESS, "-C", "killall", "rs-server" }; 

private:
};

unsigned int frames = 0;
TEST_F (Streaming, FrameDrop) {

    rs2::software_device dev = ip_device::create_ip_device((serv_addr + ":" + serv_port).c_str());

    auto sensors = dev.query_sensors();

    rs2::syncer sync;

    sensors[0].open(sensors[0].get_stream_profiles()[0]);
    sensors[0].set_notifications_callback( [](rs2::notification n){ frames++; } );
    sensors[0].start(sync);
    SLEEP_FOR(TIME_HEATUP);
    int heat_up = frames;
    SLEEP_FOR(TIME_TEST);
    sensors[0].stop();

    std::cout << "[CAMOE TEST]\n";
    try {
        int expected_frames = TIME_TEST * TEST_FPS;
        int received_frames = frames - heat_up;
        std::cout << "[CAMOE TEST] ====================================\n" ;
        std::cout << "[CAMOE TEST] Heat Up  : " << heat_up << "(" << TIME_HEATUP * TEST_FPS << ")" << "\n" ;
        std::cout << "[CAMOE TEST] Expected : " << expected_frames << "\n" ;
        std::cout << "[CAMOE TEST] Received : " << received_frames << "\n" ;
        std::cout << "[CAMOE TEST] ====================================\n" ;

        float frame_range = (float)expected_frames / (float)received_frames;
        EXPECT_NEAR(frame_range, 1.0, 0.15); // 15% acceptable difference
    } catch (...) {
        std::cout << "[CAMOE TEST] Got an exception.\n";
        FAIL();
    }   
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
