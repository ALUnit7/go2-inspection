# GO2 拓展坞配置文档

> 更新时间：2026-04-01

---

## Jetson 模组规格对比

| 规格 | Jetson Orin Nano 8GB | Jetson Orin NX 16GB |
|------|----------------------|----------------------|
| AI 性能 | 40 TOPS | 100 TOPS |
| GPU | 1024 核 Ampere，1024 Tensor Core | 1024 核 Ampere，32 Tensor Core |
| GPU 最大频率 | 625 MHz | 918 MHz |
| CPU | 6 核 Cortex-A78AE，1.5MB L2 + 4MB L3 | 8 核 Cortex-A78AE，2MB L2 + 4MB L3 |
| CPU 最大频率 | 1.5 GHz | 2 GHz |
| DL 加速器 | - | 2x NVDLA v2（614 MHz） |
| 视觉加速器 | - | 1x PVA v2 |
| 显存 | 8GB 128位 LPDDR5，68 GB/s | 16GB 128位 LPDDR5，102.4 GB/s |
| 视频编码 | 1080p30（1-2 CPU 核心） | 1x 4K60 / 3x 4K30 / 6x 1080p60（H.265） |
| 视频解码 | 1x 4K60 / 2x 4K30 / 5x 1080p60（H.265） | 1x 8K30 / 2x 4K60 / 4x 4K30（H.265） |
| 功耗 | 7W - 15W | 10W - 25W |

---

## 基本连接信息

| 项目 | 值 |
|------|----|
| 拓展坞 IP | `192.168.123.18` |
| 用户名 | `unitree` |
| 密码 | `123` |

**连接显示屏：** 使用 Type-C 转 HDMI 转接头（推荐绿联 CM475）插入拓展坞 Type-C 全功能接口。

**SSH 登录：**
```bash
ssh unitree@192.168.123.18
# 密码：123
```

> 注意：SSH 登录端口需与拓展坞在同一局域网，且 IP 必须为 `192.168.123.xxx` 网段。

---

## 模块更新

1. 通过网线连接主机与拓展坞 RJ45 口，确保主机 IP 在 `192.168.123.xx` 网段
2. 浏览器访问 `http://192.168.123.18`
3. 上传更新包 → 开始更新（进度显示在右侧窗口）
4. 支持回退至上一版本；暂不支持恢复出厂

### 更新包列表

| 日期 | 说明 |
|------|------|
| 2025-06-06 | 里程计服务 |
| 2025-09-02 | GO2 SLAM 服务（修复定位异常，增加 YSN 校验） |
| 2025-09-02 | GO2_W SLAM 服务（修复定位异常，增加 YSN 校验） |
| 2026-04-01 | GO2 SLAM 服务（适配 Mid360s 雷达） |
| 2026-04-01 | GO2_W SLAM 服务（适配 Mid360s 雷达） |

---

## 故障修复

### Type-C 无法供电修复

> 以下操作需在 root 权限下执行

**Step 1：检查当前状态**
```bash
cat /sys/class/gpio/PP.06/value
# 返回 1 表示正常，建议排查硬件；返回其他值继续 Step 2
```

**Step 2：重建 rc.local 使能脚本**

```bash
# 1. 复制服务文件
cp /lib/systemd/system/rc-local.service /etc/systemd/system

# 2. 在 /etc/systemd/system/rc-local.service 末尾添加：
# [Install]
# WantedBy=multi-user.target
# Alias=rc-local.service

# 3. 创建 /etc/rc.local，内容如下：
#!/bin/sh
busybox devmem 0x02430030
busybox devmem 0x02430030 w 0x004
echo 446 > /sys/class/gpio/export
echo out > /sys/class/gpio/PP.06/direction
echo 1 > /sys/class/gpio/PP.06/value
exit 0

# 4. 赋予可执行权限
chmod +x /etc/rc.local

# 5. 重启后重新执行 Step 1 验证
```

---

### USB Type-A 口被识别为 USB 2.0 修复

受影响版本：设备树版本 `Mar 19 2023 08:08:40`，已在 `Sep 30 2025 17:55:09` 修复。

```bash
# 下载并解压脚本
unzip b8afa4618f5f479db1bc94e2c3c166f7.zip
cd unitree_dt_update/

# 执行更新脚本，按提示重启
chmod +x unitree_update.sh
sudo ./unitree_update.sh
```

---

## 系统备份与恢复

> 需取出拓展坞内部 NVME 硬盘操作，操作不当不在免费保修范围内。建议收到设备后立即备份。
>
> 所需工具：NVME 读卡器、PC 主机，root 权限。

### 系统备份

```bash
# 查看存储设备节点（如 /dev/sdc）
lsblk -f

# 备份到主机
sudo dd if=/dev/sdc status=progress | bzip2 > nx.img.bz2
```

### 系统恢复

```bash
# 格式化新 NVME（如为新设备）
sudo mkfs -t ext4 /dev/sdc

# 烧写镜像（至少需要 300GB 空间，耗时 4~5 小时属正常）
bzip2 -dc nx.img.bz2 | sudo dd of=/dev/sdc status=progress
```

也可在 Windows 上使用 **Rufus** 刷入镜像。

### 官方出厂镜像

| 型号 | 镜像文件名 |
|------|-----------|
| Go2 NX 拓展坞 | `go2_nx_Jetpack5.1.1_20250930.img.bz2` |
| Go2 NANO 拓展坞 | `go2_nano_Jetpack5.1.1_20250930.img.bz2` |

下载地址：[百度网盘](https://pan.baidu.com/s/1GMs-DE8SSHYTSNIVKsoktw?pwd=7riu)（提取码：`7riu`）
