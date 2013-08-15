/* http://www.brokestream.com/udp_redirect.html

  Build: gcc -o udp_redirect udp_redirect.c

  udp_redirect.c
  Version 2013-05-30

  Copyright (C) 2007 Ivan Tikhonov
  Copyright (C) 2013 Stuart Shelton, HP Autonomy

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Ivan Tikhonov, kefeer@brokestream.com

  This source has been modified to support sending data to the destination
  IP address from a different source-ip to the listen-ip, to enable proxying
  on multi-homed hosts

  Stuart Shelton, stuart.shelton@hp.com

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

int bindsocket( char* ip, int port );
int main( int argc, char* argv[] );

int bindsocket( char* ip, int port ) {
	int fd = socket( PF_INET, SOCK_DGRAM, IPPROTO_IP );
	struct sockaddr_in addr;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr( ip );
	addr.sin_port = htons( port );
	if( bind( fd, (struct sockaddr*)&addr, sizeof( addr ) ) == -1 ) {
		fprintf( stderr, "Cannot bind address (%s:%d)\n", ip, port );
		exit( 1 );
	}
	return fd;
}

int main( int argc, char* argv[] ) {
	if( 3 != argc && 5 != argc && 6 != argc ) {
		fprintf( stderr, "Usage: %s <listen-ip> <listen-port> [[source-ip] <destination-ip> <destination-port>]\n", argv[0] );
		exit( 1 );
	}

	int i = 1;
	char* inip = argv[ i++ ];	/* 1 */
	char* inpt = argv[ i++ ];	/* 2 */
	char* srcip;
	char* dstip;
	char* dstpt;
	if( 6 == argc )
		srcip = argv[ i++ ];	/* 3 */
	if( 3 != argc ) {
		dstip = argv[ i++ ];	/* 3 or 4 */
		dstpt = argv[ i++ ];	/* 4 or 5 */
	}
	
	int listen = bindsocket( inip, atoi( inpt ) );
	int source = 0;
	int output = 0;
	if( 6 != argc ) {
		output = listen;
	} else {
		source = bindsocket( srcip, atoi( inpt ) );
		output = source;
	}

	struct sockaddr_in dst;
	dst.sin_family = AF_INET;
	dst.sin_addr.s_addr = inet_addr( dstip );
	dst.sin_port = htons( atoi( dstpt ) );

	struct sockaddr_in src;
	struct sockaddr_in ret;
	ret.sin_addr.s_addr = 0;
	while( 1 ) {
		char buffer[65535];
		int size = sizeof( src );
		int length = recvfrom( listen, buffer, sizeof( buffer ), 0, (struct sockaddr*)&src, &size );
		if( length <= 0 )
			continue;

		if( argc == 3 ) {
			/* echo, without tracking return packets */
			sendto( listen, buffer, length, 0, (sth, 0, (struct sockaddr*)&src, size );
		} else if( ( src.sin_addr.s_addr == dst.sin_addr.s_addr ) && ( src.sin_port == dst.sin_port ) ) {
			/* If we receive a return packet back from our destination ... */
			if( ret.sin_addr.s_addr )
				/* ... and we've previously remembered having sent packets to this location,
				   then return them to the original sender */
				sendto( output, buffer, length, 0, (struct sockaddr*)&ret, sizeof( ret ) );
		} else {
			sendto( output, buffer, length, 0, (struct sockaddr*)&dst, sizeof( dst ) );
			/* Remeber original sender to direct return packets toward */
			ret = src;
		}
	}
}
