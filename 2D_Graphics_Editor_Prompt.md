# SYSTEM ROLE
You are an expert C programmer and systems-level educator with deep knowledge of data structures, graphics algorithms, and low-level memory management. You write production-quality, well-commented C code that is modular, portable, and pedagogically clear. Your output is always complete, compilable with GCC, and suitable for academic submission in a Data Structures and Graphics Programming laboratory course.

---

# TASK
Build a **complete, fully functional, menu-driven 2D Graphics Editor** written entirely in **standard C (C99)**. The program must compile without warnings using:
```
gcc -std=c99 -Wall -Wextra -o graphics_editor graphics_editor.c
```
For the bonus ncurses build:
```
gcc -std=c99 -Wall -Wextra -o graphics_editor_ncurses graphics_editor.c -lncurses
```

---

# TECHNICAL SPECIFICATIONS

## 1. Canvas
- Represent the drawing surface as a **global 2D character array**: `char canvas[ROWS][COLS]`
- Default dimensions: `COLS = 60`, `ROWS = 22` — define both as **macros** so they are trivially configurable
- Initialization: fill every cell with `'_'` (ASCII 95) via `initializeCanvas()`
- Drawing pixel: use `'*'` (ASCII 42)
- **Boundary rule**: any pixel whose coordinates fall outside `[0, ROWS-1] x [0, COLS-1]` must be **silently clipped** — never cause a buffer overrun

## 2. Shape Functions
Implement exactly these function signatures:

```c
void drawLine(int x1, int y1, int x2, int y2);
void drawRectangle(int x, int y, int width, int height);
void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
void drawCircle(int cx, int cy, int radius);
```

### Algorithm Requirements
| Shape      | Algorithm                        | Notes                                                      |
|------------|----------------------------------|------------------------------------------------------------|
| Line       | Bresenham's Line Algorithm       | Handle all octants; steep/shallow slope cases              |
| Circle     | Midpoint Circle Algorithm        | Plot all 8 symmetric points; clip to canvas                |
| Rectangle  | Four calls to `drawLine()`       | Sides: top, bottom, left, right                            |
| Triangle   | Three calls to `drawLine()`      | Connect vertices: (P1→P2), (P2→P3), (P3→P1)              |

### `drawPixel()` — the only function that writes to canvas
```c
void drawPixel(int x, int y);
// Sets canvas[y][x] = '*' only if 0 <= x < COLS && 0 <= y < ROWS
// ALL draw functions must go through drawPixel exclusively
```

## 3. Object Management

### Enum for Shape Types
```c
typedef enum {
    SHAPE_LINE,
    SHAPE_RECTANGLE,
    SHAPE_TRIANGLE,
    SHAPE_CIRCLE
} ShapeType;
```

### Object Struct
```c
typedef struct {
    int      id;            // Unique auto-incremented ID (starts at 1)
    ShapeType type;         // Shape category
    int      x1, y1;        // Primary coordinate / center (circle)
    int      x2, y2;        // Secondary coordinate (line, rectangle width/height)
    int      x3, y3;        // Tertiary coordinate (triangle only)
    int      extra;         // radius (circle) — unused for other shapes, set to 0
    int      visible;       // 1 = visible, 0 = hidden/deleted
} GraphicObject;
```

### Object Array
```c
#define MAX_OBJECTS 100
GraphicObject objects[MAX_OBJECTS];
int objectCount = 0;        // tracks number of objects ever added (not current active count)
int nextID = 1;             // auto-increment counter
```

**Deletion strategy**: mark `visible = 0` (logical delete). Do **not** compact the array on delete. When redrawing, skip objects where `visible == 0`.

## 4. Core Function Contracts

```c
void initializeCanvas();
// Fills canvas[ROWS][COLS] with '_'

void clearCanvas();
// Calls initializeCanvas() — resets drawing surface without touching objects[]

void displayCanvas();
// Prints the canvas with:
//   - A column index header (tens digit on first line, units on second)
//   - A row index prefix (zero-padded 2-digit row number + '|')
//   - A bottom border line of '-' characters
// Example header (cols 0-9):
//   "    0         1"
//   "    0123456789..."

void redrawCanvas();
// 1. Calls clearCanvas()
// 2. Iterates objects[] from index 0 to objectCount-1
// 3. For each object where visible == 1, calls the appropriate draw function
// This is the ONLY correct way to refresh the canvas after any state change

void addObject(ShapeType type, int x1, int y1, int x2, int y2, int x3, int y3, int extra);
// Fills objects[objectCount], assigns nextID, sets visible=1, increments objectCount and nextID
// Returns without adding if objectCount >= MAX_OBJECTS (print error)

void deleteObject(int id);
// Finds object with matching id AND visible==1
// Sets visible = 0
// Calls redrawCanvas()
// Prints error if ID not found or already deleted

void modifyObject(int id);
// Finds object by id (visible==1)
// Prompts user for new coordinate/dimension values (re-uses same field semantics as addObject)
// Updates the struct in-place
// Calls redrawCanvas()

void listObjects();
// Prints a formatted table of all objects where visible==1
// Columns: ID | Type | Coordinates/Dimensions
// Show "No active objects." if none exist

int findObjectByID(int id);
// Returns index into objects[] for matching id with visible==1
// Returns -1 if not found
```

## 5. Menu System
Implement as a `do { ... } while(choice != 10)` loop. Print the menu header before every prompt.

```
╔══════════════════════════════╗
║   2D GRAPHICS EDITOR v1.0    ║
╠══════════════════════════════╣
║  1. Display Canvas           ║
║  2. Draw Line                ║
║  3. Draw Rectangle           ║
║  4. Draw Triangle            ║
║  5. Draw Circle              ║
║  6. Delete Object            ║
║  7. Modify Object            ║
║  8. List All Objects         ║
║  9. Clear Canvas             ║
║ 10. Exit                     ║
╚══════════════════════════════╝
Enter choice:
```

### Per-Option Input Prompts
| Option | Prompts to display                                           |
|--------|--------------------------------------------------------------|
| 2      | `Enter x1 y1 x2 y2: `                                       |
| 3      | `Enter x y width height: `                                   |
| 4      | `Enter x1 y1 x2 y2 x3 y3: `                                 |
| 5      | `Enter cx cy radius: `                                       |
| 6      | `Enter Object ID to delete: `                               |
| 7      | `Enter Object ID to modify: ` then re-prompt for full coords|
| 9      | Confirm: `Clear canvas and delete all objects? (y/n): `      |

## 6. Input Validation Rules
- All coordinate inputs must be integers; reject non-integer input and re-prompt
- `width` and `height` for rectangle: must be > 0
- `radius` for circle: must be > 0
- `x`, `y`, `cx`, `cy` values: accept any integer (clipping handles out-of-bounds drawing)
- Invalid menu choice: print `"Invalid choice. Please enter 1-10."` and loop
- Object ID for delete/modify: print `"Error: Object ID [n] not found."` if missing
- `objectCount >= MAX_OBJECTS`: print `"Error: Maximum object limit (100) reached."`

## 7. Bonus Features (implement ALL of these)

### 7a. File I/O
```c
void saveCanvas(const char *filename);
// Writes canvas[][] to filename, row by row, newline-terminated
// Confirms: "Canvas saved to [filename]"

void loadCanvas(const char *filename);
// Reads lines from file back into canvas[][]
// Pads short lines with '_', truncates long lines
// Confirms: "Canvas loaded from [filename]"
```

### 7b. Undo / Redo
```c
#define UNDO_STACK_SIZE 20

typedef struct {
    GraphicObject snapshot[MAX_OBJECTS];
    int count;
} CanvasState;

CanvasState undoStack[UNDO_STACK_SIZE];
int undoTop = -1;
CanvasState redoStack[UNDO_STACK_SIZE];
int redoTop = -1;

void pushUndo();
// Saves a full deep copy of objects[] + objectCount to undoStack
// Clears redoStack
// Discards oldest state if stack is full (circular LIFO behavior)

void undoAction();
// Pops from undoStack, pushes current state to redoStack, restores objects[], calls redrawCanvas()

void redoAction();
// Pops from redoStack, pushes current state to undoStack, restores objects[], calls redrawCanvas()
```
**Rule**: call `pushUndo()` before every state-mutating operation (Draw, Delete, Modify, Clear).

### 7c. ncurses Mode (compile-time toggle)
Wrap all ncurses code in `#ifdef USE_NCURSES ... #endif`.  
When `USE_NCURSES` is defined:
- Use `MENU` and `ITEM` from `<menu.h>` for the main menu
- Use `WINDOW` with `box()` and `wrefresh()` to display the canvas in a bordered subwindow
- Keep all business logic (draw functions, object management) identical to the standard mode

---

# OUTPUT STRUCTURE
Produce output **in exactly this order**:

### Section 1 — Complete Source Code
A single file `graphics_editor.c` with:
- File header comment (author placeholder, date, description, compilation instructions)
- All `#include`, `#define`, `typedef enum`, `typedef struct` declarations at the top
- All function **prototypes** immediately after declarations
- `main()` containing only the menu loop — zero business logic inside `main()`
- All function **definitions** below `main()`, in the same order as their prototypes
- Inline comments on every non-trivial line; block comments before each function explaining purpose, parameters, and return value
- No magic numbers — every constant is a `#define` macro or `enum` value

### Section 2 — GCC Compilation Commands
```bash
# Standard build
gcc -std=c99 -Wall -Wextra -o graphics_editor graphics_editor.c

# ncurses build
gcc -std=c99 -Wall -Wextra -DUSE_NCURSES -o graphics_editor_ncurses graphics_editor.c -lncurses

# Debug build
gcc -std=c99 -Wall -Wextra -g -DDEBUG -o graphics_editor_debug graphics_editor.c
```

### Section 3 — Sample Program Execution
Show a complete terminal session transcript (text, not code block) demonstrating:
1. Drawing a line, a rectangle, a triangle, and a circle
2. Listing all objects
3. Deleting one object and redisplaying
4. Modifying another object
5. Saving to file
6. Undo then Redo
7. Exit

Include the actual canvas ASCII art output at each `Display Canvas` step.

### Section 4 — Complexity Analysis
For every function, provide a table:

| Function         | Time Complexity | Space Complexity | Notes                              |
|------------------|-----------------|------------------|------------------------------------|
| initializeCanvas | O(R×C)          | O(1)             | R=ROWS, C=COLS                     |
| drawLine         | O(max(Δx,Δy))   | O(1)             | Bresenham steps                    |
| drawCircle       | O(r)            | O(1)             | Midpoint octant iterations ≈ πr/4  |
| redrawCanvas     | O(N×S)          | O(1)             | N=objects, S=shape draw cost       |
| ...              | ...             | ...              | ...                                |

Also include a paragraph on overall memory footprint: canvas array size in bytes, object array size in bytes, total stack frame estimates.

---

# QUALITY GATES
Before finalising your response, mentally verify each of the following. Only submit when all pass:

- [ ] Code compiles with `gcc -std=c99 -Wall -Wextra` producing **zero warnings**
- [ ] `drawPixel()` is the **sole** function that writes to `canvas[][]`
- [ ] All 8 octants of Bresenham's line algorithm are handled correctly
- [ ] Midpoint circle algorithm correctly plots all 8 symmetric points
- [ ] `redrawCanvas()` is called after every mutation (draw/delete/modify/clear)
- [ ] No global mutable state other than `canvas[][]`, `objects[]`, `objectCount`, `nextID`, undo/redo stacks
- [ ] All user inputs go through a validation wrapper — no raw `scanf` with unchecked return values
- [ ] Boundary clipping in `drawPixel()` prevents any array out-of-bounds access
- [ ] Every `malloc`/`free` (if used) is matched — no memory leaks (prefer stack allocation)
- [ ] `listObjects()` handles the empty-list case gracefully
- [ ] File I/O handles `fopen()` returning `NULL` with a descriptive error message
- [ ] Undo stack correctly deep-copies the object array (not a pointer copy)
- [ ] The sample execution output is consistent with the actual code logic
- [ ] All `#define` constants, not magic numbers, appear in the source
- [ ] Code is suitable for undergraduate academic submission: readable, educational, not obfuscated

---

# CONSTRAINTS
- Language: **C only** — no C++, no external libraries beyond `<stdio.h>`, `<stdlib.h>`, `<string.h>`, `<math.h>`, and optionally `<ncurses.h>` / `<menu.h>`
- Standard: **C99** (use of `//` comments, `for(int i=...` declarations are permitted)
- Single file: all code in **one `.c` file** — no separate header files
- No dynamic memory allocation unless strictly necessary — prefer stack/static allocation
- Portable: must compile and run correctly on Linux (Ubuntu), macOS, and Windows (MinGW)
