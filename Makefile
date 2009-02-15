# root of the project tree, relative from the current directory
PROJECT_ROOT := .

# include global makefile
include $(PROJECT_ROOT)/build/global.mak


all: nms-client

nms-client: nms-common
	$(MAKE) -C $(CLIENTDIR)
	$(CP) $(CLIENTDIR)/$@ $(BINDIR)/

nms-common:
	$(MAKE) -C $(COMMONDIR)

# clean targets
clean: clean-common clean-client
	-$(RMF) $(BINDIR)/*

clean-common:
	$(MAKE) -C $(COMMONDIR) clean

clean-client:
	$(MAKE) -C $(CLIENTDIR) clean