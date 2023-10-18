# Assignment 2: Hierarchical Modeling & SSD
<br>

In this assignment, I implemented the following functionalities:
* Rendered skeleton using hierarchical modeling:
  * Local-to-world transformation for all joints;
  * Optimization for computing transformation using an implicit matrix stack;
  * Handled UI to update skeleton positions;

* Implemented skeleton subspace deformation (SSD):
  * Attached skin mesh to skeleton, where each vertex is attached to neighboring joints according to attachment weights;
  * Computed normals (per-face normals, and then per-vertex normals) for shading;

* Wired the `S` key to toggle between skeleton view & SSD view.
<br><br>


### Compile & Run Code

I followed the steps on the assignment page to compile and run my code, and I didn't run into any compilation error.
<br><br>


### Collaboration

I went to Office Hours a couple times for debugging. I collaborated with Liane Xu and Karen Ma on some parts.
<br><br>


### References


<br><br>


### Known Problems with My Code

There is no known problem with my code.
<br><br>


### Extra Credit


<br><br>


### Comments

I enjoyed this assignment overall, as it really led me to understand the math underlying Bezier & B-Spline curves as well as patches. But the instructions for the last problem — creating two artifacts — were very vague, and I wasn't sure how to get started on computing control points for patches even though we could reference outside resources. It'd be better if it's an extra-credit problem.
