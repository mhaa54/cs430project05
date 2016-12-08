# Specify compiler
CC=cl.exe

# Specify linker
LINK=link.exe

.PHONY : all
all : ezview

# Link the object files into a binary
ezview : ezview.obj ppmrw_io.obj
	$(LINK) /OUT:ezview.exe ezview.obj ppmrw_io.obj 

# Compile the source files into object files
ezview.obj ppmrw_io.obj : ezview.cpp ppmrw_io.cpp
	$(CC) /c ezview.cpp /Foezview.obj
	$(CC) /c ppmrw_io.cpp /Foppmrw_io.obj

# Clean target
clean :
	del ezview.obj ppmrw_io.obj ezview.exe

