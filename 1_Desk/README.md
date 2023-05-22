# *Desk*
This tutorial illustrates how to program the *Franka Emika AIR Platform* with *Desk*.

## Getting started
Make sure you are connected to the robot, either to the *Arm* base or to *Control*. Then enter the robot ip in a web browser to open *Desk*. Your robot ip will be:

* `robot.franka.de` if you are connected to the *Arm* base, which is running a DHCP server.
* The already configured shopfloor IP if you are connected to *Control*. If you don't know your shopfloor IP you can always connect your PC to the *Arm* base first and configure it in the Admin menu of *Desk*.

## Programming with Desk
This tutorial implements the plug in task using *Desk* as described in Section III-A in the [Tutorial Paper](../README.md#tutorial-paper). </br>
The plug in task is programmed using the following three basic Apps included in the Research Apps bundle:

* The *Cartesian Motion App* will position the robot in its initial pose.
* The *Move to Contact App* will move the robot towards the socket until it detects a contact.
* The *Lissajous* will regulate a constant force in z-axis and produces a wiggle motion in the `(x,y)` -plane for `n` seconds to insert the plug.

Drag and drop the three Apps from the App pane to the timeline one after another.

With *Desk* you can also **download** and **import** your tasks. The `plug-in.task` file included in this folder is a preprogrammed timeline for the task described in the tutorial.
To import it, just drag the `plug-in.task` file and drop it into the task area in *Desk*. Please make sure that *CART MOTION*, *MOVE TO CONTACT* and *LISSAJOUS* apps are already available in desk.

## Parameterizing the task
In order to run the plug in task in your custom setup you will have to first parameterize each App. If you imported the task file included in this repo, the parameters will not fit your setup so you will also require a reparameterization. You can do it using the web interface and your mouse but we recommend using the *Pilot* buttons:

1. Click on the *Cartesian Motion App* and guide the robot to a position above the hole while the robot is holding the plug between its fingers. Press the <span style="color:blue">O</span>  button in the *Pilot* to store poses or the <span style="color:red">X</span> button to delete them. To set the velocity and acceleration factors you can navigate the menus with the directional pad
<span>&#10133;</span> and validate with the <span style="color:green"> &#10004; </span>.

2. Click on the *Move to Contact App* and guide the robot to the socket pose by inserting the grasped plug into the socket. The context menu of this App is very similar to the previous one.

3. Click on the *Lissajous App* and set the wiggle and the force until the plug is successfully inserted. To set the desired wiggle motion and force/torque values you can navigate the menus with the directional pad <span>&#10133;</span> and validate with the <span style="color:green"> &#10004; </span>.  For the wiggle motion you need to configure the number of loops and size of each axis. A good starting point is 3 loops in one axis and 2 in the other one. Try to keep the size as small as possible! Start here with a size of 1 and increase the values carefully if required. In a table-top configuration select the `(x,y)` - plane in which to move.
Moreover you will only need positive force values in the `z` axis. Try to keep the force as small as possible! Start with 5 N and increase the values carefully if needed. Also increase the collision thresholds accordingly to avoid any undesired errors. If after trying high values for the forces the robot is still not able to insert the plug, try teaching again the wiggle motion and/or the poses from the first and the second App.

## Running the task

You can run the task in two different ways:

1. **Using *Desk* (web-based interface):**</br>
   First open *Desk* in a web browser, activate the external activation device (robot lights should be blue) and press the run button on the side bar.
2. **Using the `ride-cli` command-line tool:** </br>
    You can also use the command-line tool `ride-cli` to start your task from a Linux terminal. To do so, open a terminal and connect to the *Core* by using the following command:

    ```sh
    ride login <robot_ip>
    ```

    To run the task activate the external activation device, stop any execution currently running (e.g. idle state machine) by

    ```sh
    ride cli stop
    ```

    and then enter

    ```sh
    ride cli start 0_plug-in
    ```
