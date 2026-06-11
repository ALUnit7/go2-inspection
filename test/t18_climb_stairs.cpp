// t18_climb_stairs: 台阶检测 + L型爬台阶
// 流程: ArUco减速 → 全黑停止 → 上台阶 → 左转90° → 下台阶
// 用法: sudo ./t18_climb_stairs <网卡名>
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>
#include <unitree/robot/go2/video/video_client.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/idl/go2/SportModeState_.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/aruco.hpp>
#include <csignal>
#include <atomic>
#include <cmath>
#include <unistd.h>

// ── 调参区 ────────────────────────────────────────────────────────────────────
static const double TAG_SIZE        = 0.07;
static const double FOCAL_LENGTH    = 600.0;
static const int    STAIR_TAG_ID    = 0;
static const float  APPROACH_SPEED  = 0.15f;
static const float  CLIMB_SPEED     = 0.08f;
static const float  TURN_SPEED      = 0.4f;
static const float  PITCH_STABLE_DEG= 3.0f;
static const float  TOP_WHITE_RATIO = 0.6f;
static const float  YAW_TURN_DEG    = 85.0f;
// ─────────────────────────────────────────────────────────────────────────────

std::atomic<bool> g_running{true};
void on_sigint(int) { g_running = false; }
std::atomic<float> g_pitch{0}, g_yaw{0};

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr,"Usage: sudo %s <iface>\n",argv[0]); return 1; }
    signal(SIGINT, on_sigint);

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
    unitree::robot::ChannelSubscriber<unitree_go::msg::dds_::SportModeState_> sub("rt/sportmodestate");
    sub.InitChannel([](const void* m){
        auto& s=*(const unitree_go::msg::dds_::SportModeState_*)m;
        g_pitch = s.imu_state().rpy()[1]*180.f/M_PI;
        g_yaw   = s.imu_state().rpy()[2]*180.f/M_PI;
    },1);

    unitree::robot::go2::SportClient sc;
    sc.SetTimeout(10.f); sc.Init();
    unitree::robot::go2::VideoClient vc;
    vc.SetTimeout(3.f); vc.Init();
    sleep(1);

    auto dict   = cv::makePtr<cv::aruco::Dictionary>(cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250));
    auto params = cv::makePtr<cv::aruco::DetectorParameters>();
    sc.FreeWalk();

    enum Phase { FOLLOW, APPROACH, CLIMB_UP, TURN_LEFT, CLIMB_DOWN, DONE };
    Phase phase = FOLLOW;
    float yaw_start=0, yaw_accum=0;
    int stable_count=0;

    std::vector<std::vector<cv::Point2f>> corners;
    std::vector<int> ids;
    std::vector<uint8_t> buf;

    while (g_running && phase!=DONE) {
        if (vc.GetImageSample(buf)!=0) { usleep(20000); continue; }
        cv::Mat frame=cv::imdecode(cv::Mat(buf),cv::IMREAD_COLOR);
        if (frame.empty()) { usleep(20000); continue; }

        float pitch=g_pitch.load(), yaw=g_yaw.load();
        corners.clear(); ids.clear();
        cv::aruco::detectMarkers(frame, dict, corners, ids, params);

        switch (phase) {
        case FOLLOW:
            sc.Move(APPROACH_SPEED*2,0,0);
            for (size_t i=0;i<ids.size();i++) {
                if (ids[i]==STAIR_TAG_ID) {
                    double px=(cv::norm(corners[i][0]-corners[i][1])+cv::norm(corners[i][1]-corners[i][2])+
                               cv::norm(corners[i][2]-corners[i][3])+cv::norm(corners[i][3]-corners[i][0]))/4.0;
                    printf("Tag id=%d dist=%.2fm, slowing\n",ids[i],(TAG_SIZE*FOCAL_LENGTH)/px);
                    phase=APPROACH;
                }
            }
            break;
        case APPROACH: {
            sc.Move(APPROACH_SPEED,0,0);
            cv::Mat gray,bin;
            cv::cvtColor(frame,gray,cv::COLOR_BGR2GRAY);
            cv::threshold(gray,bin,80,255,cv::THRESH_BINARY_INV);
            cv::Mat top=bin(cv::Rect(0,0,bin.cols,bin.rows/2));
            float ratio=(float)cv::countNonZero(top)/top.total();
            printf("top_white=%.2f\n",ratio);
            if (ratio>TOP_WHITE_RATIO) {
                sc.StopMove(); usleep(500000);
                printf("Full black. CLIMB_UP\n");
                stable_count=0; phase=CLIMB_UP;
            }
            break;
        }
        case CLIMB_UP:
            sc.Move(CLIMB_SPEED,0,0);
            if (std::abs(pitch)<PITCH_STABLE_DEG) stable_count++;
            else stable_count=0;
            printf("pitch=%.1f stable=%d\n",pitch,stable_count);
            if (stable_count>20) {
                sc.StopMove(); usleep(300000);
                printf("Top. TURN_LEFT\n");
                yaw_start=yaw; yaw_accum=0; stable_count=0;
                phase=TURN_LEFT;
            }
            break;
        case TURN_LEFT: {
            sc.Move(0,0,TURN_SPEED);
            float d=yaw-yaw_start;
            if (d>180)d-=360; if (d<-180)d+=360;
            yaw_accum=std::abs(d);
            printf("yaw_accum=%.1f\n",yaw_accum);
            if (yaw_accum>=YAW_TURN_DEG) {
                sc.StopMove(); usleep(300000);
                printf("Turned. CLIMB_DOWN\n");
                stable_count=0; phase=CLIMB_DOWN;
            }
            break;
        }
        case CLIMB_DOWN:
            sc.Move(CLIMB_SPEED,0,0);
            if (std::abs(pitch)<PITCH_STABLE_DEG) stable_count++;
            else stable_count=0;
            printf("pitch=%.1f stable=%d\n",pitch,stable_count);
            if (stable_count>20) {
                sc.StopMove();
                printf("Done.\n"); phase=DONE;
            }
            break;
        default: break;
        }

        cv::Mat vis=frame.clone();
        cv::aruco::drawDetectedMarkers(vis,corners,ids);
        char buf2[64]; snprintf(buf2,sizeof(buf2),"Phase:%d pitch=%.1f",phase,pitch);
        cv::putText(vis,buf2,{20,50},cv::FONT_HERSHEY_SIMPLEX,1,cv::Scalar(0,255,255),2);
        cv::imshow("Climb Stairs",vis);
        if (cv::waitKey(1)==27) break;
        usleep(20000);
    }
    sc.StopMove();
    return 0;
}
