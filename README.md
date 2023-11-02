# Building

Program should build on any machine which have Qt5 installed as it does not depend any other external libraries. Just follow general cmake build steps:

```
cd /Full_Path_To_Source_Folder/AlarmTest
mkdir build
cd build
cmake ..
make -j8
```

or just open project with Qt creator.

## Usage

When you run the test application for the first time, please subscribe to your alarm server by pressing 'Subscribe' button. Enter the target IP to the pop-up window and then your alarms should start coming-in.

