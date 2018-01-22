#-------------------------------------------------
#
# Project created by QtCreator 2016-11-30T11:06:38
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = searcher
TEMPLATE = app


SOURCES += \
    main.cpp \
    widget.cpp \
    crawlerthread.cpp \
    widget_crawlslots.cpp \
    widget_funcs.cpp \
    crawler3.1.cpp \
    widget_initridslots.cpp \
    initridthread.cpp \
    rid2.1.cpp \
    expr.cpp

HEADERS  += \
    widget.h \
    cppjieba-master/include/cppjieba/Jieba.hpp \
    cppjieba-master/deps/limonp/ArgvContext.hpp \
    cppjieba-master/deps/limonp/BlockingQueue.hpp \
    cppjieba-master/deps/limonp/BoundedBlockingQueue.hpp \
    cppjieba-master/deps/limonp/BoundedQueue.hpp \
    cppjieba-master/deps/limonp/Closure.hpp \
    cppjieba-master/deps/limonp/Colors.hpp \
    cppjieba-master/deps/limonp/Condition.hpp \
    cppjieba-master/deps/limonp/Config.hpp \
    cppjieba-master/deps/limonp/FileLock.hpp \
    cppjieba-master/deps/limonp/ForcePublic.hpp \
    cppjieba-master/deps/limonp/LocalVector.hpp \
    cppjieba-master/deps/limonp/Logging.hpp \
    cppjieba-master/deps/limonp/Md5.hpp \
    cppjieba-master/deps/limonp/MutexLock.hpp \
    cppjieba-master/deps/limonp/NonCopyable.hpp \
    cppjieba-master/deps/limonp/StdExtension.hpp \
    cppjieba-master/deps/limonp/StringUtil.hpp \
    cppjieba-master/deps/limonp/Thread.hpp \
    cppjieba-master/deps/limonp/ThreadPool.hpp \
    cppjieba-master/include/cppjieba/DictTrie.hpp \
    cppjieba-master/include/cppjieba/FullSegment.hpp \
    cppjieba-master/include/cppjieba/HMMModel.hpp \
    cppjieba-master/include/cppjieba/HMMSegment.hpp \
    cppjieba-master/include/cppjieba/KeywordExtractor.hpp \
    cppjieba-master/include/cppjieba/MixSegment.hpp \
    cppjieba-master/include/cppjieba/MPSegment.hpp \
    cppjieba-master/include/cppjieba/PosTagger.hpp \
    cppjieba-master/include/cppjieba/PreFilter.hpp \
    cppjieba-master/include/cppjieba/QuerySegment.hpp \
    cppjieba-master/include/cppjieba/SegmentBase.hpp \
    cppjieba-master/include/cppjieba/SegmentTagged.hpp \
    cppjieba-master/include/cppjieba/TextRankExtractor.hpp \
    cppjieba-master/include/cppjieba/Trie.hpp \
    cppjieba-master/include/cppjieba/Unicode.hpp \
    crawlerthread.h \
    rid.h \
    crawler.h \
    initridthread.h \
    mystack.h \
    expr.h
FORMS    += widget.ui

CONFIG += c++11

DISTFILES +=

LIBS += -lws2_32

DEFINES-= UNICODE #for ExecuteShellA's use

RC_FILE = icon.rc #for exe's icon
