# Assignment 3: Physically-Based Simulation
<br>

In this assignment, I implemented the following functionalities:
* Three types of integrators:
  * Euler integration
  * Trapezoidal integration
  * RK4 integration

* Three types of systems:
  * **Simple system**: a sphere in circular motion
  * **Multi-pendulum system**: a pendulum consisting of multiple particles in a chain of springs, involving gravity force, viscous drag force, and spring force
  * **Cloth**: simulating a piece of cloth, a mass-spring with an 8 by 8 grid of particles, with structural, shear, and flexion springs

* Wired the `R` key to reset the cloth, which then naturally falls out of screen due to gravity force;

* Wired the `W` key add a random wind to the cloth system.
<br><br>


### Compile & Run Code

I followed the steps on the assignment page to compile and run my code, and I didn't run into any compilation error. I used MacOS.
<br><br>
For viewing all three systems together, RK4 integrator along with step size 0.005 works well.
<br><br>


### Collaboration

I collaborated with Liane Xu, Karen Ma, and Alan Yu for general debugging. I went to Office Hours a couple times.
<br><br>


### References

No outside reference used.
<br><br>


### Known Problems with My Code

There is no known problem with my code. **Screenshots of the three systems are in [the Images folder](/images/)**. Note that use mouse to change the view direction a bit to get a clearer view of the cloth. To better see that normals are correctly implemented for shading, it's easier to turn on wind with `W` key to add some movement to the cloth.
<br><br>


### Extra Credit

I completed two easy extra-credit problems:
* Rather than display cloth as a wireframe mesh, I implemented smooth shading. I used the code for rendering patch from Assignment 1, as well as the code for computing surface normals from Assignment 2;
* Added a random wind force to my cloth simulation. Can toggle it on and off using `W` key.
<br><br>


### Comments

I enjoyed this assignment overall.
