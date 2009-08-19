# global makefile that is common to all programs of the project

CP := cp
RMF := rm -f

# include configuration file
include $(PROJECT_ROOT)/make/config.mak

# A few directory definitions
SRCDIR := $(PROJECT_ROOT)/src
BINDIR := $(PROJECT_ROOT)/bin

COMMONDIR := $(SRCDIR)/common
CLIENTDIR := $(SRCDIR)/client
SERVDIR := $(SRCDIR)/server


# Compiler and Liner Flag definitions
CXXFLAGS += $(WX_FLAGS) $(WARNFLAGS) $(INCLUDEDIRS) -I$(COMMONDIR)
LDFLAGS += $(LIBDIRS) -L$(COMMONDIR)

# compose all boost libraries used
B_LIB_SYSTEM := $(BOOST_PREFIX)boost_system$(BOOST_SUFFIX)
B_LIB_THREAD := $(BOOST_PREFIX)boost_thread$(BOOST_SUFFIX)
