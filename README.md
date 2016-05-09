# csc418
CSC418 - Computer Graphics 

Assignment 3

Part 1 

Implement a basic ray-tracer and render a simple scene using ray casting and local shading. 
The starter code sets up a scene comprising of an ellipsoid and a plane, being illuminated by a point light source. 
Render the scene by implementing code fragments required for object intersections and Phone Shading. 

Part 2 - Dodge ball game (game folder)
Using OpenGL to produce an interactive game with content learned throughout the entire course. 



Dodge the ball game: 
	
	Instructions: 
		1) Press Space to begin the game and again to quit the game window. 
		2) Move with the numberpad with a keyboard (8, 2, 4, 6) to move up, down, left,
		    and right.
		3) Dodge all the balls and reach the end!

		Note: To load Dodge, type "make" in the "game" directory to compile, then "dodge"
		to run the actual game. 

We decided to adopt the structure of assignment 2 for this part. The objective is to move to
the top of the ramp without getting hit by a ball. We have 10 balls randomly spawning in the
top section of the ramp. We detect collisions with the player every time we animate the next
frame. Controls are the numpad numbers, excluding 5 (8, 2, 4, 6 for up, down, left, and right
and 1, 3, 7, 9 for the diagonals).
