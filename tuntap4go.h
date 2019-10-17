#ifndef LIBTUNTAP4GO_LIBRARY_H
#define LIBTUNTAP4GO_LIBRARY_H
#include <memory.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/if_tun.h>
#include <linux/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int get_errno();
char *get_strerror_r(int n);
int vni_alloc(short mode, char *dev);
int vni_configure(const char *dev, int callback(struct ifreq *ifr, struct sockaddr_in *sai, int socket_fd, uint8_t *callback_arguments), uint8_t *callback_arguments);
int set_vni_flags(const char *dev, int flag);
int tun_init(const char *dev);
int set_mtu(const char *dev, int mtu);
int set_vni_address(const char *dev, uint32_t address);
int set_vni_address_by_ascii(const char *dev, const char *address);
int set_tun_destination_address(const char *dev, uint32_t address);
int set_tun_destination_address_by_ascii(const char *dev, const char *address);

#endif //LIBTUNTAP4GO_LIBRARY_H