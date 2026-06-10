/*
 * ============================================================
 *  2D GRAPHICS EDITOR — Console-Based Character Canvas
 * ============================================================
 *  Course   : Data Structures & Graphics Programming Lab
 *  Language : C (C99 or later)
 *  Compile  : gcc graphics_editor.c -o graphics_editor -lm
 *
 *  Features :
 *    - Character-based 50×20 canvas
 *    - Bresenham Line, Midpoint Circle, Rectangle, Triangle
 *    - Object store with up to 100 GraphicObject entries
 *    - Add / Delete / Modify / List / Redraw objects
 *    - Save / Load canvas to file (canvas.txt)
 *    - Full input validation
 * ============================================================
 */

/* ────────────────────────────────────────────────────────────
 * 1. HEADER FILES
 * ──────────────────────────────────────────────────────────── */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ────────────────────────────────────────────────────────────
 * 2. MACROS
 * ──────────────────────────────────────────────────────────── */
#define WIDTH        50          /* Canvas width  (columns) */
#define HEIGHT       20          /* Canvas height (rows)    */
#define EMPTY_CHAR   '_'         /* Background fill cell    */
#define DRAW_CHAR    '*'         /* Foreground pixel cell   */
#define MAX_OBJECTS  100         /* Maximum stored objects  */
#define CANVAS_FILE  "canvas.txt"

/* Utility macros */
#define ABS(x)       ((x) < 0 ? -(x) : (x))
#define SWAP(a, b)   { int _t = (a); (a) = (b); (b) = _t; }
#define IN_BOUNDS(x, y) ((x) >= 0 && (x) < WIDTH && (y) >= 0 && (y) < HEIGHT)

/* ────────────────────────────────────────────────────────────
 * 3. ENUM DEFINITIONS
 * ──────────────────────────────────────────────────────────── */
typedef enum
{
    LINE,
    RECTANGLE,
    TRIANGLE,
    CIRCLE
} ShapeType;

/* ────────────────────────────────────────────────────────────
 * 4. STRUCT DEFINITIONS
 * ──────────────────────────────────────────────────────────── */

/*
 * GraphicObject — stores every parameter needed to re-draw
 * any supported shape.  Unused fields are set to 0.
 *
 *  LINE      : uses x1,y1  x2,y2
 *  RECTANGLE : uses x1,y1 (top-left), x2 = width, y2 = height
 *  TRIANGLE  : uses x1,y1  x2,y2  x3,y3
 *  CIRCLE    : uses x1,y1 (centre), radius
 */
typedef struct
{
    int       id;       /* Unique auto-incremented identifier */
    ShapeType type;     /* Shape category (enum)              */
    int       x1, y1;  /* Primary point / top-left / centre  */
    int       x2, y2;  /* Second point / width+height        */
    int       x3, y3;  /* Third vertex (triangle only)       */
    int       radius;  /* Radius (circle only)               */
    int       visible; /* 1 = drawn on canvas, 0 = hidden    */
} GraphicObject;

/* ────────────────────────────────────────────────────────────
 * 5. GLOBAL VARIABLES
 * ──────────────────────────────────────────────────────────── */
char         canvas[HEIGHT][WIDTH];       /* The 2-D drawing surface  */
GraphicObject objects[MAX_OBJECTS];       /* Object database          */
int          objectCount = 0;             /* Current number of objects */
int          nextID      = 1;             /* Auto-increment ID counter */

/* ────────────────────────────────────────────────────────────
 * 6. FUNCTION PROTOTYPES
 * ──────────────────────────────────────────────────────────── */

/* Canvas utilities */
void initializeCanvas(void);
void clearCanvas(void);
void displayCanvas(void);

/* Primitive drawing */
void drawPixel(int x, int y);
void drawLine(int x1, int y1, int x2, int y2);
void drawRectangle(int x, int y, int width, int height);
void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3);
void drawCircle(int xc, int yc, int radius);

/* Object management */
void addObject(void);
void deleteObject(void);
void modifyObject(void);
void listObjects(void);
void redrawCanvas(void);

/* File handling */
void saveCanvasToFile(void);
void loadCanvasFromFile(void);

/* Menu */
void showMenu(void);
int  getMenuChoice(void);

/* ============================================================
 * 7. UTILITY FUNCTIONS
 * ============================================================ */

/*
 * initializeCanvas()
 * ------------------
 * Fills every cell of the global canvas with EMPTY_CHAR ('_').
 * Must be called once at program start before any drawing.
 */
void initializeCanvas(void)
{
    int r, c;
    for (r = 0; r < HEIGHT; r++)
        for (c = 0; c < WIDTH; c++)
            canvas[r][c] = EMPTY_CHAR;
}

/*
 * clearCanvas()
 * -------------
 * Resets the canvas back to all EMPTY_CHAR without touching
 * the object database.  Use before redrawing all objects.
 */
void clearCanvas(void)
{
    initializeCanvas();
}

/*
 * displayCanvas()
 * ---------------
 * Renders the canvas to stdout with a column ruler on top
 * and row numbers on the left, enclosed in a border frame.
 *
 * Example (partial):
 *    0         1         2         3         4
 *    0123456789012345678901234567890123456789012345678 9
 *   +--------------------------------------------------+
 * 0 |__________________________________________________|
 * 1 |________*_________________________________________|
 */
void displayCanvas(void)
{
    int r, c;

    /* ── top column ruler (tens digit) ── */
    printf("   ");
    for (c = 0; c < WIDTH; c++)
        printf("%d", (c / 10) % 10);
    printf("\n");

    /* ── top column ruler (units digit) ── */
    printf("   ");
    for (c = 0; c < WIDTH; c++)
        printf("%d", c % 10);
    printf("\n");

    /* ── top border ── */
    printf("  +");
    for (c = 0; c < WIDTH; c++) printf("-");
    printf("+\n");

    /* ── rows ── */
    for (r = 0; r < HEIGHT; r++)
    {
        printf("%2d|", r);               /* row number */
        for (c = 0; c < WIDTH; c++)
            printf("%c", canvas[r][c]);
        printf("|\n");
    }

    /* ── bottom border ── */
    printf("  +");
    for (c = 0; c < WIDTH; c++) printf("-");
    printf("+\n");
}

/* ============================================================
 * 8. DRAWING ALGORITHMS
 * ============================================================ */

/*
 * drawPixel(x, y)
 * ---------------
 * Plots a single DRAW_CHAR ('*') at column x, row y if the
 * coordinate is within canvas bounds.  All higher-level draw
 * routines call this function — it is the single boundary
 * check in the system.
 *
 * NOTE: x is the horizontal (column) axis,
 *       y is the vertical   (row)    axis.
 */
void drawPixel(int x, int y)
{
    if (IN_BOUNDS(x, y))
        canvas[y][x] = DRAW_CHAR;
}

/* ──────────────────────────────────────────────────────────
 * drawLine(x1, y1, x2, y2)
 * ─────────────────────────────────────────────────────────
 * Bresenham's Line Drawing Algorithm
 * ───────────────────────────────────
 * Classic integer-only rasterisation.  The idea is to track
 * a cumulative error term 'err' that tells us whether the
 * next pixel is closer to the true line above or below.
 *
 * Key steps:
 *  1. Compute deltas dx = |x2-x1|, dy = |y2-y1|.
 *  2. Determine step directions sx, sy (+1 or -1).
 *  3. Initialise error = dx - dy.
 *  4. Loop: plot current pixel; compute 2*err to decide the
 *     next move; advance x or y (or both) accordingly.
 *
 * Time Complexity : O(max(|dx|, |dy|)) ─ proportional to the
 *                   number of pixels on the line segment.
 * Space Complexity: O(1)
 */
void drawLine(int x1, int y1, int x2, int y2)
{
    int dx = ABS(x2 - x1);       /* Total horizontal span */
    int dy = ABS(y2 - y1);       /* Total vertical span   */

    /* Step direction: +1 if destination is to the right/down */
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int err = dx - dy;            /* Initial error term */
    int e2;                       /* Doubled error used in decision */

    while (1)
    {
        drawPixel(x1, y1);        /* Plot current position */

        if (x1 == x2 && y1 == y2)
            break;                /* Reached the endpoint */

        e2 = 2 * err;

        /*
         * If e2 > -dy  →  horizontal distance dominates,
         * reduce error by 2*dy and step in x-direction.
         */
        if (e2 > -dy)
        {
            err -= dy;
            x1  += sx;
        }

        /*
         * If e2 < dx  →  vertical distance dominates (or equal),
         * increase error by 2*dx and step in y-direction.
         */
        if (e2 < dx)
        {
            err += dx;
            y1  += sy;
        }
    }
}

/* ──────────────────────────────────────────────────────────
 * drawRectangle(x, y, width, height)
 * ────────────────────────────────────────────────────────
 * Draws a hollow rectangle by calling drawLine() for each of
 * the four edges:
 *   top    : (x, y)          → (x+w-1, y)
 *   bottom : (x, y+h-1)      → (x+w-1, y+h-1)
 *   left   : (x, y)          → (x,     y+h-1)
 *   right  : (x+w-1, y)      → (x+w-1, y+h-1)
 *
 * Time Complexity : O(width + height)
 * Space Complexity: O(1)
 */
void drawRectangle(int x, int y, int width, int height)
{
    /* Top edge */
    drawLine(x,           y,          x + width - 1, y         );
    /* Bottom edge */
    drawLine(x,           y + height - 1, x + width - 1, y + height - 1);
    /* Left edge */
    drawLine(x,           y,          x,             y + height - 1);
    /* Right edge */
    drawLine(x + width-1, y,          x + width - 1, y + height - 1);
}

/* ──────────────────────────────────────────────────────────
 * drawTriangle(x1,y1, x2,y2, x3,y3)
 * ────────────────────────────────────────────────────────
 * Draws a triangle outline by connecting the three vertices
 * with three Bresenham line segments.
 *
 * Time Complexity : O(max side length × 3)
 * Space Complexity: O(1)
 */
void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3)
{
    drawLine(x1, y1, x2, y2);   /* Side A–B */
    drawLine(x2, y2, x3, y3);   /* Side B–C */
    drawLine(x3, y3, x1, y1);   /* Side C–A */
}

/* ──────────────────────────────────────────────────────────
 * drawCircle(xc, yc, radius)
 * ────────────────────────────────────────────────────────
 * Midpoint Circle Algorithm (Bresenham's Circle)
 * ───────────────────────────────────────────────
 * Exploits 8-fold symmetry: only 1/8 of the circle is
 * computed; the other seven octants are obtained by sign
 * and axis swaps around the centre (xc, yc).
 *
 * Decision parameter  p = 1 - r  (initial).
 *  • If p < 0 : move East   → p += 2x + 3
 *  • If p ≥ 0 : move SouthEast → p += 2(x - y) + 5
 *
 * The eight symmetric points for (x, y) offset from centre:
 *   (xc+x, yc+y)  (xc-x, yc+y)  (xc+x, yc-y)  (xc-x, yc-y)
 *   (xc+y, yc+x)  (xc-y, yc+x)  (xc+y, yc-x)  (xc-y, yc-x)
 *
 * Time Complexity : O(r)  — one octant iteration ≈ r/√2 steps
 * Space Complexity: O(1)
 */
void drawCircle(int xc, int yc, int radius)
{
    int x = 0;
    int y = radius;
    int p = 1 - radius;          /* Initial decision parameter */

    /*
     * plotCirclePoints — plots all 8 symmetric pixels for the
     * current (x, y) offset pair.
     */
    #define PLOT8(cx,cy,px,py) \
        drawPixel((cx)+(px), (cy)+(py)); \
        drawPixel((cx)-(px), (cy)+(py)); \
        drawPixel((cx)+(px), (cy)-(py)); \
        drawPixel((cx)-(px), (cy)-(py)); \
        drawPixel((cx)+(py), (cy)+(px)); \
        drawPixel((cx)-(py), (cy)+(px)); \
        drawPixel((cx)+(py), (cy)-(px)); \
        drawPixel((cx)-(py), (cy)-(px))

    PLOT8(xc, yc, x, y);

    while (x < y)
    {
        x++;
        if (p < 0)
        {
            /* East decision: stay at same y */
            p += 2 * x + 1;
        }
        else
        {
            /* SouthEast decision: move y inward */
            y--;
            p += 2 * (x - y) + 1;
        }
        PLOT8(xc, yc, x, y);
    }

    #undef PLOT8
}

/* ============================================================
 * 9. OBJECT MANAGEMENT FUNCTIONS
 * ============================================================ */

/*
 * redrawCanvas()
 * --------------
 * Wipes the canvas blank, then re-draws every visible object
 * from the object database in insertion order.
 *
 * This function is the single source of truth for what the
 * canvas shows.  It must be called after any operation that
 * adds, removes, or modifies an object so the display stays
 * consistent with the stored data.
 *
 * Time Complexity : O(objectCount × cost_of_shape)
 *                   Worst case O(N × W) where N = MAX_OBJECTS
 *                   and W = max(WIDTH, HEIGHT).
 * Space Complexity: O(1) extra (reuses global canvas array)
 */
void redrawCanvas(void)
{
    int i;

    clearCanvas();   /* Step 1: blank slate */

    /* Step 2: re-draw each visible object */
    for (i = 0; i < objectCount; i++)
    {
        if (!objects[i].visible)
            continue;

        switch (objects[i].type)
        {
            case LINE:
                drawLine(objects[i].x1, objects[i].y1,
                         objects[i].x2, objects[i].y2);
                break;

            case RECTANGLE:
                /* x2 stores width, y2 stores height */
                drawRectangle(objects[i].x1, objects[i].y1,
                              objects[i].x2, objects[i].y2);
                break;

            case TRIANGLE:
                drawTriangle(objects[i].x1, objects[i].y1,
                             objects[i].x2, objects[i].y2,
                             objects[i].x3, objects[i].y3);
                break;

            case CIRCLE:
                drawCircle(objects[i].x1, objects[i].y1,
                           objects[i].radius);
                break;
        }
    }
}

/* ──────────────────────────────────────────────────────────
 * addObject()
 * ──────────────────────────────────────────────────────────
 * Called by each "Draw X" menu option AFTER the shape-specific
 * parameters have been gathered and validated.  It appends the
 * new GraphicObject to the objects[] array, increments
 * objectCount, and draws the shape on the live canvas (no
 * full redraw needed since we're only adding, not modifying).
 *
 * Helper — not called directly from the menu; each draw
 * function fills a local GraphicObject then passes it here.
 */
static void storeObject(GraphicObject obj)
{
    if (objectCount >= MAX_OBJECTS)
    {
        printf("  [ERROR] Object store is full (limit %d).\n", MAX_OBJECTS);
        return;
    }
    obj.id      = nextID++;
    obj.visible = 1;
    objects[objectCount++] = obj;
    printf("  [OK] Object ID %d created.\n", obj.id);
}

/*
 * addObject() — top-level dispatcher.
 * Prompts the user to choose a shape type, collects parameters,
 * validates them, draws the shape, and calls storeObject().
 */
void addObject(void)
{
    int choice;
    GraphicObject obj;
    memset(&obj, 0, sizeof(obj));

    printf("\n  Shape type:\n");
    printf("    1. Line\n");
    printf("    2. Rectangle\n");
    printf("    3. Triangle\n");
    printf("    4. Circle\n");
    printf("  Choice: ");
    if (scanf("%d", &choice) != 1) { while(getchar()!='\n'); return; }

    switch (choice)
    {
        /* ── LINE ── */
        case 1:
            obj.type = LINE;
            printf("  Enter x1 y1 : "); scanf("%d %d", &obj.x1, &obj.y1);
            printf("  Enter x2 y2 : "); scanf("%d %d", &obj.x2, &obj.y2);

            if (!IN_BOUNDS(obj.x1, obj.y1) || !IN_BOUNDS(obj.x2, obj.y2))
            { printf("  [ERROR] Coordinates out of canvas bounds.\n"); return; }

            drawLine(obj.x1, obj.y1, obj.x2, obj.y2);
            storeObject(obj);
            break;

        /* ── RECTANGLE ── */
        case 2:
            obj.type = RECTANGLE;
            printf("  Enter top-left x y : "); scanf("%d %d", &obj.x1, &obj.y1);
            printf("  Enter width        : "); scanf("%d", &obj.x2);
            printf("  Enter height       : "); scanf("%d", &obj.y2);

            if (!IN_BOUNDS(obj.x1, obj.y1))
            { printf("  [ERROR] Top-left coordinate out of bounds.\n"); return; }
            if (obj.x2 <= 0 || obj.y2 <= 0)
            { printf("  [ERROR] Width and height must be > 0.\n"); return; }
            if (!IN_BOUNDS(obj.x1 + obj.x2 - 1, obj.y1 + obj.y2 - 1))
            { printf("  [ERROR] Rectangle extends outside canvas.\n"); return; }

            drawRectangle(obj.x1, obj.y1, obj.x2, obj.y2);
            storeObject(obj);
            break;

        /* ── TRIANGLE ── */
        case 3:
            obj.type = TRIANGLE;
            printf("  Enter x1 y1 : "); scanf("%d %d", &obj.x1, &obj.y1);
            printf("  Enter x2 y2 : "); scanf("%d %d", &obj.x2, &obj.y2);
            printf("  Enter x3 y3 : "); scanf("%d %d", &obj.x3, &obj.y3);

            if (!IN_BOUNDS(obj.x1,obj.y1) || !IN_BOUNDS(obj.x2,obj.y2)
                || !IN_BOUNDS(obj.x3,obj.y3))
            { printf("  [ERROR] One or more vertices out of canvas bounds.\n"); return; }

            drawTriangle(obj.x1,obj.y1, obj.x2,obj.y2, obj.x3,obj.y3);
            storeObject(obj);
            break;

        /* ── CIRCLE ── */
        case 4:
            obj.type = CIRCLE;
            printf("  Enter center x y : "); scanf("%d %d", &obj.x1, &obj.y1);
            printf("  Enter radius     : "); scanf("%d", &obj.radius);

            if (!IN_BOUNDS(obj.x1, obj.y1))
            { printf("  [ERROR] Centre out of canvas bounds.\n"); return; }
            if (obj.radius <= 0)
            { printf("  [ERROR] Radius must be > 0.\n"); return; }

            drawCircle(obj.x1, obj.y1, obj.radius);
            storeObject(obj);
            break;

        default:
            printf("  [ERROR] Invalid shape choice.\n");
    }
}

/*
 * deleteObject()
 * --------------
 * 1. Asks for the ID of the object to remove.
 * 2. Searches the objects[] array for a matching ID.
 * 3. Removes it by shifting all subsequent entries left
 *    (keeps the array compact; no gaps).
 * 4. Decrements objectCount.
 * 5. Calls redrawCanvas() to reflect the deletion.
 *
 * Time Complexity : O(N) search + O(N) shift  →  O(N)
 * Space Complexity: O(1)
 */
void deleteObject(void)
{
    int targetID, i, found = -1;

    printf("  Enter Object ID to delete: ");
    if (scanf("%d", &targetID) != 1) { while(getchar()!='\n'); return; }

    /* Linear search for the ID */
    for (i = 0; i < objectCount; i++)
    {
        if (objects[i].id == targetID)
        {
            found = i;
            break;
        }
    }

    if (found == -1)
    {
        printf("  [ERROR] Invalid Object ID.\n");
        return;
    }

    /* Shift remaining objects left to fill the gap */
    for (i = found; i < objectCount - 1; i++)
        objects[i] = objects[i + 1];

    objectCount--;
    memset(&objects[objectCount], 0, sizeof(GraphicObject));

    redrawCanvas();
    printf("  [OK] Object deleted successfully.\n");
}

/*
 * modifyObject()
 * --------------
 * 1. Locates object by ID.
 * 2. Detects its ShapeType.
 * 3. Prompts for new parameters (type-specific).
 * 4. Validates the new values.
 * 5. Updates the stored object in-place.
 * 6. Calls redrawCanvas().
 *
 * Time Complexity : O(N) search + redraw cost
 * Space Complexity: O(1)
 */
void modifyObject(void)
{
    int targetID, i, found = -1;

    printf("  Enter Object ID to modify: ");
    if (scanf("%d", &targetID) != 1) { while(getchar()!='\n'); return; }

    for (i = 0; i < objectCount; i++)
    {
        if (objects[i].id == targetID)
        {
            found = i;
            break;
        }
    }

    if (found == -1)
    {
        printf("  [ERROR] Invalid Object ID.\n");
        return;
    }

    GraphicObject *obj = &objects[found];

    switch (obj->type)
    {
        case LINE:
            printf("  New x1 y1 : "); scanf("%d %d", &obj->x1, &obj->y1);
            printf("  New x2 y2 : "); scanf("%d %d", &obj->x2, &obj->y2);
            if (!IN_BOUNDS(obj->x1,obj->y1) || !IN_BOUNDS(obj->x2,obj->y2))
            { printf("  [ERROR] Coordinates out of bounds. Modification cancelled.\n"); return; }
            break;

        case RECTANGLE:
            printf("  New top-left x y : "); scanf("%d %d", &obj->x1, &obj->y1);
            printf("  New width        : "); scanf("%d", &obj->x2);
            printf("  New height       : "); scanf("%d", &obj->y2);
            if (!IN_BOUNDS(obj->x1,obj->y1) || obj->x2<=0 || obj->y2<=0 ||
                !IN_BOUNDS(obj->x1+obj->x2-1, obj->y1+obj->y2-1))
            { printf("  [ERROR] Invalid dimensions. Modification cancelled.\n"); return; }
            break;

        case TRIANGLE:
            printf("  New x1 y1 : "); scanf("%d %d", &obj->x1, &obj->y1);
            printf("  New x2 y2 : "); scanf("%d %d", &obj->x2, &obj->y2);
            printf("  New x3 y3 : "); scanf("%d %d", &obj->x3, &obj->y3);
            if (!IN_BOUNDS(obj->x1,obj->y1) || !IN_BOUNDS(obj->x2,obj->y2)
                || !IN_BOUNDS(obj->x3,obj->y3))
            { printf("  [ERROR] Vertex out of bounds. Modification cancelled.\n"); return; }
            break;

        case CIRCLE:
            printf("  New center x y : "); scanf("%d %d", &obj->x1, &obj->y1);
            printf("  New radius     : "); scanf("%d", &obj->radius);
            if (!IN_BOUNDS(obj->x1,obj->y1) || obj->radius <= 0)
            { printf("  [ERROR] Invalid circle parameters. Modification cancelled.\n"); return; }
            break;
    }

    redrawCanvas();
    printf("  [OK] Object ID %d modified successfully.\n", targetID);
}

/*
 * listObjects()
 * -------------
 * Iterates the objects[] array and prints a formatted table
 * showing each object's ID, type, and shape-specific parameters.
 *
 * Time Complexity : O(N)
 * Space Complexity: O(1)
 */
void listObjects(void)
{
    if (objectCount == 0)
    {
        printf("  No objects stored.\n");
        return;
    }

    printf("\n  %-5s %-12s %s\n",  "ID", "TYPE", "DETAILS");
    printf("  %s\n", "----------------------------------------------------");

    int i;
    for (i = 0; i < objectCount; i++)
    {
        GraphicObject *o = &objects[i];
        switch (o->type)
        {
            case LINE:
                printf("  %-5d %-12s (%d,%d) -> (%d,%d)\n",
                       o->id, "LINE", o->x1, o->y1, o->x2, o->y2);
                break;

            case RECTANGLE:
                printf("  %-5d %-12s (%d,%d) w=%d h=%d\n",
                       o->id, "RECTANGLE", o->x1, o->y1, o->x2, o->y2);
                break;

            case TRIANGLE:
                printf("  %-5d %-12s (%d,%d) (%d,%d) (%d,%d)\n",
                       o->id, "TRIANGLE",
                       o->x1, o->y1, o->x2, o->y2, o->x3, o->y3);
                break;

            case CIRCLE:
                printf("  %-5d %-12s center=(%d,%d) r=%d\n",
                       o->id, "CIRCLE", o->x1, o->y1, o->radius);
                break;
        }
    }
    printf("\n  Total objects: %d\n", objectCount);
}

/* ============================================================
 * 10. FILE HANDLING FUNCTIONS
 * ============================================================ */

/*
 * saveCanvasToFile()
 * ------------------
 * Writes the raw canvas character array (HEIGHT rows, each
 * WIDTH characters wide) to CANVAS_FILE ("canvas.txt").
 *
 * Format: one canvas row per file line, no extra characters.
 *
 * Also writes the object database after the canvas so that
 * loadCanvasFromFile() can restore objects and their IDs.
 *
 * File layout:
 *   Line 1..HEIGHT  : canvas rows (WIDTH chars each)
 *   Line HEIGHT+1   : "OBJECTS <count> <nextID>"
 *   Line HEIGHT+2.. : one object record per line (CSV-ish)
 */
void saveCanvasToFile(void)
{
    FILE *fp = fopen(CANVAS_FILE, "w");
    if (!fp)
    {
        printf("  [ERROR] Cannot open '%s' for writing.\n", CANVAS_FILE);
        return;
    }

    int r;
    /* Write canvas rows */
    for (r = 0; r < HEIGHT; r++)
    {
        fwrite(canvas[r], 1, WIDTH, fp);
        fputc('\n', fp);
    }

    /* Write object database header */
    fprintf(fp, "OBJECTS %d %d\n", objectCount, nextID);

    /* Write each object as a fixed-field record */
    int i;
    for (i = 0; i < objectCount; i++)
    {
        GraphicObject *o = &objects[i];
        fprintf(fp, "%d %d %d %d %d %d %d %d %d %d\n",
                o->id, (int)o->type,
                o->x1, o->y1,
                o->x2, o->y2,
                o->x3, o->y3,
                o->radius, o->visible);
    }

    fclose(fp);
    printf("  [OK] Canvas saved to '%s'.\n", CANVAS_FILE);
}

/*
 * loadCanvasFromFile()
 * --------------------
 * Reads canvas rows and the object database from CANVAS_FILE.
 * Replaces both the global canvas[] array and objects[] array.
 * Calls redrawCanvas() at the end to synchronise the visual
 * canvas with the restored objects.
 */
void loadCanvasFromFile(void)
{
    FILE *fp = fopen(CANVAS_FILE, "r");
    if (!fp)
    {
        printf("  [ERROR] Cannot open '%s' for reading.\n", CANVAS_FILE);
        return;
    }

    int r;
    char lineBuf[WIDTH + 4];

    /* Read canvas rows */
    for (r = 0; r < HEIGHT; r++)
    {
        if (!fgets(lineBuf, sizeof(lineBuf), fp))
        {
            printf("  [ERROR] Unexpected end of file at row %d.\n", r);
            fclose(fp);
            return;
        }
        /* Copy exactly WIDTH characters; ignore newline */
        memcpy(canvas[r], lineBuf, WIDTH);
    }

    /* Read object database header */
    int savedCount, savedNextID;
    if (fscanf(fp, "OBJECTS %d %d\n", &savedCount, &savedNextID) != 2)
    {
        printf("  [WARN] No object database found in file.\n");
        fclose(fp);
        return;
    }

    if (savedCount > MAX_OBJECTS)
    {
        printf("  [ERROR] Saved object count (%d) exceeds MAX_OBJECTS.\n",
               savedCount);
        fclose(fp);
        return;
    }

    objectCount = 0;
    nextID      = savedNextID;

    int i;
    for (i = 0; i < savedCount; i++)
    {
        GraphicObject o;
        int typeInt;
        if (fscanf(fp, "%d %d %d %d %d %d %d %d %d %d\n",
                   &o.id, &typeInt,
                   &o.x1, &o.y1,
                   &o.x2, &o.y2,
                   &o.x3, &o.y3,
                   &o.radius, &o.visible) == 10)
        {
            o.type = (ShapeType)typeInt;
            objects[objectCount++] = o;
        }
    }

    fclose(fp);
    printf("  [OK] Canvas loaded from '%s'.\n", CANVAS_FILE);

    /*
     * Rebuild the canvas from the restored object list so that
     * the pixel data reflects the current object database.
     */
    redrawCanvas();
}

/* ============================================================
 * 11. MENU FUNCTIONS
 * ============================================================ */

void showMenu(void)
{
    printf("\n");
    printf("  ==============================\n");
    printf("       2D GRAPHICS EDITOR\n");
    printf("  ==============================\n");
    printf("   1.  Display Canvas\n");
    printf("   2.  Draw Line\n");
    printf("   3.  Draw Rectangle\n");
    printf("   4.  Draw Triangle\n");
    printf("   5.  Draw Circle\n");
    printf("   6.  Delete Object\n");
    printf("   7.  Modify Object\n");
    printf("   8.  List All Objects\n");
    printf("   9.  Clear Canvas\n");
    printf("  10.  Save Canvas\n");
    printf("  11.  Load Canvas\n");
    printf("  12.  Exit\n");
    printf("  ==============================\n");
    printf("  Choice: ");
}

int getMenuChoice(void)
{
    int choice;
    if (scanf("%d", &choice) != 1)
    {
        /* Flush non-numeric junk from the input buffer */
        while (getchar() != '\n');
        return -1;
    }
    return choice;
}

/* ============================================================
 * 12. MAIN FUNCTION
 * ============================================================ */
int main(void)
{
    int running = 1;
    int choice;

    /* Initialise the canvas on program start */
    initializeCanvas();

    printf("\n  Welcome to the 2D Graphics Editor!\n");
    printf("  Canvas: %d columns × %d rows\n", WIDTH, HEIGHT);

    while (running)
    {
        showMenu();
        choice = getMenuChoice();

        switch (choice)
        {
            /* ── 1. Display Canvas ── */
            case 1:
                printf("\n");
                displayCanvas();
                break;

            /* ── 2. Draw Line ── */
            case 2:
            {
                GraphicObject obj;
                memset(&obj, 0, sizeof(obj));
                obj.type = LINE;

                printf("  Enter x1 y1 : ");
                scanf("%d %d", &obj.x1, &obj.y1);
                printf("  Enter x2 y2 : ");
                scanf("%d %d", &obj.x2, &obj.y2);

                if (!IN_BOUNDS(obj.x1, obj.y1) || !IN_BOUNDS(obj.x2, obj.y2))
                {
                    printf("  [ERROR] Coordinates out of canvas bounds "
                           "(x: 0-%d, y: 0-%d).\n", WIDTH-1, HEIGHT-1);
                    break;
                }

                drawLine(obj.x1, obj.y1, obj.x2, obj.y2);
                storeObject(obj);
                break;
            }

            /* ── 3. Draw Rectangle ── */
            case 3:
            {
                GraphicObject obj;
                memset(&obj, 0, sizeof(obj));
                obj.type = RECTANGLE;

                printf("  Enter top-left x y : ");
                scanf("%d %d", &obj.x1, &obj.y1);
                printf("  Enter width        : ");
                scanf("%d", &obj.x2);
                printf("  Enter height       : ");
                scanf("%d", &obj.y2);

                if (!IN_BOUNDS(obj.x1, obj.y1))
                { printf("  [ERROR] Top-left out of bounds.\n"); break; }
                if (obj.x2 <= 0 || obj.y2 <= 0)
                { printf("  [ERROR] Width and height must be > 0.\n"); break; }
                if (!IN_BOUNDS(obj.x1 + obj.x2 - 1, obj.y1 + obj.y2 - 1))
                { printf("  [ERROR] Rectangle extends outside canvas.\n"); break; }

                drawRectangle(obj.x1, obj.y1, obj.x2, obj.y2);
                storeObject(obj);
                break;
            }

            /* ── 4. Draw Triangle ── */
            case 4:
            {
                GraphicObject obj;
                memset(&obj, 0, sizeof(obj));
                obj.type = TRIANGLE;

                printf("  Enter x1 y1 : ");
                scanf("%d %d", &obj.x1, &obj.y1);
                printf("  Enter x2 y2 : ");
                scanf("%d %d", &obj.x2, &obj.y2);
                printf("  Enter x3 y3 : ");
                scanf("%d %d", &obj.x3, &obj.y3);

                if (!IN_BOUNDS(obj.x1,obj.y1) || !IN_BOUNDS(obj.x2,obj.y2)
                    || !IN_BOUNDS(obj.x3,obj.y3))
                {
                    printf("  [ERROR] One or more vertices out of canvas bounds.\n");
                    break;
                }

                drawTriangle(obj.x1,obj.y1, obj.x2,obj.y2, obj.x3,obj.y3);
                storeObject(obj);
                break;
            }

            /* ── 5. Draw Circle ── */
            case 5:
            {
                GraphicObject obj;
                memset(&obj, 0, sizeof(obj));
                obj.type = CIRCLE;

                printf("  Enter center x y : ");
                scanf("%d %d", &obj.x1, &obj.y1);
                printf("  Enter radius     : ");
                scanf("%d", &obj.radius);

                if (!IN_BOUNDS(obj.x1, obj.y1))
                { printf("  [ERROR] Centre out of canvas bounds.\n"); break; }
                if (obj.radius <= 0)
                { printf("  [ERROR] Radius must be > 0.\n"); break; }

                drawCircle(obj.x1, obj.y1, obj.radius);
                storeObject(obj);
                break;
            }

            /* ── 6. Delete Object ── */
            case 6:
                deleteObject();
                break;

            /* ── 7. Modify Object ── */
            case 7:
                modifyObject();
                break;

            /* ── 8. List All Objects ── */
            case 8:
                listObjects();
                break;

            /* ── 9. Clear Canvas ── */
            case 9:
                clearCanvas();
                objectCount = 0;
                nextID      = 1;
                printf("  [OK] Canvas cleared. All objects removed.\n");
                break;

            /* ── 10. Save Canvas ── */
            case 10:
                saveCanvasToFile();
                break;

            /* ── 11. Load Canvas ── */
            case 11:
                loadCanvasFromFile();
                break;

            /* ── 12. Exit ── */
            case 12:
                running = 0;
                printf("  Goodbye!\n\n");
                break;

            default:
                printf("  [ERROR] Invalid choice. Please enter 1-12.\n");
                break;
        }
    }

    return 0;
}

/* ============================================================
 * END OF FILE
 * ============================================================ */
