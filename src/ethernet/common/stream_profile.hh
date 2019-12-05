
typedef enum stream_type_id
{
    STREAM_DEPTH,
    STREAM_COLOR,
};

typedef struct stream_resolution
{ 
     int width; 
     int hight; 
};

class stream_profile
{
    public:
        stream_profile(stream_type_id id, stream_resolution res, int fps)
        :m_stream_id(id),
        m_stream_res(res),
        m_stream_fps(fps){}

        stream_profile(char* sdp_line);

    private:
        stream_type_id m_stream_id;
        stream_resolution m_stream_res;
        int m_stream_fps;

};