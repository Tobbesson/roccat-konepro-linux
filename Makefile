gcc:
	gcc konepro.c -lusb-1.0 -lm -o konepro -std=c11 -Wall -pedantic -Wextra -Werror
debug:
	gcc konepro.c -lusb-1.0 -lm -o konepro -std=c11 -Wall -pedantic -Wextra -Werror -g
static:
	gcc konepro.c -l:libusb-1.0.a -lm -ludev -o konepro -std=c11 -Wall -pedantic -Wextra -Werror