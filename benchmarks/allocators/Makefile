# You may need to set the following variables:
#
# CXX      - path to Clang++ 3.6
# CC       - path to Clang 3.6
# AR       - path to LLVM's 'ar' tool
# RANLIB   - path to LLVM's 'ranlib' tool
# BDEWAF   - path to 'waf' from BDE Tools (https://github.com/bloomberg/bde-tools)

CXX = clang++-3.6
CC = clang-3.6
AR = llvm-ar-3.6
RANLIB = llvm-ranlib-3.6
BDEWAF = waf

OPTIM = -O -DBSLS_ASSERT_LEVEL_NONE
LTO = -flto
DEBUG = 
PARALLEL = 
GROWTH_SIZE = 20  # use 16 or less for testing

BSLSRC = ../..
BSL = $(BSLSRC)/groups/bsl
BDL = $(BSLSRC)/groups/bdl
BSLINC = $(INST)/include/bsl
BDLINC = $(INST)/include/bdl
BSLLIB = $(BSLSRC)/build/groups/bsl
BDLLIB = $(BSLSRC)/build/groups/bdl

STDLIB = -stdlib=libc++
# If libc++ was built from source (not installed from a system package), then libc++abi may not
# have been linked into libc++. In this situation, the linker must explicitly be told to include
# it in the link, by uncommenting the line below.
#STDLIBABI = -lc++abi

INCLUDES = \
   -I$(BSL)/bsls -I$(BSL)/bslma -I$(BSL)/bslscm -I$(BSL)/bslh \
   -I$(BSL)/bsl+bslhdrs -I$(BSL)/bslstl -I$(BSL)/bslmf -I$(BSL)/bslalg \
   -I$(BDL)/bdlscm -I$(BDL)/bdlma

DEFS = -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -DBSLS_IDENT_OFF

LIBS = \
   -Wl,-Bstatic \
   -L$(BDLLIB) -lbdl -L$(BSLLIB) -lbsl \
   -Wl,-Bdynamic \
   $(STDLIBABI) \
   -lpthread

CFLAGS_BDE = $(DEBUG) $(OPTIM) $(LTO) $(DEFS) $(CFLAGS) -std=c99
CXXFLAGS_BDE = $(DEBUG) $(OPTIM) $(LTO) $(DEFS) $(STDLIB)
CXXFLAGS_LOCAL = $(CXXFLAGS_BDE) -std=c++14 $(INCLUDES) $(CXXFLAGS)
LDFLAGS_LOCAL = $(LIBS) $(LDFLAGS)

all: run

BINARIES = growth \
           locality-AS1 locality-AS7 locality-AS9 locality-AS13 \
           zation tention

build: $(BINARIES)

$(BINARIES): bde-tag

OUTPUT =  *-result growth-*-*

clean:
	rm -rf $(BINARIES) $(OUTPUT) bde-tag $(BSLSRC)/build

bde-tag: Makefile
	@( \
	cd $(BSLSRC) && \
	export CC='$(CC)' && \
	export CXX='$(CXX)' && \
	export AR='$(AR)' && \
	export RANLIB='$(RANLIB)' && \
	export CFLAGS='$(CFLAGS_BDE)' && \
	export CXXFLAGS='$(CXXFLAGS_BDE)' && \
	$(BDEWAF) configure --abi-bits=64 --cpp11 --build-type release --assert-level=none $(PARALLEL) && \
	$(BDEWAF) build -v \
	)
	touch bde-tag

# section 7
growth: growth.cc allocont.h bde-tag
	$(CXX) -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)

# section 8
locality-AS1: locality.cc allocont.h
	$(CXX) -DSTDALLOC -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)
locality-AS7: locality.cc allocont.h
	$(CXX) -DCTMULTI -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)
locality-AS9: locality.cc allocont.h
	$(CXX) -DRTMULTI -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)
locality-AS13: locality.cc allocont.h
	$(CXX) -DRTMULTIMONO -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)

# section 9
zation: zation.cc
	$(CXX) -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)

# section 10
tention: tention.cc
	$(CXX) -o $@ $(CXXFLAGS_LOCAL) $< $(LDFLAGS_LOCAL)

run: run-growth run-locality run-zation run-tention

run-growth: growth
	@echo \"test-growth 20\" may take a full day to complete:
	./test-growth $(GROWTH_SIZE)
	cat growth-*-* >growth-result

run-locality: locality-AS1 locality-AS7 locality-AS9 locality-AS13
	@( \
	echo "********** using AS1 default std::allocator:"; \
	./test-locality ./locality-AS1 2>&1; \
	echo "********** using AS7 compile-time-bound multipool"; \
	./test-locality ./locality-AS7 2>&1; \
	echo "********** using AS9 polymorphic multipool:"; \
	./test-locality ./locality-AS9 2>&1; \
	echo "********** using AS13 polymorphic multipool backed by monotonic:"; \
	./test-locality ./locality-AS13 2>&1; \
	) | tee locality-result

run-zation: zation
	time ./test-zation | tee zation-result

run-tention: tention
	time ./test-tention | tee tention-result
