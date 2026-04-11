.PHONY: all banner clean

CFLAGS = -lssl -lcrypto

all: banner
	@echo "Building server and client..."
	@gcc server.c -o dist/server $(CFLAGS)
	@gcc client.c -o dist/client $(CFLAGS)
	@echo "Compilation complete."

banner:
	@echo "=================================================="
	@echo " _____                                          "
	@echo "/ ____|                                         "
	@echo "| (___   ___  ___  ___  ___ __ ___   _____ _ __ "
	@echo "\___ \ / _ \/ __| / __|/ _ \ '__\ \ / / _ \ '__| "
	@echo "____) |  __/ (__  \__ \  __/ |   \ V /  __/ |   	"
	@echo "|_____/\___|\___| |___/\___|_|    \_/ \___|_|   "
	@echo "=================================================="
	
clean:
	@echo "Cleaning up..."
	@rm -f server client