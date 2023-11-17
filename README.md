Kelvin Jeon, Nicholas Veselskiy, Kevin Benitez, Benjamin Barbeau
Assignment 6

Nick
Added the ability to load terrain data from a basic custom heightfield map format using the resource manager and to render it using a basic terrain shader.

Heightfield (kelvin)
Created a Python script that produced the terrain.heightfield file present in the resources folder. The heightfield included simple craters/hills to give the sense and feel of the general shape of the moon's surface.

Kevin
Added a new function in camera.h/.cpp called UpdateYPos() that finds the player's surrounding vertices and corrects the y position of the camera.

Ben
Edited resource_manager to add a function called GetImpassableCells to create a
grid showing the impassable cells in the terrain.

To determine whether a cell is impassable it first takes data from a impassable.csv
file and sets the cell to impassable if the corresponding cell in the csv is set to 1.

Then it calculates the height differences between the points on each cell
and checks if the maximum of those is higher than a given threshold. If so it marks the cell as impassable.

Camera.cpp was modified to check to see if the target cell the player would move
to is set to impassable from the impassable grid that got constructed and if so returns early to prevent movement.
