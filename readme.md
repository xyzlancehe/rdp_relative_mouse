# RDP Relative Mouse

RDP只发送鼠标的绝对位置, 这导致在远程桌面中, 像《原神》这样依赖于鼠标的相对位移的3D游戏无法操作（无法转动视角）。

本程序使用[Interception库](https://github.com/oblitum/Interception), 在远程主机上拦截收到的鼠标绝对位置, 为其附加一个相对移动, 使得可以通过远程桌面操作一些3D游戏（至少可以游玩原神）。

+ 原理

  使用远程桌面时，我们看到的鼠标是本地鼠标。在远程主机上，会有一个虚拟的鼠标设备。当本地鼠标在远程桌面窗口中移动时，远程虚拟鼠标设备将产生对应的输入信号，仅包含鼠标的绝对位置。

  正常情况下，本地鼠标的位置和远程鼠标的位置始终是重合的。

  本程序在远程主机上拦截虚拟鼠标的输入信号，根据上一次输入信号的绝对位置，计算相对位移，产生一个相对位移的信号。

  此外，仅当本地鼠标在远程桌面窗口中的位置产生变化时，远程主机才会收到信号。如果本地鼠标位于窗口边界或者角落，继续向边界外侧移动本地鼠标，鼠标在窗口中的实际位置没有发送变化，那么远程主机也不会收到信号。

  因此，本程序还会判断鼠标位置是否位于屏幕边界，如果位于边界，也会做特别处理，产生相对位移信号。
+ 安装Interception驱动

  本程序依赖的Interception库需要安装一个驱动才能正常运行。前往[Release v1.0.1 · oblitum/Interception](https://github.com/oblitum/Interception/releases/tag/v1.0.1)下载压缩包并解压, 进入 `command line installer`目录, 打开具有管理员权限的命令行, 运行命令进行安装：

  ```
  ./install-interception.exe /install
  ```
  重启计算机生效。
+ Build

  使用CMake构建本程序，`bin`目录下会生成 `mouse_info.exe`, `calibrate.exe `和 `main.exe`。
+ 运行

  将 `interception.dll`复制到 `exe`的目录或者添加到 `path`。

  + `mouse_info.exe`
    运行此程序，会在终端中实时输出鼠标移动信息。

    一般来说，本地桌面运行此程序，将会看到鼠标移动类型为 `relative`, 此时的 `x,y`是相对位移, 单位(或许是)像素。

    而通过远程桌面登录时运行此程序，将会看到鼠标移动类型为 `absolute`, 此时的 `x,y`是归一化的绝对位置, 范围为0~65535(会有少量偏差), 0表示屏幕左/上, 65535表示屏幕右/下。

    此外, 还有 `CursorPos`表示鼠标在屏幕上的位置, 单位(或许是)像素。
  + `calibrate.exe`

    此程序应当在通过远程桌面登录时运行。此程序的作用是标定屏幕边界。如前所述，屏幕边界理论上为0和65535，但实际上会有少量出入。

    运行此程序，然后把鼠标移动到每个边界各一次，即可记录下实际边界。（保存于 `config.ini`中，没有则自动创建）。也可以运行 `mouse_info.exe`手动查看并记录。
    **每当使用新的设备登录远程桌面，都应该先标定一下屏幕边界。**
  + `main.exe`

    此程序应当在通过远程桌面登录时运行。程序启动后，就会开始接管远程主机的鼠标移动，为其附加相对位移。并且会在鼠标位于边界时也会产生相对位移。关闭程序即可停止接管。

    启动时会加载一些配置，可于 `config.ini`中修改。
+ 配置

  此程序使得远程桌面可以操作一些游戏，但是手感和本地还是有一些差距的。主要的问题出在本地鼠标移动到边界时（游戏内可能看不见）。这时，如果继续向着边界外移动，会出现：

  + 如果位移有沿着边界方向的分量，那么本地鼠标的位置产生了变化，远程主机可以收到信号，但是由于垂直于边界的方向上的绝对位置没有变化，所以无法计算这个方向的相对位移。
  + 如果位移完全垂直于边界，那么本地鼠标的位置不变，远程主机完全不会收到鼠标信号。

  对此，有两种方案：

  + 远程主机上收到信号时，判断是否位于边界，如果位于边界，则附加一个固定的向着边界外的相对位移。
    这要求操作鼠标时，移动方向不能完全水平/竖直，需要稍微倾斜一下，否则远程主机收不到信号。
    例如，向左转动视角，鼠标可以以一个微小的角度向左上或左下移动。如果完全水平向左移动，那么移到边界就转不动了。
    这种方案比较符合直觉，只要不移动鼠标就不会产生操作。但有时会产生意外的移动，例如左右移动视角时鼠标移动到了上下边界，会产生上下移动。
  + 远程主机时刻判断是否位于边界，如果位于边界，则持续附加一个位移（速度随着位于边界的时间递增）。
    这样就不必注意鼠标移动时需要倾斜。当鼠标移到了边界，视角就会开始自动转动，如果想要停止，鼠标稍微往里移一点，离开边界即可。
    这种方案会产生一些自动操作，与本地操作的直觉有一些出入，但配置好参数的话还是比较稳定的。

  两种方案都不是完美的。它们都有一些参数，可以在 `config.ini`配置，以决定边界处的移动速度。可以两种方案都体验一下，并根据实际情况调整速度参数，以找到最合适的方案。

  可配置的参数如下：

  ```ini
  # 边界标定信息，通常由标定程序获得
  [Boundary]
  xMin = 17
  xMax = 65566
  yMin = 30
  yMax = 65555

  [Main]
  factor = 1,0 # 非边界处的速度因数
  mode = 1 # 边界处的模式, 0表示需要手动倾斜移动，1表示自动移动

  [Manual]
  speed = 50 # 边界处手动移动时，向边界方向附加的固定位移

  [Auto]
  interval = 100 # 处于边界时每固定时间间隔发送一次位移，单位：毫秒
  # 边界处自动移动时的初始速度，末速度和速度变化步长
  speedStart = 30 
  speedEnd = 80
  speedStep = 10

  ```
