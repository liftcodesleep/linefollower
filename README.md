[Demo run video!](https://youtu.be/ZOHZrwPSCeM)

I built this project to run a line following and obstacle avoiding robot.

The robot hardware I used can be found here: [The perseids DIY Robot Smart Car Chassis Kit for Raspberry Pi](https://www.amazon.com/gp/product/B07DNXBFQN)

The computer I used was a [Raspberry Pi model B](https://www.raspberrypi.com/products/raspberry-pi-4-model-b/)

An array of three [TCRT5000 IR sensors](https://www.amazon.com/HiLetgo-Channel-Tracing-Sensor-Detection/dp/B00LZV1V10) and one [HC-SR04 ultrasonic sensor](https://www.amazon.com/EPLZON-HC-SR04-Ultrasonic-Distance-Arduino/dp/B09PG4HTT1/) were used to follow the line and detect obstacles, respectively.

There were also external batteries used to power the Pi and the motors.

To run the program, build the car and wire it according to main.h, or whatever your preference provided you update the file as necessary.</br>
Clone the repository directly onto the Raspberry Pi, then cd into /src and run 'make' and then sudo ./main</br>
At this point the car should begin to run a course provided it detects one, or search for one if it does not.

Some notes:
The run can be linked to the press of a button by uncommenting the debounce block near the start of main()

The code is somewhat hardware specific, abstractions were made where feasible, but it's possible that with different motors or batteries the timings passed to the motion functions will need to be adjusted.

The performance of the HC-SR04 sensor can be dramatically improved with the use of makeshift paper tubes as described in [this excellent article](https://www.davidpilling.com/wiki/index.php/HCSR04)</br>
The tubes were sufficient to negate the need for a larger sonic sensor array, but did create failure modes where when the car would bounce on uneven terrain the tubes would shift in such a way as to seem to indicate an obsacle. A sturdier connection than the electric tape I used would likely prevent this.
