# RDP Relative Mouse

> This page is mostly translated by machine.

RDP only sends the absolute position of the mouse, which makes it impossible to operate a 3D game (unable to rotate the perspective) that depends on the relative displacement of the mouse, such as Genshin Impact, in the remote desktop.

This project uses two hooks to input relative displacement in the remote desktop, allowing players to play some games on the remote desktop. The test experience in Genshin Impact is good.

+ Principle

  When using remote desktop, when the position of the local mouse changes, the remote host will receive input containing absolute location information. This project intercepts this input on a remote host using a mouse hook and generates an input containing relative displacement, allowing for game control. Due to the fact that remote hosts only receive input when the local mouse position changes, if the mouse is located at a boundary/corner,  the mouse position does not change, the remote host will not receive input. To solve this problem, a mouse hook was run on the local computer. When the mouse moves to the boundary of the screen, it appears on the other side of the screen, repeating in a loop to ensure that the position of the local mouse can always change.
+ Usage

  Compile two executable programs and run them on different computers:

  + Run `MouseLoop.exe` on the local computer. After running, when the mouse moves to the screen boundary, it will come out from the other side.
  + `RelativeMove.exe` runs on a remote host **with administrator privileges**. After running, you can control games like Genshin Impact. After connecting to a remote host using different clients, this program should be restarted to ensure that the obtained screen resolution is correct.
+ Some implementation details

  + The program uses `MSLLHOOKSTRUCT` and `GetCursorPos/SetCursorPos`, which use different scales of mouse coordinates (screen resolution) and are obtained through `GetDeviceCaps` and `GetSystemMetrics`, respectively. The former is the physical resolution of the display, while the latter is the resolution perceived by the application. The difference between the two is the system's scaling factor.
  + When using the `SendInput` function on a remote host to send relative movement information, this information will also be intercepted by the mouse hook. The value of `flag` can be used to determine whether it is sent as `SendInput`. Moreover, even if this information is not transmitted to the next hook, Genshin Impact can still read this input. So during implementation, this information was not passed on to the next hook, avoiding the situation of local and remote mouse misalignment. Moreover, if not passed to the next hook, it will not affect most other programs, ensuring that most other programs can still be used normally.
