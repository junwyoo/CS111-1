run:
	@python lab3b.py > lab3b_check.txt

check:
	sort lab3b_check.txt -o lab3b_check.txt
	sort lab3b_correct.txt > sorted_correct.txt
	diff lab3b_check.txt sorted_correct.txt

dist:
	@rm -rf lab3b-304184918.tar.gz
	tar -czf lab3b-304184918.tar.gz *

clean:
	@rm -rf lab3b_check.txt sorted_correct.txt

