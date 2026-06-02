# GO2 摄像头连续存图 - 环境配置与使用指南

## 环境信息

| 项目 | 值 |
|------|-----|
| 用户电脑网卡 | `enp108s0` |
| 用户电脑 IP | `192.168.123.200` |
| Go2 内置电脑 IP | `192.168.123.161` |
| 视频流地址 | `230.1.1.1:1720`（UDP 多播） |
| 视频规格 | 1280×720，15fps，H264 |

---

## 第一步：确认 ffmpeg 可用

```bash
ffmpeg -version
# 应输出 ffmpeg version 8.x ...
```

> 使用 conda 环境时 ffmpeg 已内置，无需额外安装。

---

## 第二步：验证视频流可达

运行存图脚本前，先用 ffmpeg 确认能收到视频流：

```bash
ffmpeg -i "udp://@230.1.1.1:1720" -vframes 1 /tmp/test.jpg
# 成功则生成 /tmp/test.jpg
```

也可用 GStreamer 查看实时画面：

```bash
gst-launch-1.0 udpsrc address=230.1.1.1 port=1720 multicast-iface=enp108s0 \
  ! queue ! application/x-rtp, media=video, encoding-name=H264 \
  ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! autovideosink
```

---

## 第三步：运行存图脚本

脚本位置：`/home/wzl/GO2_RCOM/record_frames.py`

```bash
cd /home/wzl/GO2_RCOM
python3 record_frames.py --iface enp108s0 --output ./frames
```

- 图片按时间戳命名存入 `./frames/` 目录，格式：`YYYYMMDD_HHMMSS_NNN.jpg`
- `Ctrl+C` 停止，打印总帧数和平均帧率

---

## 故障排查

| 现象 | 原因 | 解决 |
|------|------|------|
| ffmpeg 无输出/卡住 | 视频流未启动或网络不通 | 先 `ping 192.168.123.161` 确认连通 |
| 帧率远低于 15fps | 存图 IO 瓶颈 | 换 SSD 或降低 JPEG 质量（`-q:v` 参数） |
| ping 161 不通 | 网络未连接 | 检查网线和 IP 配置（需在 `192.168.123.x` 网段） |
