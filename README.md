## kinnect-body-tracker
this project use the Boost library to implements the networking function. So the first step is to setup the Boost environment in windows, you can find the instructions from the link: https://www.boost.org/doc/libs/1_60_0/more/getting_started/windows.html 

## Add Dependent Libraries

right click the project to select the "manage nuget packages" to install the dependent libraries for kinect development.

<img src="https://github.com/SlumDunk/kinnect-body-tracker/blob/master/docs/denpendencies.PNG?raw=true" alt="image-20200506100053755" style="zoom: 67%;" />

## OS requirement
Windows 10


## tips
In this project, everytime when we run the program, we will dump a file that records the position of every frame in a file named "frame"+currentTimeStamp.json under the path "D:\data\kinect\src\"+currentDate. when you get this file, you need to add the "]}" to make it
as a valid json format file. And then you can import it into the AR_VR program.