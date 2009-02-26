# root of the project tree, relative from the current directory
PROJECT_ROOT := .

# include global makefile
include $(PROJECT_ROOT)/build/global.mak


all: nms-client nms-serv

nms-client: nms-common
	$(MAKE) -C $(CLIENTDIR)
	$(CP) $(CLIENTDIR)/$@ $(BINDIR)/

nms-serv: nms-common
	$(MAKE) -C $(SERVDIR)
	$(CP) $(SERVDIR)/$@ $(BINDIR)/

nms-common:
	$(MAKE) -C $(COMMONDIR)




# clean targets
clean: clean-common clean-client clean-serv
	-$(RMF) $(BINDIR)/*

clean-common:
	$(MAKE) -C $(COMMONDIR) clean

clean-client:
	$(MAKE) -C $(CLIENTDIR) clean

clean-serv:
	$(MAKE) -C $(SERVDIR) clean