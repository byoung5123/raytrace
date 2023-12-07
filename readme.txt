# raytrace

raytrace is a simple C++ program that implements a basic ray tracing algorithm to render 3D objects in a scene. The program uses the SDL2 library for graphics rendering.

## Introduction

This program simulates the rendering of 3D scenes by casting rays from a virtual camera and calculating the interaction of these rays with objects in the scene. The program supports spheres and triangles as primitive objects and implements shading with ambient, diffuse, and specular components and reflections.

## Features

- Basic ray tracing algorithm.
- Support for spheres and triangles as primitive objects.
- Can read objects from text files containing triangle vertices
- Shading with ambient, diffuse, and specular components and reflections.
- Adjustable camera position, light source, and object properties.

## Getting Started

### Prerequisites

- C++ compiler
- SDL2 library

### Installation

1. Clone the repository:

   git clone https://github.com/byoung5123/raytrace.git

2. Navigate to the project directory:

   cd raytrace

3. Build with CMake:

   cmake .

4. Compile the program:

   make

5. Run the executable:

   ./raytrace

### Customisation

You can customize the scene and rendering parameters by modifying the code in the main.cpp file. Adjust the camera position, light source properties, and object properties to create different scenes.

### Object files

The program reads triangle vertices from a file named objectLite.txt. You can replace this file with your own text file. The format of each line in the file is as follows:

x1,y1,z1 x2,y2,z2 x3,y3,z3

This format defines a triangle with vertices at (x1, y1, z1), (x2, y2, z2), and (x3, y3, z3).