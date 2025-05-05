OBJ_DIR:=build
SRC_DIR:=source
HDR_DIR:=include

CC:=g++

WARNINGS := -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat   	\
			-Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts 	\
			-Wconditionally-supported -Wconversion -Wctor-dtor-privacy 		  	\
			-Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security  	\
			-Wformat-signedness -Wformat=2 -Winline -Wlogical-op 			  	\
			-Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked	  	\
			-Wpointer-arith -Winit-self -Wredundant-decls -Wshadow 			  	\
			-Wsign-conversion -Wsign-promo -Wstrict-null-sentinel 				\
			-Wstrict-overflow=2 -Wsuggest-attribute=noreturn 					\
			-Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override 	\
			-Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code \
			-Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix 		\
			-Wno-missing-field-initializers -Wno-narrowing 						\
			-Wno-old-style-cast -Wno-varargs -Wstack-protector

ASAN_FLAGS := -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

OTHER := -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla

CFLAGS := -g -D _DEBUG -DHASH_TABLE_DEBUG -ggdb3 -std=c++17 -O0 -march=native -Wall  $(WARNINGS) $(OTHER)

PERF_FLAGS := -fno-omit-frame-pointer  
#-fno-optimize-sibling-calls -fno-inline

RELEASE_FLAGS := -DNDEBUG -g -O3 -std=c++17 -march=native 

BUILD := DEBUG
ASAN = 1
ifeq ($(BUILD),RELEASE)
	override CFLAGS := $(RELEASE_FLAGS)
else ifeq ($(ASAN), 1)
	override CFLAGS := $(CFLAGS) $(ASAN_FLAGS)
endif

ifeq ($(BUILD),PERF)
	override CFLAGS := $(RELEASE_FLAGS) $(PERF_FLAGS)
endif

override CFLAGS += -I./$(HDR_DIR)

EXEC_NAME = hashMap.exe

$(EXEC_NAME): $(addprefix $(OBJ_DIR)/,hashTable_v1.o hashTable_v2.o perfTester.o textParse.o crc32.o main.o)
	$(CC) $(CFLAGS) $^ -o $@

static: $(OBJ_DIR)/hashTable.o
	mkdir -p $(OBJ_DIR)
	ar rcs $(OBJ_DIR)/libhashTable.a $^

#  There are two versions of hashTable, but one of them is deactivated
$(OBJ_DIR)/hashTable_v1.o: $(SRC_DIR)/hashTable_v1.c $(HDR_DIR)/hashTable.h
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/hashTable_v1.c -o $@

$(OBJ_DIR)/hashTable_v2.o: $(SRC_DIR)/hashTable_v2.c $(HDR_DIR)/hashTable.h
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/hashTable_v2.c -o $@

$(OBJ_DIR)/crc32.o: $(SRC_DIR)/crc32.s
	mkdir -p $(OBJ_DIR)
	nasm -g -f elf64  -l $(OBJ_DIR)/crc32.lst $< -o $@

$(OBJ_DIR)/perfTester.o: $(SRC_DIR)/perfTester.c $(HDR_DIR)/hashTable.h $(HDR_DIR)/perfTester.h
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


$(OBJ_DIR)/textParse.o: $(SRC_DIR)/textParse.c $(HDR_DIR)/hashTable.h $(HDR_DIR)/perfTester.h
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c $(HDR_DIR)/hashTable.h $(HDR_DIR)/perfTester.h
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean compile_commands test_file perfTest run

clean:
	rm build/* || true

compile_commands:
	make clean
	bear -- make BUILD=DEBUG

TESTS = 10000000 # 10 millions
FOUND_PERCENT = 0.9
TEST_FILE = shakespeare.txt
test_file:
	g++ scripts/generateTest.c -o scripts/generateTest.exe
	g++ scripts/prepareText.c  -o scripts/prepareText.exe
	./scripts/prepareText.exe $(TEST_FILE) testStrings.txt
	./scripts/generateTest.exe testStrings.txt testRequests.txt $(TESTS) $(FOUND_PERCENT)


FREQ = 10000
FLAMEGRAPH_PATH = ~/Utils/FlameGraph/

perfTest:
	make clean
	make BUILD=PERF
	sudo perf record -g --call-graph dwarf -F $(FREQ) ./$(EXEC_NAME)
# 	sudo perf script | \
# 	$(FLAMEGRAPH_PATH)stackcollapse-perf.pl | \
# 	$(FLAMEGRAPH_PATH)flamegraph.pl > ./flame.svg

run:
	make clean
	make BUILD=RELEASE
	taskset 0x1 ./$(EXEC_NAME)
	for i in `seq 1 5`; do \
		taskset 0x1 ./$(EXEC_NAME) -s;\
	done;

