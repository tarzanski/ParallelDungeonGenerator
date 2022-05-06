# Parallel Dungeon Generation
Larry Geng and Aleksander Tarczynski

Final report: https://github.com/tarzanski/ParallelDungeonGenerator/blob/main/Final%20Project%20Report.pdf

Final video: https://drive.google.com/file/d/1xwf4xca5ZWAR-SLOHAwFs6nYDVThZbNI/view?usp=sharing
# Summary
We are going to develop a parallel version of a procedural 2-dimensional dungeon generation algorithm in an effort to find drastic improvements that would allow for much larger scale workloads. We plan to run the project on the NVIDIA GPUs on the GHC cluster and compare the results to that of an ISPC implementation run on the GHC cluster CPUs.
# Background
Many 2-dimensional video games use “dungeons” as their setting. Players move through rooms in the dungeon as they play through the game. Each room is often a different size or shape, and rooms are connected by hallways or other smaller rooms.

![Example dungeon map from “Enter the Gungeon”, https://www.boristhebrave.com/2019/07/28/dungeon-generation-in-enter-the-gungeon/](https://www.boristhebrave.com/wp-content/uploads/2019/07/gungeon_castle.png)
*Example dungeon map from “Enter the Gungeon”, https://www.boristhebrave.com/2019/07/28/dungeon-generation-in-enter-the-gungeon/*

Some games use algorithms to generate dungeons procedurally. One algorithm we found for generating dungeons can be found here: https://www.gamedeveloper.com/programming/procedural-dungeon-generation-algorithm. It is this algorithm that we want to accelerate by developing a parallel solution.

![Example generated map using our chosen algorithm](https://i.imgur.com/bSV1gpV.png)
*Example generated map using our chosen algorithm*

The algorithm involves many sequential steps. At a high level, these steps are:
1. Generate randomly sized rectangles placed in a circle
2. Push the rectangles away from each other until they no longer overlap
3. Filter out the “main rooms” of the dungeon
4. Construct a graph of the main room center points using Delaunay Triangulation
5. Generate a minimum-spanning tree for the graph
6. Add back some edges from the original triangulated graph
7. Map out hallways connecting the main rooms
8. Fill in the hallways using both leftover small rooms and additional tiles, as needed.

By parallelising this dungeon generation algorithm, we hope to enable more flexible usage of procedural dungeons. For example, a fast dungeon generation algorithm could let games create enormously large dungeons. Games could also generate many smaller dungeons at the same time. Having such options could help game creators better express their ideas.

We are not yet sure whether doing the generation on the GPU is worth the expense of copying data. One of our goals for this project is therefore to also evaluate the performance difference between CPU and GPU for dungeon generation.
# The Challenge
While there are many ways to connect rectangles together on a two dimensional plane, procedurally generating a set of rectangles that creates an enjoyable and interesting experience for a player can be more difficult and computationally intensive. Parts of the algorithm that we intend to use do not have obvious parallel implementations, and are expensive when done sequentially. Since there is a specific order to the generation algorithm, each step presents its own challenges.

Due to the nature of the problem, a lot of the steps present communication-heavy tasks that will require well thought-out parallel solutions. This includes the room separation, triangular graph and minimum spanning tree creation, and pathway generation. Each step is almost entirely different from both a resource perspective and an implementation perspective. The variation in how parallel processing can be used in each step is part of what makes this project so interesting to us. Some steps might yield higher performance gains than others as well, allowing for in-depth analysis as to how different parallelization methods improve performance on GPU vs CPU.

The given solution for the room separation step is “separation steering”, which involves both heavy dependencies and strong locality due to the overlapping rectangles. Moving the rectangles steadily away from each other involves a lot of communication about position and could be difficult to structure in parallel. The arithmetic intensity of the process is likely to be low, as the most intensive computation would be calculating which rectangles are overlapping.

One data-parallel algorithm for Delaunay triangulation is given here: (https://web.archive.org/web/20180425231851/https://www.cs.cmu.edu/~ygu1/paper/SPAA16/Incremental.pdf). This algorithm involves removing and replacing triangles around points, which creates dependencies when trying to operate over multiple points in parallel. Additionally, the authors note that implementing parallel algorithms efficiently in comparison to the many existing simple sequential solutions is difficult.

Finding a minimum spanning tree is a well-studied problem with many potential parallel solutions. One solution, Boruvka’s algorithm, involves communication between processors of edge and vertex information to perform graph contraction and finding light edges. Generating the hallways in the dungeon requires communication analogous to the VLSI wire routing in Assignments 3 and 4. Both of these steps involve levels of communication and locality that may make it challenging to implement well on CUDA and ISPC. Simpler solutions may prove to have better real-world performance.

Finally, a significant goal of our parallel implementation would be to drastically speed up the algorithm, generating hundreds of rooms compared to the tens that the sequential algorithm was used for. Thus the parallel algorithm needs to be very memory efficient when dealing with the large number of data structures.
# Resources
We plan to use the GHC cluster machines for our project. The GHC cluster machines should allow us to sufficiently test both CUDA and ISPC implementations.

We will use the gamedeveloper.com post linked above as a starting point, which offers pseudocode for the dungeon generation algorithm. Our first action item will be to complete a working sequential implementation, which we do not anticipate much difficulty with.
# Goals and Deliverables
## 75%:
- A working parallel GPU (or CPU) implementation.
- A Basic UI to show each step of the algorithm.
- Have the parallel implementation be efficient enough to quickly (under 30s) generate structures with 200+ rooms. (4x sample problem)
- Performance analysis compared to sequential version

## 100%:
- Working parallel GPU and CPU (ISPC) implementations.
- Have the parallel implementation be efficient enough to quickly (under 30s) generate structures with 400+ rooms. (8x sample problem)
- Performance analysis and comparison for GPU, CPU, and sequential.
- In-depth GUI with interactivity (zoom, panning), and step-by-step animations.

## 125%:
- Parameter tweaking in the GUI
- 1000+ room working (slow but within 5 minutes) structure generation.
- (if going really well) full 3D dungeon generation with 3D GUI.

## Demo:
We plan to use the interactive GUI during the demo session to show off our project in motion. This will allow us to show the different steps of the algorithm, and any possible implementation optimizations to the original algorithm that we found along the way. The interactivity of the GUI would be great for allowing students/staff to interact with this project themselves.

We plan on presenting graphs of speedup on both the CPU and GPU, as well as speedup of individual steps of the algorithm if it yields interesting results.

# Platform Choice
The GHC cluster machines fit our workload very well. They contain powerful but consumer-grade components, which match the workload of generating dungeons for a video game. Most people playing video games likely do so on equivalent or lesser hardware.

The GHC cluster machines allow us to attempt a variety of parallel implementations. We will be able to work with both CPU and GPU resources. Since we also want to make a GUI for our project, the GHC machines should allow us to develop one without much hassle.

# Schedule
**Mar 21 - Mar 27**  
Project proposal

**Mar 28 - Apr 3**  
Sequential implementation, simple output visualization

**Apr 4 - Apr 10**  
Parallel implementation on GPU, checkpoint

**Apr 11 - Apr 17**  
Parallel implementation on CPU/ISPC

**Apr 18 - Apr 24**  
Interactive GUI, performance analysis, final report

**Apr 25 - Apr 29**  
Final report, poster,  and presentation

# MILESTONE
## Work Completed
We did not have as much time to work on our project as we expected, and have not met the deliverables stated on our schedule. So far, we are partially done with the sequential implementation and are working on visualizing our partial output. We think producing a nice visualization early on would help accelerate later development stages. Our unfamiliarity with graphical visualization using C++ has slowed us down quite a bit. We have managed to install a graphics library onto the AFS machines and hope to make faster progress soon.

## Updated Goals
We still believe we can meet all of our 100% goals, and we hope to add parameter tweaking to our visualization as well. We will not be planning to accomplish anything in 3D.
- Working sequential implementations
- Working parallel GPU and CPU (ISPC) implementations.
- Have the parallel implementation be efficient enough to quickly (under 30s) generate structures with 400+ rooms. (8x sample problem)
- Performance analysis and comparison for GPU, CPU, and sequential.
- In-depth GUI with interactivity (zoom, panning), and step-by-step animations.
- Parameter tweaking in the GUI

## What we plan to show at the poster session
Our main demo will be a visualization of dungeon generation given certain parameters. We want to have two versions of the generation: one sequential and one parallel, so that any speed difference is obvious. We also plan to have graphs showcasing speedup.

## Issues that concern us
We are somewhat concerned about implementing the more complicated graph algorithms using CUDA. There are some examples online that could help us, however, so we hope this is not much of an issue.

# Updated Schedule
**Mar 21 - Mar 27**  
Project proposal

**Mar 28 - Apr 11**  
Sequential implementation, simple output visualization

**Apr 12 - Apr 15**  
Sequential implementation (Larry), simple output visualization (Aleksander)

**Apr 16 - Apr 18**  
Parallel implementation on GPU/CUDA (Larry), Parallel implementation on CPU/ISPC (Aleksander)

**Apr 18 - Apr 22**  
Parallel implementation on GPU/CUDA (Larry), Parallel implementation on CPU/ISPC (Aleksander)

**Apr 23 - Apr 25**  
Interactive GUI (Aleksandr), performance analysis (Larry)

**Apr 26 - Apr 29**  
Final report, poster, and presentation (both)
