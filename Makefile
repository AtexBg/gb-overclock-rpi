all:
	@echo -e "\033[32mCompiling ./gboc binary...\033[0m"
	gcc main.c -lpigpio -o gboc
