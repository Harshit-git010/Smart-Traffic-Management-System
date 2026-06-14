# Smart-Traffic-Management-System
 <img width="1917" height="910" alt="Screenshot 2026-06-14 131258" src="https://github.com/user-attachments/assets/e8b8824f-d45e-4f9b-b828-724da00ed1e6" />



I build a code for smart traffic management system using arduino mega <br>
We have four lanes (alpha , beta , gamma , delta) each with a rgb light and a ultra sonic sensor<br>

<br>
The IR sensor counts the vehicle in each lane before it passes through the signal. <br>
According to the vehicle count in the each lane, the system manipulates the traffic signal timings.<br>
<br>
If a lane is open it goes green and if a lane is closed it goes red.<br>
Each lane opens once (green light), followed by others in squence of alpha, beta, gamma , delta <br>
<br>
Currently we are not taking any readings from the IR sensor. Hence, take a random number of vehicles in each lane each time and manipulate the timings based on the vehicle count<br>
The number of vehicles in each lane should always in between (3-15) <br>
Time taken by each vehicle to cross the signal = 7 seconds <br>
Total time for a signal to open = (number of vehicles in that lane * time taken by each vehicle) + 8 seconds (buffer time)<br>
<br>
#These timings can be changed according to weather and the condition<br>
The lcd screen (16x2 I2C)  reflect, which signal is in the green state, timing (just like countdown after which it goes red), and the vehicle count of that signal.<br>
Signal names are alpha , beta , gamma , delta.<br>
<br>

The pinouts are <br>
for alpha -> red 5, green 7<br>
for beta -> red 8, green 10<br>
for gamma -> red 2, green 4<br>
for delta -> red 11, green 13 <br>
<br>
On each signal there is a slide switch (on INPUT_PULLUP), in case of a emergency we can make any of the lane green (making others blue reflecting emergency) by sliding the switch.<br>
It helps easy for emergency vehicles to pass without delay.<br>
During emergency the lcd display reflects lane with emergency.<br>
After the switch is slide back to normal, the default traffic management system works as it is.<br>
<br>
The pinouts are <br>
for alpha -> blue 6, emergency 14<br>
for beta -> blue 9, emergency 15<br>
for gamma -> blue 3, emergency 16<br>
for delta -> blue 12, emergency 17<br>
<br>

# Simulation Link:
https://wokwi.com/projects/432996883110376449

