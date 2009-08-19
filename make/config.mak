
# --- commands section ---
RM := rm -f
RMDIR := rm -rf
DOCGEN := doxygen

# --- directory section ---

# this is a list of directories preceded with "-I", that contains the include
# directories
INCLUDEDIRS :=

# this is a list of directories preceded with "-L" that contains the Library
# directories
LIBDIRS :=


# configuration section

# boost library names follow a certain pattern
BOOST_PREFIX :=
BOOST_SUFFIX := -mt

# wxWidgets flags and libraries
WX_FLAGS := `wx-config --cxxflags`
WX_LIBS := `wx-config --libs`

# Warning flags
WARNFLAGS = -Wall -pedantic -ansi -Wno-long-long
# -Wno-long-long needed for wxWidget headers that use long long in ANSI C++

# Set additional Compiler flags, such as optimization or debuging
CXXFLAGS += -g

