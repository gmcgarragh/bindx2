DESCRIPTION
------------
bindx2 was originally developed to produce bindings to the XRTM (X Radiative Transfer Model) interface, written in C, for C++, Fortran 77, Fortran 90, IDL, Julia, and Python, based on a common interface definition file.  I am making this available for XRTM developers and also as some people have thought it might be useful for other applications.  In my opinion it is not very general and is geared towards the native interface to XRTM so I am not sure if it will be useful for other applications.


INSTALLATION
------------
The source maybe obtained by git cloning from github.com with:

git clone https://github.com/gmcgarragh/bindx2.git

The main code is C89 compliant and is dependent only on the C standard library and flex which should be apart of any Linux distribution.  The code should compile with any modern C compiler and probably with most C++ compilers.  The build system requires GNU Make, with which the following steps will compile the code:

1) Copy make.inc.example to make.inc.
2) Edit make.inc according to the comments within.
3) Run make.

The default setup in make.inc.example is for GCC and should work on any Linux or MacOS systems with programming tools installed and probably most modern UNIX systems.

For other platforms/environments, such as MS Windows Visual C++, it is up to user to set up the build within that platform/environment.

After the build the binary will be located in the same directory as the source.  It is up to the user to move it to or link to it from other locations.


USAGE
-----
Sorry but there is no documentation yet.  I will try to do this soon.  Take a look at the comand line options and arguments in bindx2.c.  It should be fairly obvious how to run it.  Then take a look at the interface definitions files included with XRTM ad compare it to the actual XRTM C interface.


CONTACT
-------
For questions, comments, or bug reports contact Greg McGarragh at greg.mcgarragh@colostate.edu.

Bug reports are greatly appreciated!  If you would like to report a bug please include the interface defintion file that reproduces the bug.
