[EN]
# Making a simple Tetris game in the terminal
When trying to learn how to code complex projects, it can be hard going from the simple `farenheit.c` example to a fully GPU rendered game. As so, and as I have done myself, I think the best intermediate project to attempt is a terminal-rendered game.

In this blog, I will build a simple terminal-rendered ***Tetris*** using ASCII characters and terminal escape sequences (for colors and such). This project will be built from the ground up (no dependencies on libraries not preinstalled on basically every machine) to be as accessible as possible. You can find the finished game in this [GitHub repository](https://github.com/SupSyDeMarciou/tetris).

The main objective is to have fun: this is not a tutorial on efficient TUI application design, general software development or game design. My goal is to make this project as well documented as possible to help new C programmers have a better grasp of how to actually achieve such a project.

## Summary
1. A common base for terminal-rendered applications  
    1.1 How to draw using a terminal  
    1.2 How to query user inputs  
2. Architecture of the project  
3. Practical example: The game of Tetris  
    3.1 Creating a simple menu  
    3.2 How to manage the pieces  
    3.3 The core gameplay loop  


![Finished Tetris game in the terminal](terminal_view.png)

# 1. A common base for terminal-rendered applications

## 1.1 How to draw using a terminal

The main idea behind drawing in the terminal is the same as when drawing *ASCII-Art*: we use the characters as lines and/or values in a grid (unlike a grid of *pixels*) to build up larger shapes.
```
                ^     ^
               / \___/ \
            __/  _   _  \__
            ===    ^    ===  ,--,
               '--   --'    /    \
               /   V   \    |   _/
              |  | | |  |   /
             (,_(,,|,,)_,)-"

Cat in ASCII-ART (only for illustrative purposes)
```

The large majority of terminals display text in a *monospace* font (meaning that each character is the same width, just like in the font I use for this website). Therefore, we can think of the terminal window as a *grid* of characters, each occupying a rectangle of roughly 2:1 proportions. Thanks to this consistency, we can *draw* shapes anywhere in the terminal window without worrying about distorsions due to varying character widths.

We can index the different characters based on which column and row they occupy, just like a pixel on a screen. If we imagine printing an image row by row in the terminal, then a *row* would be a `'\n'` terminated line (if we assume the terminal window is wide enough) and a *column* would be the character's position within this line :
```C
printf(
    ".----------.\n" //
    "| JUST     |\n" //
    "|     A    |\n" // In C, string litterals are automaticaly concatenated if nothing separates
    "|      BOX |\n" // them. Therefore, these five lines form a single string litteral
    "'----------'\n" //
);
// Expected:
//              1
//    0 2 4 6 8 0
// 0  .----------.
// 1  | JUST     |
// 2  |     A    |
// 3  |      BOX |
// 4  '----------'
//
// Character 'A' is a position (6;2), so on position 6 of line 2
```

We have a random box, nice! However, if we want to draw anything else now it will have to be bellow this box as we can only print entire lines from top to bottom. We will have to be thoughtfull of what we need to print and where we need it so that we may sequence the print commands accordingly and ensure that everything appears in its right place. Alternatively, we could store complete references to the lines to display within an editable buffer so that the order in which edits are made doesn't affect the result. In the game we are making, we will be using a bit of both. Here is an example displaying two boxes to show how we can work by line:
```C
// This setup achieves the same result as the preview example. However, separating the lines like
// this allows us to index in which line we are (which may be useful when indexing an array for
// example).
for (int l = 0; l < 5; l++)
{
    switch (l)
    {
        case 0: printf(".----------."); break;
        case 1: printf("| THE      |"); break;
        case 2: printf("|   FIRST  |"); break;
        case 3: printf("|      BOX |"); break;
        case 4: printf("'----------'"); break;
    }

    printf("    "); // Four spaces

    switch (l)
    {
        case 0: printf(".----------."); break;
        case 1: printf("| THE      |"); break;
        case 2: printf("|  SECOND  |"); break;
        case 3: printf("|      BOX |"); break;
        case 4: printf("'----------'"); break;
    }

    printf("\n");
}

// Expected:
//              1 1 1 1 1 2 2 2 2
//    0 2 4 6 8 0 2 4 6 8 0 2 4 6
// 0  .----------.    .----------.
// 1  | THE      |    | THE      |
// 2  |   FIRST  |    |  SECOND  |
// 3  |      BOX |    |      BOX |
// 4  '----------'    '----------'
```

Thinking about printing in this way grants us much better flexibility in how to arrange our rendering logic.

Now that we can draw what we want, we need a way to clear the terminal or move the *cursor* (the position within the terminal window from which we print) back to the top of the terminal window again in order to draw the next *frame* of our application. To achieve this and much more, we will use ***[ANSI Escape Sequences](https://en.wikipedia.org/wiki/ANSI_escape_code)*** which are a set of commands one can use to manipulate the terminal's appearence and behaviour.

The commands we will use take the form `"\033[..."` and are to be placed within the string outputed to the terminal. Indeed, `\033` is an *octal number* (written in base 8) which equals to `27` in decimal. This number is called the *escape code* and is pretty much standard throughout terminals as the introduction to a terminal command (it is also defined as the *escaped character* `'\e'` in C). In our case, all commands will also have an opening square bracket `[` as the second byte because we only manipulate the terminal appearence (these are called *Control Sequence Introducer commands*).

Specificaly, we will need:
- `"\033[`*`n`*`J"` to erase the screen, with *`n`* either 0 (erase from cursor position to *end* of screen), 1 (erase from cursor position to *top* or screen) or 2 (erase *entire* screen).
- `"\033[`*`n`*`;`*`m`*`H"` to move the cursor to row `n` and column `m`.
- `"\033[`*`n`*`m"` to change the color (and other things) for the next print operations, with `n` an integer (the *[wikipedia page](https://en.wikipedia.org/wiki/ANSI_escape_code)* references all the possible values).

Using this, we can now make our boxes bounce up and down in two different colors :
```C
#include <stdio.h>
#include <unistd.h>
#include <math.h>

int main()
{
    // This is used for the box boucing, so that we have a time reference
    for (int frame_count = 0; ; ++frame_count)
    {
        const int offset1 = 2.5 + 2.5 * sin(frame_count * 0.1);
        const int offset2 = 2.5 - 2.5 * sin(frame_count * 0.1);

        printf(
            "\033[2J" // Clear entire screen
            "\033[H" // Reset cursor position (top-left is the default)
        );
        for (int l = 0; l < 10; l++) // We will work with 10 lines for the screen
        {
            
            printf("\033[31m"); // Set color to "red"
            switch (l - offset1) // Check current line
            {
                case 0:  printf(".----------."); break;
                case 1:  printf("| THE      |"); break;
                case 2:  printf("|   FIRST  |"); break;
                case 3:  printf("|      BOX |"); break;
                case 4:  printf("'----------'"); break;
                default: printf("            "); break; // If we're not drawing a box, then we draw
                          // blank characters to keep the spacing intact for the next box
            }
            printf("\033[0m"); // Reset color (not necessary, but good practice when you don't know
                               // what comes next).
            
            printf("    "); // Four spaces
            
            printf("\033[32m"); // Set color to "green"
            switch (l - offset2) // Same as the first box
            {
                case 0:  printf(".----------."); break;
                case 1:  printf("| THE      |"); break;
                case 2:  printf("|  SECOND  |"); break;
                case 3:  printf("|      BOX |"); break;
                case 4:  printf("'----------'"); break;
                default: printf("            "); break;
            }
            printf("\033[0m");
    
            printf("\n"); // Finish the row by passing a newline character
        }

        usleep(1000*1000/30); // Just to slow down the execution (we set a target of 30 frames per second, or 30 FPS)
    }
}
```
This programs outputs the following (just imagine the boxes moving up and down, I don't know how to embed video yet):

![Output of the previous example on the terminal](example_boxes.png)

> **(i)** Technically, we could also use *ANSI escape sequences* to move the cursor to a specific coordinate using `"\033[n;mH"` as a replacement to printing line by line, but I prefer to limit my uses of escape sequences as, depending on the terminal *(looking at you, Windows terminals)*, printing may be slow and reducing the number of times we have to jump around and redraw can become crucial. This constraint also applies to changing color, and in a sense extends to graphics rendering in general as the *overdraw problem*.

## 1.2 How to query user inputs

Now let's say we wanted to change the color of the second box based on user input, how wan we proceed?  
This can be achieved in multiple ways, but the simplest is to listen to the *standard input stream (`stdin`)* and wait for the user to press a key:
```C
int c = getchar();
printf("Got %c (%d)\n", c, c);
```
Sadly, it is not that simple as the *standard input* (where the user types) is *buffered* by default, which means that a newline character has to be entered in order for `getc` to return (so everytime the user writes something, they would have to also press *enter* for their input to be taken into account, meanwhile the program is put on pause (the *thread* is put to sleep)).

What we want is a function which exactly retrieves a character from the standard input as soon as it is written. Such a function exists on Windows in the library `<conio.h>`, but on Linux we have to write it ourselves... So I didn't write it myself and found *[this on StackOverflow](https://stackoverflow.com/questions/3276546/how-to-implement-getch-function-of-c-in-linux)*:
```C
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

int getch(void) {
    // credits: https://stackoverflow.com/questions/3276546/how-to-implement-getch-function-of-c-in-linux
    struct termios oldattr, newattr;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO | ISIG ); // Here, we flip these flags, so we disable them
    //                    ^        ^      ^
    //                    |        |      Killing with ^C and ^Z
    //                    |        Printing inputs to stdout
    //                    Canonical mode (buffered, waits for enter to be pressed)
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    int c = getchar(); // Like `getc(stdin)` but more adequate as we don't consider the standard
                       // input as a file in this case
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr ); // Set back the old attributes so the terminal is
                                                  // unchanged after this function call
    return c;
}
```
This works by changing the terminal I/O settings so that the standard input is no longer buffered for the duration of this function (this is why `oldattr` is queried, to put the terminal back to its initial state before the function is exited). As a bonus, we also make sure that `ctrl+C` does not kill the program directly so that we may add our own logic before really killing the program (for example, making sure to write some data to a file before exiting).

To finalize this function, we can use *macro magic* (basic `#if .. #else ... #endif`) to make our code compilable on both Windows and Linux:
```C
#include <stdio.h>

#ifdef _WIN32
#   include <conio.h>
#   define getch _getch
#else
#   include <termios.h>
#   include <unistd.h>
    int getch(void) {
        // credits: https://stackoverflow.com/questions/3276546/how-to-implement-getch-function-of-c-in-linux
        struct termios oldattr, newattr;
        tcgetattr( STDIN_FILENO, &oldattr );
        newattr = oldattr;
        newattr.c_lflag &= ~( ICANON | ECHO | ISIG );
        tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
        int c = getchar();
        tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
        return c;
    }
#endif
```
Now that we can retrieve inputs as they come, we can start to analyse what they look like. For characters from `'a'` to `'z'`, from `'A'` to `'Z'` as well as some punctuation and whitespaces like `'\n'`, the `getch` function returns exactly their *ASCII code* (as these are all ASCII characters). For more complex inputs like `left_arrow` (which is not represented by a number in the ASCII standard), an *escape sequence* can occur. For the arrows, these sequences are:
- `"\033[A"` for `up_arrow`.
- `"\033[B"` for `down_arrow`.
- `"\033[C"` for `right_arrow`.
- `"\033[D"` for `left_arrow`.

> **(i)** Note that these are also the sequences for moving the *cursor* (position from which the text is printed in the terminal window) in these directions. This shows how the escape sequences can be used to *dialogue* with the terminal (for example, we can ask for the cursor position with `"\033[6n"` and the terminal will respond in the *standard input* with `"\033[`*`n`*`;`*`m`*`H"`, that we know to be the sequence for moving the cursor to row `n` and column `m`).

These inputs are composed of multiple characters so we have to call `getch` three times to retrieve the complete input, which can make our code messy when we will start to link inputs to game logic. To account for that, I decided to create an *enum* with the non-alphanumerical inputs and a function to deal with the `getch` calls for me:
```C
/// @brief Some non-alphanumerical inputs we might expect
typedef enum InputKey {
#ifdef _WIN32
    INPUT_ENTER       = '\r',
    INPUT_BACKSPACE   = '\b',
#else
    INPUT_ENTER       = '\n',
    INPUT_BACKSPACE   = 127,
#endif
    INPUT_A_CTRL      = 1,
    INPUT_C_CTRL      = 3,
    INPUT_V_CTRL      = 22,
    INPUT_X_CTRL      = 24,
    INPUT_Z_CTRL      = 26,
    
    ESCAPE_CODE       = 27,
    __INPUT_NON_ASCII = 256, // This ensure no collisions with existing ASCII characters codes, as
                             // as the next entries will count up from 256:
    INPUT_UP,    // 257
    INPUT_DOWN,  // 258
    INPUT_RIGHT, // 259
    INPUT_LEFT,  // 260
} input_key;

/// @brief Get the last pressed key
/// @return The pressed input_key
int inputGet() {
    const int base = getch();
    if (base != ESCAPE_CODE) return base;

    const int expect_sqr_bracket = getch();
    if (expect_sqr_bracket != '[') return 0;

    const int c = getch();
    switch (c) {
        case 'A': return INPUT_UP;
        case 'B': return INPUT_DOWN;
        case 'C': return INPUT_RIGHT;
        case 'D': return INPUT_LEFT;
        default:  return 0;
    }
}
```
This function either returns an ASCII character or value from the aforementionned enum if it is recognized as such. In any other case, it returns `0` (so that it may be interpreted as `false`). Now that we have a single function to call, we can query the user input in a loop like so :
```C
while (1)
{
    int input = inputGet();
    switch(input)
    {
        case 'a': {
            // Action to realize for pressing 'a' without CTRL or MAJ pressed too
        } break;
        ...
        case INPUT_UP: {
            // Action to realize for pressing up arrow without CTRL or MAJ pressed too
        } break;
        ...
        default: {
            fprintf(stderr, "UNKNOWN INPUT %d\n", input);
        } break;
    }
}
```
> **(i)** The `getch` function (on both Windows and Linux) is still *blocking* in the sense that it waits for the user to press a key before returning a value (aka. the calling thread is put to sleep and woken up by the user input). This means that we can't do *inputs pulling* (waiting on user inputs) in the same thread as the rest of the application (in our case, the game update and the rendering) with this method. An alternative would be to make the standard input *non-blocking* using `fcntl(STDIN_FILENO, ``F_SETFL, ``fcntl(STDIN_FILENO, ``F_GETFL) | O_NONBLOCK)` on Linux or use `getch_nolock` on Windows and just check for *non-zero values*.

In this project we will accomodate for the *blocking* nature of `getch` by calling it in a separate *thread* and store the pressed inputs using a global variable:
```C
struct inputs
{
    union { // Using a union allows us to iterate over all inputs in a loop in `inputs_update`
        struct {
            bool key_a:  // The simple `a` key without CTRL or MAJ
            bool key_up; // The simple up arrow key without CTRL or MAJ
        };
        bool inputs[2];
    };
    pthread_mutex_t mutex; // Because the data is shared between two threads, we need to resolve
                           // collisions when writting by using a mutex
} INPUTS = { // We use a global variable here as there will only ever be one source for user inputs
             // and we never manipulate this variable directly (we always use functions which use a
             // mutex)
    .mutex = PTHREAD_MUTEX_INITIALIZER
};

// Shorthand for pthread_mutex_lock(&INPUTS.mutex) and pthread_mutex_unlock(&INPUTS.mutex)
void inputs_mutex(bool lock)
{
    (lock ? pthread_mutex_lock : pthread_mutex_unlock)(&INPUTS.mutex);
}
// Function executed in another thread to get user inputs, passed to `pthread_create`
void *inputs_thread_func(void *_)
{
    while (1)
    {
        int key = inputGet(); // The infamous blocking function

        inputs_mutex(true);
        switch (key) {
            case 'a':      INPUTS.key_a  = true; break;
            case INPUT_UP: INPUTS.key_up = true; break;
            default:                             break;
        }
        inputs_mutex(false);
    }
}
// Function to be called once per frame which resets the inputs to not being pressed
void update_inputs()
{
    inputs_mutex(true);
    for (int i = 0; i < 2; ++i) INPUTS.inputs[i] = false;
    inputs_mutex(false);
}

int main()
{
    // Initialization code ...
    pthread_t inputs_thread;
    pthread_create(&inputs_thread, NULL, inputs_thread_func, NULL);

    while (1)
    {
        // Logic for updating the game, rendering...
        
        update_inputs();   // Last thing in the loop, we reset the input so that it may be updated
                           // again during the sleep 
        sleep(1.0 / 30.0); // Sleep to set target FPS to 30 (let's assume `sleep` takes in a float
                           // for now, we will redefine it as so later anyways)
    }
}
```

# 2. Architecture of the project
Now that we have a way to display information and a way to acquire user input, we're all set... Or are we ? *Queue in Moon Men by Jake Chudnow...* Yeah we basicaly are, but let's organize everything so that we know how our final project's source code will look like.

We are dealing with three main sections:
- **Game logic**: where the state of the application is changed (in our case, where the game state is updated).
- **Rendering**: where the state of the application is converted to be displayed (in our case, the game of tetris in the terminal window)
- **Inputs processing**: where the user inputs are captured to be used by the application.

We know that we have to separate the inputs processing in a separate thread, but game logic and rendering don't create the same kind of dependency so they can be executed sequencialy in the same thread, which will be the *main* thread (the one running the *main* function). Because the inputs data is very much independant from the rest of the program's data, and because the input thread **only** manipulates the input data, we are safe to shove everything related to game logic and rendering inside of a global variable to be accessed from any function (we could pass it as an argument everytime but why bother when there is no benefit?).

Therefore, here is the final program's structure:
```C
static struct data
{
    struct game_data      { ... } GAME;      // All the global game state
    struct rendering_data { ... } RENDERING; // All the global render state
    struct inputs_data    { ... } INPUTS;    // All the global inputs state (as previously seen)
} DATA = {
    .INPUTS.mutex = PTHREAD_MUTEX_INITIALIZER
    // ...
};

int main(int argc, char **argv)
{
    // Initialization code ...
    initialize_rendering_logic();
    initialize_gameplay_logic();
    initialize_inputs_logic();

    // Start inputs thread
    pthread_t inputs_thread;
    pthread_create(&inputs_thread, NULL, inputs_thread_func, NULL);

    // Enter main loop
    while (1)
    {
        update_game();
        render();
        update_inputs(); 
        
        sleep(1.0 / 30.0); // 30 FPS
    }
}
```

> ### /!\ WORK IN PROGRESS /!\
> This section will continue to be worked on

# 3. Practical example: The game of Tetris
Let's define what we want our Tetris implementation to achieve in terms of functionnality and appearence so that we know what we are working towards.

> ### /!\ DISCLAIMER: I'M NOT A TETRIS PLAYER AND HAVE MADE THIS CLONE WITHOUT REALLY RESEARCHING HOW THE GAMES WORK! I'M ONLY DOING THIS FOR FUN! /!\

For starters

## 3.1 Creating a simple menu

## 3.2 How to manage the pieces

## 3.3 The core gameplay loop

> ### /!\ WORK IN PROGRESS /!\
> This page will continue to be worked on

