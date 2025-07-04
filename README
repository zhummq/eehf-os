# eehf-os

这是一个从零开始实现的教学型操作系统，目标是通过亲手构建内核、驱动和协议栈，深入理解现代操作系统的本质。

## 🌟 特性

- ✅ 多任务：内核态调度器、上下文切换、进程状态管理
- ✅ 内存管理：分页机制、页表映射、内核/用户地址空间隔离
- ✅ 文件系统：简易块设备抽象层
- ✅ 网络支持：
  - E1000 网卡驱动（基于 Bochs 模拟器）
  - 协议支持：Ethernet、ARP、IPv4、ICMP、UDP
  - 支持 Ping 与 UDP 回显测试
- ✅ socket 抽象接口（未来支持 TCP）
- 🛠️ 支持 GDB 调试与调试日志系统

## 🧰 开发环境

- 模拟器：Bochs2.8-e1000
- 编程语言：C + 少量汇编
- 工具链：`gcc`, `make`, `nasm`, `gdb`

## 🚀 快速开始

```bash
git clone https://github.com/zhummq/eehf-os.git
cd eehf-os
make run  # 启动 Bochs 模拟器

## 📚 学习参考
- 《操作系统真相还原》
-  [Intel E1000 规范文档](https://pdos.csail.mit.edu/6.828/2018/readings/hardware/8254x_GBe_SDM.pdf)
- b站视频 [写网络协议栈](https://www.bilibili.com/video/BV1CJM8z8Eb4?vd_source=f9e49111868edc5258768c0629d63793)
