MAKE = gmake
CC = g++9

INCDIR =
LIBDIR =
BINDIR = ..
OBJDIR = .obj

GCC_VERSION = 4
BSD_VERSION = $(shell uname -v 2>&1 | cut -d' ' -f2 | cut -d'.' -f1)
#SVN_VERSION = $(shell svn info |grep Revision: |cut -c11-)
SVN_VERSION = $(shell cat ../../__VERSION__)
$(shell if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi)

TARGET = $(BINDIR)/db_r$(SVN_VERSION)

CFLAGS = -m32 -g -Wall -O2 -pipe -fno-rtti -fno-exceptions -pthread -fno-strict-aliasing -D_THREAD_SAFE
CFLAGS += -Wno-deprecated-declarations -Wno-format-truncation -Wno-nonnull-compare
CFLAGS += -Wno-int-in-bool-context
CXXFLAGS = -std=c++11 -Wl,-rpath=/usr/local/lib/gcc9

# boost
INCDIR += -I../../../Extern/include/boost

# Extern
INCDIR += -I../../../Extern/include
LIBDIR += -I../../../Extern/lib

# Project Library
INCDIR += -I/usr/local/include
LIBDIR += -L../../libthecore/lib -L../../libsql -L../../libpoly -L../../libgame/lib
LIBS += -lthecore -lsql -lpoly -lgame -lm -lc++

# MySQL
INCDIR += -I/usr/local/include/mysql
LIBS += /usr/local/lib/mysql/libmysqlclient.a /usr/lib/libz.a /usr/local/lib/libzstd.a

#OpenSSL
LIBDIR += -L/usr/local/lib 
LIBS += -lunwind -lssl -lcrypto


SRCS =	Config.cpp NetBase.cpp Peer.cpp PeerBase.cpp Main.cpp Lock.cpp DBManager.cpp \
		Cache.cpp LoginData.cpp ClientManager.cpp ClientManagerPlayer.cpp ClientManagerLogin.cpp \
		ClientManagerBoot.cpp ClientManagerParty.cpp ClientManagerGuild.cpp GuildManager.cpp HB.cpp \
		PrivManager.cpp MoneyLog.cpp ItemAwardManager.cpp ClientManagerEventFlag.cpp Marriage.cpp \
		Monarch.cpp BlockCountry.cpp ItemIDRangeManager.cpp ClientManagerHorseName.cpp version.cpp \
		AuctionManager.cpp ProtoReader.cpp CsvReader.cpp 

OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)

default: $(TARGET)

$(TARGET): $(OBJS)
	@echo linking $(TARGET) ...
	@$(CC) $(CFLAGS) $(CXXFLAGS) $(LIBDIR) $(OBJS) $(LIBS) -o $(TARGET)
	@touch version.cpp

$(OBJDIR)/%.o: %.cpp
	@echo compile $<
	@$(CC) $(CFLAGS) $(CXXFLAGS) $(INCDIR) -c $< -o $@

$(OBJDIR)/version.o: version.cpp
	@$(CC) $(CFLAGS) $(CXXFLAGS) -D__USER__=\"$(USER)\" -D__HOSTNAME__=\"$(HOSTNAME)\" -D__PWD__=\"$(PWD)\" -D__SVN_VERSION__=\"$(SVN_VERSION)\" -c $< -o $@
	@echo compile $<

$(OBJDIR):
	@mkdir $(OBJDIR)

clean:
	@rm -f $(OBJS) $(BINDIR)/db_r*

dep:
	@touch Depend
	makedepend -fDepend $(INCDIR) -I/usr/include/c++/3.3 -I/usr/include/c++/4.2 -p$(OBJDIR)/ $(SRCS) 2> /dev/null

sinclude Depend
