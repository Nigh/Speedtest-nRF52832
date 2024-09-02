# nRF-Speedtest

本项目为 `nRF52832` 测速工程

使用 `cmake` 进行编译。  
支持使用`JLink`和`cmsis-dap`进行下载调试。

## 环境配置

- [工具链](./toolchian.md)
- [项目配置](./requirement.md)

## 使用

> [!TIP]
推荐直接使用配套 [PC端应用](https://github.com/Nigh/Speedtest-BLE-PC) 进行测试。

> [!NOTE]
> 使用其他设备或调试工具手动测试时，请遵循如下操作：
> 1. 首先，应当开启 `0xA801` 特征Notify的监听。
> 2. 然后，向 `0xA802` 特征写入十六进制的 `01AA0300` 即可开启传输测试。
> 3. 固件会向 `0xA801` 持续推送约 `512kb` 的数据。其中，每一包数据的前 `4` 个字节为连续的32位序号，可用于确认丢包情况。

## 蓝牙配置

### 广播
- 广播间隔: `125ms`
- 广播包:
	- Device type: Len=`2`
	- Name: Len=`N`
	- Manufacturer data: Len=`12`

其中，`Manufacturer data` 格式为:
- Company ID: 2 bytes 
- Project ID: 3 bytes
- MAC: 6 bytes

### 连接
- 连接间隔: `20-50` ms
- 连接超时: `800` ms
- slave latency: `2`
- GAP event length: 最大
- ATT Data length: `247` bytes

### 服务
- Service UUID: `0xA800`
	- Char UUID: `0xA801`
		- 属性: `Notify`
	- Char UUID: `0xA802`
		- 属性: `Write without response`

### 实测结果
本项目使用安卓手机App `nRF Connect` 实测连续传输 512kB 数据，取多次传输耗时的中位数作为结果：

- PHY: 2Mbps
	- Data length: 512kB
	- Time elapsed: 3.920s
	- Speed: 130KB/s
- PHY: 1Mbps
	- Data length: 512kB
	- Time elapsed: 6.881s
	- Speed: 74KB/s
