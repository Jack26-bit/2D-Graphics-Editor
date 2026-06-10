# 2D Graphics Editor in C

A menu-driven **2D Graphics Editor** implemented in C using a character-based canvas. The program allows users to create, modify, delete, and manage graphical objects such as lines, rectangles, triangles, and circles using classic computer graphics algorithms.

## Features

### Canvas Management

* Configurable canvas size (default: 50 × 20)
* Character-based drawing surface
* Empty cells represented by `_`
* Drawn pixels represented by `*`
* Automatic canvas redraw after modifications

### Supported Shapes

* Line
* Rectangle
* Triangle
* Circle

### Graphics Algorithms

* **Bresenham's Line Algorithm** for line drawing
* **Midpoint Circle Algorithm** for circle drawing
* Rectangles rendered using four lines
* Triangles rendered using three lines

### Object Management

Each graphical object is stored with:

* Unique Object ID
* Shape Type
* Coordinates
* Dimensions / Radius
* Visibility Status

Supports:

* Add Object
* Delete Object
* Modify Object
* List All Objects

### Menu Operations

```text
1. Display Canvas
2. Draw Line
3. Draw Rectangle
4. Draw Triangle
5. Draw Circle
6. Delete Object
7. Modify Object
8. List All Objects
9. Clear Canvas
10. Exit
```

### Error Handling

* Boundary checking for canvas limits
* Validation of user inputs
* Invalid object ID detection
* Safe object management

---

# Project Structure

```text
graphics_editor.c
│
├── initializeCanvas()
├── clearCanvas()
├── displayCanvas()
├── drawPixel()
│
├── drawLine()
├── drawRectangle()
├── drawTriangle()
├── drawCircle()
│
├── addObject()
├── deleteObject()
├── modifyObject()
├── redrawCanvas()
├── listObjects()
│
└── main()
```

---

# Data Structures

### Shape Enumeration

```c
typedef enum {
    LINE,
    RECTANGLE,
    TRIANGLE,
    CIRCLE
} ShapeType;
```

### Graphical Object Structure

```c
typedef struct {
    int id;
    ShapeType type;

    int x1, y1;
    int x2, y2;

    int x3, y3;

    int width;
    int height;
    int radius;

    int visible;
} GraphicObject;
```

---

# Compilation

Compile using GCC:

```bash
gcc graphics_editor.c -o graphics_editor
```

Run:

```bash
./graphics_editor
```

For Windows (MinGW):

```bash
gcc graphics_editor.c -o graphics_editor.exe
graphics_editor.exe
```

---

# Sample Execution

```text
=================================
      2D GRAPHICS EDITOR
=================================

1. Display Canvas
2. Draw Line
3. Draw Rectangle
4. Draw Triangle
5. Draw Circle
6. Delete Object
7. Modify Object
8. List All Objects
9. Clear Canvas
10. Exit

Enter choice: 2

Enter x1 y1: 2 2
Enter x2 y2: 20 10

Line created successfully.
Object ID = 1
```

Canvas Output:

```text
   012345678901234567890123456789
0  ______________________________
1  ______________________________
2  __**__________________________
3  ____**________________________
4  ______**______________________
5  ________**____________________
6  __________**__________________
7  ____________**________________
8  ______________**______________
9  ________________**____________
10 __________________**__________
```

---

# Time Complexity

## Line Drawing (Bresenham)

```text
Time  : O(max(dx, dy))
Space : O(1)
```

## Rectangle Drawing

Uses four line draws:

```text
Time  : O(width + height)
Space : O(1)
```

## Triangle Drawing

Uses three line draws:

```text
Time  : O(side1 + side2 + side3)
Space : O(1)
```

## Circle Drawing (Midpoint Circle)

```text
Time  : O(radius)
Space : O(1)
```

## Canvas Redraw

```text
Time  : O(number_of_objects × drawing_cost)
Space : O(canvas_size)
```

---

# Memory Usage

Canvas:

```text
50 × 20 = 1000 characters
```

Object Storage:

```text
Maximum Objects = 100
```

Overall memory usage is small and suitable for educational purposes and laboratory assignments.

---

# Bonus Features (Optional Extensions)

* Save canvas to file
* Load canvas from file
* Undo operation
* Redo operation
* ncurses-based graphical menu
* Shape visibility toggle
* Filled shapes
* Color support using ANSI escape codes
* Object layering (z-index)

---

# Educational Objectives

This project demonstrates:

* 2D Graphics Fundamentals
* Bresenham Line Drawing Algorithm
* Midpoint Circle Algorithm
* Arrays and Structures
* Dynamic Object Management
* Menu-Driven Programming
* Data Structures in C
* Computer Graphics Concepts

---

# Author

Data Structures & Graphics Programming Laboratory Project

Language: **C (ANSI C / GCC Compatible)**

Suitable for:

* B.Tech / B.E. Computer Science
* Data Structures Laboratory
* Computer Graphics Laboratory
* Programming Fundamentals Coursework
