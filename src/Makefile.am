# bin_PROGRAMS = mipscore

lib_LTLIBRARIES = libmipscore.la
noinst_LIBRARIES = libmipscore.a

libmipscore_la_SOURCES = channel.cxx channel.h\
                   comparator.h\
                   dllmain.cxx dllmain.h\
                   event.cxx event.h\
                   exception.h\
                   mips.cxx mips.h\
                   mipscore.cxx mipscore.h\
                   mipsrecfile.cxx mipsrecfile.h\
                   record.cxx record.h\
                   recordset.cxx recordset.h\
                   sensor.cxx sensor.h

libmipscore_a_SOURCES = channel.cxx channel.h\
                   comparator.h\
                   event.cxx event.h\
                   exception.h\
                   mips.cxx mips.h\
                   mipscore.cxx mipscore.h\
                   mipsrecfile.cxx mipsrecfile.h\
                   record.cxx record.h\
                   recordset.cxx recordset.h\
                   sensor.cxx sensor.h

libmipscore_la_CPPFLAGS = -DUNICODE $(PYTHON_CPPFLAGS)
libmipscore_la_CXXFLAGS = -fpic -Wno-deprecated -Wno-invalid-offsetof -Wno-write-strings -DUNICODE $(PYTHON_CFLAGS)
libmipscore_la_LDFLAGS  = -shared $(PYTHON_LIBS) $(PTHREAD_CFLAGS)

libmipscore_a_CPPFLAGS = -DUNICODE $(PYTHON_CPPFLAGS)
libmipscore_a_CXXFLAGS = -fpic -Wno-deprecated -Wno-invalid-offsetof -Wno-write-strings -DUNICODE $(PYTHON_CFLAGS)

# include_HEADERS =

