# Standalone check of the Exult tts.cc (from the Exult tree) with stub config/utils headers.
EXULT ?= ../exult
CXX = g++
CXXFLAGS = -std=gnu++17 -Wall -Wextra -g

all: tts_test

tts_test: test/tts_test.cc $(EXULT)/tts.cc $(EXULT)/tts.h
	$(CXX) $(CXXFLAGS) -Itest/inc -I$(EXULT) -o $@ test/tts_test.cc $(EXULT)/tts.cc

check: tts_test
	./tts_test >/dev/null
	diff -u test/expected.log test/said.log && echo 'tts check ok'

clean:
	rm -f tts_test test/said.log

.PHONY: all check clean
