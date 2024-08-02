
## 项目配置

- 在`CMakeLists`中指定`project_name`为项目名称，`Makefile`中的`OUTPUT_BIN`的名称应当一致。
- 新建`make.env`文件，在其中指定 `SDK_PATH` 和 `ARM_GCC_PATH`

### 0. cmsis-dap + openOCD + cmake (推荐流程)

需要确保如下组件已经加入了系统的`PATH`中：

1. `make`
2. `cmake`
3. `ninja`
4. `openocd`

```shell
# 编译
make
# 进入编译目录
cd build
# 烧录softdevice
ninja openocd_softdevice
# 烧录application
ninja openocd
```

推荐使用`DRTTView`通过在`daplink`上实现的`RTT`进行调试。

### 1. cmsis-dap + openOCD + make

需要确保如下组件已经加入了系统的`PATH`中：

1. `make`
2. `cmake`
3. `ninja`
4. `openocd`

```shell
# 编译
make
# 擦除
make flash_erase
# 烧录softdevice
make flash_softdevice
# 烧录application
make flash
# gdb server
make debug
```

### 2. jlink + nrfjprog+ make

需要确保如下组件已经加入了系统的`PATH`中：

1. `make`
2. `cmake`
3. `ninja`
4. `nrfjprog`

```shell
# 编译
make
# 擦除
make jlink_erase
# 烧录softdevice
make jlink_flash_softdevice
# 烧录application
make jlink_flash
```
