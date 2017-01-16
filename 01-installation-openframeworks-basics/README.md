<a name="session-01---012417---installation--openframeworks-basics"></a>
# Session 01 - 01/24/17 - Installation / openFrameworks basics

<a name="course-overview"></a>
## Course Overview

This course breaks down the components of audio and video using openFrameworks and allows you to explore their contents and manipulate them in real-time.  We'll look at some basic components of openFrameworks and how they can be used to explore audio and video data in real-time, and how what we build can be used for interactive installation, generative video, and exploratory visualizations.

Homeworks will be worth about 50% of your overall grade, and 2 projects which are worth 25% each. Pretty much every session will have a homework, not too demanding but hopefully designed to help you and each other understand, and provide a way for me to give you feedback. The first homework for instance will be to explain the program flow of an example in openFrameworks, describing all variables, methods, classes, and objects that are used in the main and ofApp files.

<a name="topics"></a>
## Topics

<a name="installation"></a>
### Installation

Download the zip file and place it somewhere.  E.g. ~/dev/openFrameworks

<a name="openframeworks-folder-structure"></a>
### openFrameworks folder structure

- /addons
- /apps
- /docs
- /examples
- /export
- /libs
- /other
- /projectGenerator
- /scripts

<a name="creating-a-project"></a>
### Creating a project

- [EASY] Project Generator
- [MEDIUM] Copy existing project but maintain "hierarchy" with relationship to base oF directory
- [HARD] Makefile based projects just need to refer to the base oF directory and use an existing makefile + addons.make

<a name="project-structure"></a>
### Project structure

- /src  
- /bin  
- /bin/data  
- ../../../addons  

<a name="xcode-walkthrough"></a>
### Xcode Walkthrough

- Folder navigator  
- Schemes  
- Target executable  
- Project and application settings

<a name="main-entrypoint"></a>
### Main Entrypoint

Every c++ program needs a "main" function.  This is where the program begins and then continues execution, line by line.  We can put everything into main.cpp and play with a camera like so:

```cpp
#include "ofMain.h"

class ofApp : public ofBaseApp{
public:
    // i get called once
    void setup() {
        cam.setup(width, height);
        ofSetWindowShape(width, height);
        ofSetFrameRate(1);
    }
    
    // i get called in a loop that runs until the program ends
    void update() {
        cam.update();
    }
    
    // i also get called in a loop that runs until the program ends,
    // just after update is called.
    void draw() {
        cam.draw(0, 0);
    }
private:
    ofVideoGrabber cam;
    int width = 640;
    int height = 480;
};


int main(){
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new ofApp());
}
```

<a name="headers-declaration-files-h-files"></a>
### Headers, declaration files, .h files

Generally instead of putting everything into a giant file, we split them up into headers, or declaration files, and source files.  The header files provide an interface to everything that will be defined.  For those of you that haven't programmed before, this won't make much sense right now but stick with it.  The header files is like a blueprint.  It doesn't actually define anything, meaning you can't actually execute any of this code.  It is just showing you all the code that once it is defined, can be executed.

So let's see a simple example:

```cpp
void addTwoNumbers(int a, int b);
```

This is an example of a function declaration.  It's curly braces haven't been opened and the function isn't defined.  It's just the declaration.  I'm declaring that I will define it at some point.

**ofApp.h**

```cpp
#pragma once
 
#include "ofMain.h"
 
class ofApp : public ofBaseApp{
 
    public:
         
        // declaration of functions
        void setup();
        void update();
        void draw();
};
```

<a name="source-files-definition-files-cpp-files"></a>
### Source files, definition files, .cpp files

**ofApp.cpp**

```cpp
// i get called once
void ofApp::setup(){
     
    // do some initialization
     
    // set the size of the window
    ofSetWindowShape(250, 250);
     
    // the rate at which the program runs (FPS)
    ofSetFrameRate(30);
     
}
 
// i get called in a loop that runs until the program ends
void ofApp::update(){
     
}
 
// i also get called in a loop that runs until the program ends,
// just after update is called.
void ofApp::draw(){
     
}
```

<a name="openframeworks-commands"></a>
### OpenFrameworks Commands

oF commands all start with "of".  If you are using an IDE w/ code completion, this makes coding a lot easier.

<a name="coding-style"></a>
### Coding Style

Stylistically, be consistent.

<a name="basic-window-setup-framerate-shape-fullscreen"></a>
### Basic window setup: framerate, shape, fullscreen

- Program flow

    * `main()`  
    * ...
    * `void setup()`  
    * `void update()`  
    * `void draw()`  
    * `void update()`  
    * `void draw()`  
    * `void update()`  
    * `void draw()`  
    * ...
    * `void exit()`  

- Framerate functions

    * `ofGetFrameRate()`  
    * `ofSetFrameRate(int rate)`  

<a name="keyboard-interaction"></a>
### Keyboard Interaction

- `void keyPressed(int key)`  
- `void keyReleased(int key)`  

<a name="mouse-interaction"></a>
### Mouse Interaction

- Called on the active window when the mouse is moved:
    `void mouseMoved( int x, int y )`
- Called on the active window when the mouse is dragged, i.e. moved with a button pressed:
    `void mouseDragged( int x, int y, int button )`
- Called on the active window when a mouse button is pressed:
    `void mousePressed( int x, int y, int button )`
- Called on the active window when a mouse button is released:
    `void mouseReleased(int x, int y, int button )`
- Called on the active window when the mouse wheel is scrolled:
    `void mouseScrolled(int x, int y, float scrollX, float scrollY )`

<a name="window-events"></a>
### Window Events

- resize  
- dragEvent  
- fullscreen  

<a name="introductory-coding-topics"></a>
### Introductory Coding Topics

#### What is a function?

They are blocks of code that run when you "call" them.  They have a return type, which can also be "void", meaning they don't return anything, and can have parameters, which are things that you must give to the function in order for it to run.  Here is a simple function (also known as a method):

```cpp
int addTwoNumbers(int firstNumber, int secondNumber)
{
    return firstNumber + secondNumber;
}
```

The return type is `int`.  Its name is `addTwoNumbers`.  And it takes two parameters of type `int` and `int`.  The names of the variables of these parameters are not important or necessary for us to know in order to use the function.  We can call the function by using its name, `addTwoNumbers`, and giving it values that are the same type as the required parameters it needs.  There are 2 required parameters in this function, both which should be integers.

Here is an example of how we'd call this function:

```cpp
cout << addTwoNumbers(10, 20) << endl;
```

This will call the function giving the value of 10 to `firstNumber` and 20 to `secondNumber`.  The code moves from the function call of `addTwoNumbers(10, 20)` to the function declaration `int addTwoNumbers(int firstNumber, int secondNumber)`, replacing the values of the variables `firstNumber` and `secondNumber` with 10 and 20.  Then the function executes, and returns the result of 10 + 20;

The `return` type of this function is `int`, meaning there is an integer returned. If it were `void`, we would not have had to return anything.

#### What are variables?

Variables are ways to store information.  They have a type, such as `int` for integers, `float` for floating point values, or `string` for strings.  Variables can also store more complicated types such as "instances" of a `class`.  We could have called the function above like so:

```cpp
void setup()
{
    int someNumber = 10;
    int x = addTwoNumbers(5, someNumber);
}
```

The value of x is now 15.

#### What is a `class`?

A `class` contains a set of methods and variables. We can instantiate an object in a very similar way as we do with an `int` or `float`, by simply creating a variable, e.g. `ofVideoGrabber camera`; When we create this object, we can call the public methods that are defined in the class definition.  That's unlike the `int` or `float` variable which does not have any methods.

#### What is the difference between a *method* and a *function*?

They mean the same thing. 

#### What is the difference between a class definition and a class declaration?

A definition is often contained in the .cpp file, and is the code that is run when a function is called. This is the code that goes between the curly braces after the function’s re-declaration in the .cpp file. A class declaration appears in the .h file, and only provides syntax for what the return type of a function is (e.g.: `void`, `int`, `float`, `string`, etc…), and what parameters the function takes.

#### What's the difference between an object and a class?

An object is an "instance" of a class.  Consider a simple class like so:

```cpp
class Box {
    public:
        int x, y, width, height;
};

Box box_1, box_2;
```

`Box` is a class, and we created two instances of it called `box_1` and `box_2`.  We could have called it whatever we wanted, e.g. `zebra`.  But often we try to use names of variables that are meaningful.

#### What are `if`/`else if`/`else` statements

`if` statements are ways to control the flow of execution.  They take "boolean" logic in order to understand whether to execute a block of code, or continue onto an `else` statement.  An example is below:

```cpp
if(5 > 2) {
    // do something
}
else if(4 > 2) {
    // this will never happen
}
else {
    // this will never happen
}
```

#### What are boolean operations?

* `>`: greater than
* `<`: less than
* `>=`: greater than or equal to
* `<=`: less than or equal to
* `==`: equal to
* `||`: or
* `&&`: and

#### For loops

`for` loops allow you to repeat the execution of a block of code, keeping track of the number of times the block of code has been executed.  This is useful for things like iterating over a list of elements, or a collection of some sort.  Or for doing something many times, perhaps with some slight variation each time.  Here is a simple example of a for loop:

```cpp
for(int i = 0; i < 10; i++) {
    cout << i << endl;
}
```

#### Comments
    
Comments allow us to add notes that aren't compiled/linked/executed.  These are handy for documenting your code.

```cpp
int x; // one line comment
/* 
block
comments
span
multiple
lines
*/ 
```

#### What is the difference between compiling, linking, and executing?

So there are three stages essentially for your program.  Compilation, linking, and execution.  And these each have their own types of errors.  Compiler errors, linker errors, and runtime errors.

What's compilation?  Once you have coded your program, you run a program called a compiler.  The compiler takes the c++ code, parses it, analyzes it, generates an abstract syntax tree, generates some immediate representation code, optimizes it, creates some assembly level language which some hardcore programmers actually write directly.  This finally produces what's called object code and is often just a .o file.  Every one of your header/source files generates 1 .o file.

After compiling, the linker combines all of the object files that you told it to compile into an executable file.  If you refer to some symbol in another object file, then the linker will be the one to tell you if you forgot to define it.  The compiler won't care.

Finally, once you have created your program and it is running, there can be runtime errors.  Maybe it tries to load a file that doesn't exist.  That is not a compiler or linker error.  It's more of a logic error.  The program expected it could do something and it couldn't.

So it's handy to know the basic difference between compilation, linking, and runtime errors.  Compilers care about the syntax.  Linkers care about whether you have actually defined what you said you would.  And at runtime, you just hope that everything happens as you thought it would.

<a name="where-to-find-stuff"></a>
## Where to find stuff

<a name="openframeworks"></a>
### OpenFrameworks

- [ofBook](http://openframeworks.cc/ofBook/chapters/foreword.html)
- [Forums](https://forum.openframeworks.cc/)
- [Documentation](http://openframeworks.cc/documentation/)
- [ofxAddons](http://ofxaddons.com/categories)

<a name="c"></a>
### C++

- [C++ Tutorial](http://www.cplusplus.com/doc/tutorial/)  
- [Another C++ Tutorial](http://www.cprogramming.com/tutorial/)  
- Bjarne Stroustrup’s The C++ Programming Language  
- Deitel & Deitel’s C++ How to Program  
- StackOverflow  

<a name="homework"></a>
## Homework

For your homework I want you to have a look at the examples, compile them, run them, explore the code, and make sure you don't have any issues getting code to run.  Try making tweaks and explore what you can do with just the examples.  They are a great place to get a sense of what is possible.

Then, I'd like you to pick your favorite example and attempt to explain what the code is doing.  I want you to describe the flow of the execution of the program from start to finish.  I also want you to describe every variable, method, class, and object that are used in the main.cpp and ofApp.h and ofApp.cpp files.  If you are feeling adventurous, you can even try an add-on that comes with examples.

You should submit your homework through the kannu portal.  You can upload a screencast or a document describing code in fragments.  This will allow each of you as well as myself to give you feedback and also get a chance to know everyone's capabilities and understanding a bit better.

<a name="lab"></a>
## Lab

For the next 50 minutes I'd like to make sure everyone has a working installation of openFrameworks and can create a project w/ the Project Generator and run an example.
