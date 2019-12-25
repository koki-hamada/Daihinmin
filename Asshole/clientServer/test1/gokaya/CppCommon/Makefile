# 
# 1. General Compiler Settings
#
CXX       = g++
CXXFLAGS  = -std=c++14 -Wall -Wextra -Wcast-qual -Wno-unused-function -Wno-sign-compare -Wno-unused-value -Wno-unused-label -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter -fno-exceptions -fno-rtti \
            -pedantic -Wno-long-long -msse4.2 -mbmi -mbmi2 -mavx2 -D__STDC_CONSTANT_MACROS -fopenmp
INCLUDES  =
LIBRARIES = -lpthread -DHAVE_AVX2

#
# 2. Target Specific Settings
#
ifeq ($(TARGET),release)
	CXXFLAGS += -Ofast -DNDEBUG
        output_dir := out/release/
endif
ifeq ($(TARGET),debug)
	CXXFLAGS += -O0 -g -ggdb -D_GLIBCXX_DEBUG
        output_dir := out/debug/
endif
ifeq ($(TARGET),default)
	CXXFLAGS += -Ofast -g -ggdb
        output_dir := out/default/
endif

#
# 2. Default Settings (applied if there is no target-specific settings)
#
sources      ?= $(shell ls -R test/*.cc)
sources_dir  ?= src/test/
objects      ?=
directories  ?= $(output_dir)

#
# 4. Public Targets
#
default release debug profile:
	$(MAKE) TARGET=$@ preparation index_test long_bits_test random_test bit_array_test bit_operation_test bit_partition_test search_test solver_test string_test selection_test

run-coverage: coverage
	out/coverage --gtest_output=xml

clean:
	rm -rf out/*

scaffold:
	mkdir -p out test out/data doc lib obj resource

#
# 5. Private Targets
#
preparation $(directories):
	mkdir -p $(directories)

bit_array_test :
	$(CXX) $(CXXFLAGS) -o $(output_dir)bit_array_test $(sources_dir)bit_array_test.cc $(LIBRARIES)

bit_operation_test :
	$(CXX) $(CXXFLAGS) -o $(output_dir)bit_operation_test $(sources_dir)bit_operation_test.cc $(LIBRARIES)

bit_partition_test :
	$(CXX) $(CXXFLAGS) -o $(output_dir)bit_partition_test $(sources_dir)bit_partition_test.cc $(LIBRARIES)

search_test :
	$(CXX) $(CXXFLAGS) -o $(output_dir)search_test $(sources_dir)search_test.cc $(LIBRARIES)

solver_test :
	$(CXX) $(CXXFLAGS) -o $(output_dir)solver_test $(sources_dir)solver_test.cc $(LIBRARIES)

string_test :
	$(CXX) $(CXXFLAGS) -o $(output_dir)string_test $(sources_dir)string_test.cc $(LIBRARIES)

selection_test :
	$(CXX) $(CXXFLAGS) -o $(output_dir)selection_test $(sources_dir)selection_test.cc $(LIBRARIES)

random_test :
	$(CXX) $(CXXFLAGS) -o $(output_dir)random_test $(sources_dir)random_test.cc $(LIBRARIES)

long_bits_test :
	$(CXX) $(CXXFLAGS) -o $(output_dir)long_bits_test $(sources_dir)long_bits_test.cc $(LIBRARIES)

index_test :
	$(CXX) $(CXXFLAGS) -o $(output_dir)index_test $(sources_dir)index_test.cc $(LIBRARIES)

-include $(dependencies)
