/* 
 * logoinsert - insert an image into a dreamcast ip.bin
 *
 * use pngtomr to produce images to insert
 * images must be less than 8192 bytes to use in a "normal" ip.bin
 *
 * adk / napalm 2001 
 *
 * andrewk@napalm-x.com
 *
 * http://www.napalm-x.com
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdio.h>

int main(int argc, char *argv[])
{
    FILE *mr, *ip;
    int i;
    char *data;
    int length;

    printf("logoinsert 0.1 by adk/napalm\n");

    if (argc != 3) {
	printf("%s <image.mr> <ip.bin>\n",argv[0]);
	exit(0);
    }

    mr = fopen(argv[1], "rb");
    ip = fopen(argv[2], "rb+");

    if (!mr) {
	perror(argv[1]);
	exit(0);
    }

    if (!ip) {
	perror(argv[2]);
	exit(0);
    }

    fseek(ip, 0x3820, SEEK_SET);

    fseek(mr, 0, SEEK_END);
    length = ftell(mr);
    fseek(mr, 0, SEEK_SET);

    if (length > 8192)
	printf("Warning: this image is larger than 8192 bytes and will corrupt a normal ip.bin, inserting anyway!\n");

    data = (char *)malloc(length);

    fread(data, length, 1, mr);

    fwrite(data, length, 1, ip);

    fclose(ip);
    fclose(mr);
}
	
    

