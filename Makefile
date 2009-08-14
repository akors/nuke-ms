# root of the project tree, relative from the current directory
PROJECT_ROOT := .

# include global makefile
include $(PROJECT_ROOT)/build/global.mak


all: nuke-ms-serv

nuke-ms-client: nuke-ms-common
	$(MAKE) -C $(CLIENTDIR)
	$(CP) $(CLIENTDIR)/$@ $(BINDIR)/

nuke-ms-serv: nuke-ms-common
	$(MAKE) -C $(SERVDIR)
	$(CP) $(SERVDIR)/$@ $(BINDIR)/

nuke-ms-common:
	$(MAKE) -C $(COMMONDIR)

tests: nuke-ms-common
	$(MAKE) -C $(SRCDIR)/test



# clean targets
clean: clean-common clean-client clean-serv clean-tests
	-$(RMF) $(BINDIR)/*

clean-common:
	$(MAKE) -C $(COMMONDIR) clean

clean-client:
	$(MAKE) -C $(CLIENTDIR) clean

clean-serv:
	$(MAKE) -C $(SERVDIR) clean

clean-tests:
	$(MAKE) -C $(SRCDIR)/test clean
