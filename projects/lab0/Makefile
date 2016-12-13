CC = gcc
CFLAGS = -g -Wall
TARGET = lab0

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

check:
	@echo "Test 1: --input flag only"
	@echo "This is a test file for project zero" > test_input.txt
	@echo "Created test input file called test_input.txt\n"
	@echo "Running executable with flag --input=test_input.txt"
	@./lab0 --input=test_input.txt
	@echo "\nTest 2: --input and --output flags"
	@echo "Running executable with input flag using the same input file from test 1,"
	@echo "Output flag, creating output file to: test_output.txt"
	@./lab0 --input=test_input.txt --output=test_output.txt
	@echo "\nTesting differences with diff command. Should return nothing or error message if different."
	@diff test_input.txt test_output.txt > /dev/null || (echo "Error: input and output files are different")
	@echo "\nRemoving both test files"
	@rm -rf test_input.txt test_output.txt
	@echo "\nTest 3: --segfault and --catch flags"
	@echo "Running executable with both --segfault and --catch flags"
	@echo "Should display a SIGSEGV caught message and catch exit code 3"
	@./lab0 --segfault --catch 2>&1 || [ $$? -eq 3 ]
clean:
	$(RM) $(TARGET)

dist: 
	rm -rf lab0-304184918.tar.gz
	tar -czf lab0-304184918.tar.gz *.c *.png README Makefile 
