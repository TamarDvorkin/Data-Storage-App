# Linux-Storage-App Project

This Project is a requested engine framework that monitors folders and, upon changes, transfer files to storage area using 
Mater-Slave architecture and Design Pattrens.
The Framework enables:
1 Configing resources of requests(keyboard, NBD) and configuring tasks("q" on the keyboard, read/write).
2 Executing tasks asyncronously using a Thread Pool and Waitable Priorty Queue.
3 Plug and Play(pnp) - dynamically teaching the framework at runtime to create new objects.
4 Fail-safe operation - using Watch-Dog-Service (see project repo here: https://github.com/TamarDvorkin/Watch-Dog-Service).
5 Implementing syncronization using conditional variable, locks(mutex) and memory barrier.
6 Implementing Design Patterns : Publisher-Subscriber(pnp), Factory(factory), Object Pool(thread pool), Singelton(logger and factory).
7 Implementing a logger file that enable the programmer to choose the logging level for program tracking.


# How to install this project
1 Clone this project
2 Run with Makefile (see file "Run with Make" file).
3 When executing the framework, observe the log file (you can adjust the log level for your requirements).
