# RDP Relative Mouse

> This page is mostly translated by machine.

RDP only sends the absolute position of the mouse, which makes it impossible to operate a 3D game (unable to rotate the perspective) that depends on the relative displacement of the mouse, such as Genshin Impact, in the remote desktop.

This program uses the [Interception library](https://github.com/oblitum/Interception), intercept the absolute position of the mouse received on the remote host, and attach a relative movement to it, so that some 3D games(at least Genshin Impact) can be operated through the remote desktop.

+ Principle

  When using remote desktop, the mouse we see is a local mouse. On the remote host, there will be a virtual mouse device. When the local mouse moves in the remote desktop window, the remote virtual mouse device will generate a corresponding input signal that only includes the absolute position of the mouse.

  Under normal circumstances, the position of the local mouse and the position of the remote mouse always coincide.

  This program intercepts the input signal of the virtual mouse on a remote host, calculates the relative displacement based on the absolute position of the previous input signal, and generates a relative displacement signal.

  In addition, the remote host only receives a signal when the position of the local mouse changes in the remote desktop window. If the local mouse is located at the window boundary or corner and continues to move towards the outside of the boundary, and the actual position of the mouse in the window does not change, then the remote host will not receive a signal.

  Therefore, this program will also determine whether the mouse position is located at the screen boundary. If it is located at the boundary, special processing will be performed to generate a relative displacement signal.
+ Installing Interception Driver

  The Interception library that this program relies on requires a driver to be installed in order to run properly. Go to [Release v1.0.1 · OBLITUM/Interception](https://github.com/oblitum/Interception/releases/tag/v1.0.1) Download the compressed package and unzip it, enter the 'command line installer' directory, open a command line with administrator privileges, and run the command to install:

  ```

  ./install-interception.exe /install

  ```

  Restarting the computer takes effect.
+ Build

  Using CMake to build this program,  `mouse_info.exe ` ,`calibrate.exe`, and `main.exe` will be generated in the `bin` directory.
+ Run

  Copy 'interception.dll' to the directory of 'exe' or add it to 'path'.

  + `mouse_info.exe`

    Running this program will output real-time mouse movement information in the terminal.

    Generally, when running this program on the local desktop, you will see that the mouse movement type is `relative`, and at this point `x, y`are relative displacements, in pixels (perhaps).

    When running this program through remote desktop login, you will see that the mouse movement type is `absolute`, where  `x` and `y` are normalized absolute positions, ranging from 0 to 65535 (with a slight deviation), where 0 represents the left/top of the screen and 65535 represents the right/bottom of the screen.

    In addition, `CursorPos` represents the position of the mouse on the screen, in pixels (perhaps).
  + `calibrate.exe`

    This program should run when logging in through remote desktop. The purpose of this program is to calibrate screen boundaries. As mentioned earlier, the screen boundaries are theoretically 0 and 65535, but in reality there may be slight discrepancies.

    Run this program, and then move the mouse to each boundary once to record the actual boundary. (saved in `config.ini`, automatically created if not present). You can also run `mouse_info.exe` Manually view and record.

    **Whenever logging into a remote desktop using a new device, the screen boundaries should be calibrated first**
  + `main.exe`

    This program should run when logging in through remote desktop. After the program starts, it will begin to take over the mouse movement of the remote host and attach relative displacement to it. And it will also generate relative displacement when the mouse is at the boundary. Close the program to stop taking over.

    Some configurations will be loaded during startup and can be modified in `config.ini`.
+ Configuration

  This program allows remote desktops to operate some games, but there are still some differences in the feel compared to local games. The main issue is when the local mouse moves to the boundary (which may not be visible within the game). At this point, if you continue to move outside the boundary, you will see:

  + If the displacement has components along the boundary direction, the position of the local mouse changes, and the remote host can receive the signal. However, since the absolute position perpendicular to the boundary does not change, the relative displacement in this direction cannot be calculated.
  + If the displacement is completely perpendicular to the boundary, the position of the local mouse remains unchanged and the remote host will not receive mouse signals at all.

  There are two options for this:

  + When a signal is received on the remote host, it is determined whether it is at the boundary. If it is at the boundary, a fixed relative displacement towards the outside of the boundary is added.

    This requires that when operating the mouse, the direction of movement should not be completely horizontal/vertical, and it needs to be slightly tilted, otherwise the remote host will not receive the signal.

    For example, by turning the perspective to the left, the mouse can move up or down to the left at a small angle. If it moves completely horizontally to the left, then it cannot turn when it reaches the boundary.

    This plan is more intuitive, as long as the mouse is not moved, there will be no operation.But sometimes unexpected movements can occur, such as when the mouse moves to the upper and lower boundaries while moving the perspective left and right, causing up and down movement.
  + The remote host constantly determines whether it is at the boundary, and if it is at the boundary, continuously adds a displacement (the speed increases with the time it is at the boundary).

    This eliminates the need to tilt the mouse when moving. When the mouse moves to the boundary, the perspective will start to automatically rotate. If you want to stop, move the mouse slightly inward and leave the boundary.

    This scheme will generate some automatic operations, which may differ from the intuition of local operations, but if the parameters are configured properly, it is relatively stable.

  Neither solution is perfect. In addition, they all have some parameters that can be configured in `config.ini` to determine the movement speed at the boundary. You can experience both options and adjust the speed parameters according to the actual situation to find the most suitable solution.

The configurable parameters are as follows:

```Ini

#Boundary calibration information, usually obtained by calibrate.exe

[Boundary]

xMin = 17

xMax = 65566

yMin = 30

yMax = 65555


#main configurations
[Main]

factor = 1.0 # velocity factor at non boundary points

mode = 0 # The mode at the boundary, where 0 indicates manual movement and 1 indicates automatic movement



[Manual]

speed = 50 # Fixed displacement added to the boundary direction when manually moving at the boundary



[Auto]

interval = 100 # Send displacement every fixed time interval when at the boundary, in miliseconds

speedStart = 30

speedEnd = 80

speedStep = 10

```
