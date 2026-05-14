/*
 * tests/test_app.c — User-space test for the custom_midi driver
 * Assigned to Person 4.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define DEVICE_PATH "/dev/custom_midi"
#define READ_BUF_SIZE 256

static volatile int running = 1;

static void sigint_handler(int sig)
{
	(void)sig;
	running = 0;
}

static void print_hex_dump(const unsigned char *data, int len)
{
	int i;
	printf("  ");
	for (i = 0; i < len; i++) {
		printf("0x%02X ", data[i]);
		if ((i + 1) % 16 == 0) printf("\n  ");
	}
	if (len % 16 != 0) printf("\n");
}

static const char *decode_midi_status(unsigned char byte)
{
	if (byte < 0x80) return NULL;
	switch (byte & 0xF0) {
	case 0x80: return "Note Off";
	case 0x90: return "Note On";
	case 0xA0: return "Aftertouch";
	case 0xB0: return "Control Change";
	case 0xC0: return "Program Change";
	case 0xD0: return "Channel Pressure";
	case 0xE0: return "Pitch Bend";
	case 0xF0:
		switch (byte) {
		case 0xF0: return "SysEx Start";
		case 0xF7: return "SysEx End";
		case 0xFE: return "Active Sensing";
		case 0xFF: return "System Reset";
		default:   return "System Message";
		}
	}
	return "Unknown";
}

int main(void)
{
	int fd;
	ssize_t bytes_read, bytes_written;
	unsigned char buf[READ_BUF_SIZE];
	int total_bytes = 0, read_count = 0;

	signal(SIGINT, sigint_handler);

	printf("===========================================\n");
	printf("  custom_midi — User-Space Test App (P4)  \n");
	printf("===========================================\n\n");

	fd = open(DEVICE_PATH, O_RDWR);
	if (fd < 0) {
		perror("Failed to open " DEVICE_PATH);
		printf("Hint: Run 'make load' first.\n");
		return EXIT_FAILURE;
	}
	printf("[+] Opened %s (fd=%d)\n\n", DEVICE_PATH, fd);

	/* Write a dummy SysEx command */
	unsigned char sysex[] = { 0xF0, 0x43, 0x10, 0x4C, 0x00, 0x00, 0x7E, 0x00, 0xF7 };
	printf("[*] Sending SysEx command:\n");
	print_hex_dump(sysex, sizeof(sysex));

	bytes_written = write(fd, sysex, sizeof(sysex));
	if (bytes_written < 0) perror("  write() failed");
	else printf("  -> %zd bytes written to driver\n\n", bytes_written);

	/* Continuous read loop */
	printf("[*] Reading MIDI stream (Ctrl-C to stop)...\n");
	while (running) {
		bytes_read = read(fd, buf, sizeof(buf));

		if (bytes_read < 0) {
			if (errno == EINTR) break;
			perror("  read() error");
			break;
		}

		if (bytes_read == 0) continue;

		read_count++;
		total_bytes += bytes_read;

		printf("[Read #%03d]  %zd byte(s):\n", read_count, bytes_read);
		print_hex_dump(buf, bytes_read);

		for (int i = 0; i < bytes_read; i++) {
			const char *label = decode_midi_status(buf[i]);
			if (label)
				printf("    ^ 0x%02X = %s (ch %d)\n", buf[i], label, buf[i] & 0x0F);
		}
		printf("\n");
	}

	printf("\n[+] Closing device. Total bytes: %d\n", total_bytes);
	close(fd);
	return EXIT_SUCCESS;
}
