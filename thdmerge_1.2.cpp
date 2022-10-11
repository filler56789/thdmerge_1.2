#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

int next_ac3(unsigned char *buf, FILE *ac3_fd, FILE *out_fd)
{
	int fsize;

	if (feof(ac3_fd))
		return -1;
	// Determine the frame size to write.
	if (fread(buf, 1, 8, ac3_fd) != 8)
		return -1;
	switch (buf[4] & 0b00111110)
	{
	case 0b001000:
		fsize = 2 * 128;
		break;
	case 0b001010:
		fsize = 2 * 160;
		break;
	case 0b001100:
		fsize = 2 * 192;
		break;
	case 0b001110:
		fsize = 2 * 224;
		break;
	case 0b010000:
		fsize = 2 * 256;
		break;
	case 0b010010:
		fsize = 2 * 320;
		break;
	case 0b010100:
		fsize = 2 * 384;
		break;
	case 0b010110:
		fsize = 2 * 448;
		break;
	case 0b011000:
		fsize = 2 * 512;
		break;
	case 0b011010:
		fsize = 2 * 640;
		break;
	case 0b011100:
		fsize = 2 * 768;
		break;
	case 0b011110:
		fsize = 2 * 896;
		break;
	case 0b100000:
		fsize = 2 * 1024;
		break;
	case 0b100010:
		fsize = 2 * 1152;
		break;
	case 0b100100:
		fsize = 2 * 1280;
		break;
	default:
		return -1;
	}
	// Write the frame.
	if (_fseeki64(ac3_fd, -8, SEEK_CUR) == -1L)
		return -1;
	if (fread(buf, 1, fsize, ac3_fd) != fsize)
		return -1;
	if (fwrite(buf, 1, fsize, out_fd) != fsize)
	{
		printf("Could not write AC3 frame\n");
		exit(1);
	}
	return 0;
}

int next_truehd(unsigned char *buf, FILE *truehd_fd, FILE *out_fd)
{
	int length;

	if (feof(truehd_fd))
		return -1;
	// Determine the frame size to write.
	if (fread(buf, 1, 8, truehd_fd) != 8)
		return -1;
	length = 2 * (((buf[0] << 8) | (buf[1])) & 0xfff);
	// Write the frame.
	if (_fseeki64(truehd_fd, -8, SEEK_CUR) == -1L)
		return -1;
	if (fread(buf, 1, length, truehd_fd) != length)
		return -1;
	if (fwrite(buf, 1, length, out_fd) != length)
	{
		printf("Could not write TrueHD frame\n");
		exit(1);
	}
	return 0;
}

int main(int argc, char** argv)
{
	unsigned char *buf;
	double truehd_time, ac3_time;
	FILE *truehd_fd, *ac3_fd, *out_fd;
	int ac3_eof, truehd_eof;
	int truncate;

	if (argc == 1 || argc > 5)
	{
		printf("thdmerge 1.2 by Donald A. Graft\n");
		printf("Usage: thdmerge input.thd input.ac3 output.thd+ac3 [-t]\n");
		printf("-t: truncate longer stream to length of shorter stream\n");
		printf("NOTICE:\n");
		printf("input.ac3 can be a silent and mono stream;\n");
		printf("the ac3 bitrate must be equal to or greater than 64 kbps.\n");
		exit(0);
	}
	if (argc == 5)
		truncate = 1;
	else
		truncate = 0;

	buf = (unsigned char*)malloc(1024 * 1024);
	if (buf == NULL)
	{
		printf("thdmerge: could not allocate memory\n");
		exit(1);
	}
	truehd_fd = fopen(argv[1], "rb");
	if (truehd_fd == NULL)
	{
		printf("Cannot open TrueHD file\n");
		exit(0);
	}
	ac3_fd = fopen(argv[2], "rb");
	if (ac3_fd == NULL)
	{
		printf("Cannot open AC3 file\n");
		exit(0);
	}
	_unlink(argv[3]);
	out_fd = fopen(argv[3], "wb");
	if (out_fd == NULL)
	{
		printf("Cannot open output file\n");
		exit(0);
	}

	ac3_time = truehd_time = 0;
	ac3_eof = truehd_eof = 0;
	// Interleave keeping sync.
	// Keep going until both streams hit EOF.
	while (1)
	{
		if (ac3_eof && truehd_eof)
			break;
		if (!ac3_eof && (truehd_eof || ac3_time <= truehd_time))
		{
			if (next_ac3(buf, ac3_fd, out_fd) == -1)
			{
				ac3_eof = 1;
				if (truncate)
					break;
			}
			else
				ac3_time += 0.032f;
		}
		else
		{
			if (next_truehd(buf, truehd_fd, out_fd) == -1)
			{
				truehd_eof = 1;
				if (truncate)
					break;
			}
			else
				truehd_time += 1.0f / 1200;
		}
	}

	free(buf);
	fclose(truehd_fd);
	fclose(ac3_fd);
	fclose(out_fd);
	return 0;
}
