#ifndef _RS_COMMON_HH
#define _RS_COMMON_HH
#pragma pack(push,1)

struct rs_over_ethernet_data_header {
            uint32_t size;
        };
struct rs_frame_metadata {
            double timestamp;
            long long frame_counter;
            int actual_fps;
            rs2_timestamp_domain timestamp_domain;
        };
struct rs_frame_header {
            rs_over_ethernet_data_header ethernet_header;
            rs_frame_metadata metadata;
        };

#pragma pack(pop)
#endif
