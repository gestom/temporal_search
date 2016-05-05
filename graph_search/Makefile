#
# Date:      2014/08/19 09:15
# Author:    Miroslav Kulich


BUILDDIR=./build/
SRCDIR=./src/
OBJS=$(patsubst $(SRCDIR)%.cc,$(BUILDDIR)%.o,$(wildcard $(SRCDIR)*.cc))
LIBOBJS=$(filter-out $(BUILDDIR)main.o,$(OBJS))
TARGET=search

#MAKE=make

LDFLAGS+=-L/usr/local/lib -L/usr/lib/



target: $(TARGET) libsearch.a

libsearch.a:	$(LIBOBJS)
	rm -f $@
	ar cq $@ $(LIBOBJS)
	$(info library $@ made from: [${LIBOBJS}])


$(OBJS): $(BUILDDIR)%.o: $(SRCDIR)%.cc
	$(CXX) -O2   -Wall -c $< $(CXXFLAGS) $(CPPFLAGSS) -o $@


$(TARGET): $(OBJS)

	$(CXX) $(OBJS) $(LDFLAGS) -O2 -Wall -o $@

clean:
	$(RM) $(OBJS) $(TARGET)
