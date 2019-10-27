#include "tuntap4go.h"

static int
tun_set_flags_callback(struct ifreq *ifr, struct sockaddr_in *sai, int socket_fd, uint8_t *callback_arguments);

static int set_mtu_callback(struct ifreq *ifr, struct sockaddr_in *sai, int socket_fd, uint8_t *callback_arguments);

static int
set_vni_address_callback(struct ifreq *ifr, struct sockaddr_in *sai, int socket_fd, uint8_t *callback_arguments);

static int set_tun_destination_address_callback(struct ifreq *ifr, struct sockaddr_in *sai, int socket_fd,
                                                uint8_t *callback_arguments);

int get_errno() {
    return errno;
}

char *get_strerror_r(int n) {
    static const size_t buffer_size = 256;
    char *buffer = malloc(buffer_size * sizeof(char));
    if (buffer == NULL) {
        return "malloc(): allocated memory for strerror_r failed";
    }
    strerror_r(n, buffer, buffer_size);
    return buffer;
}

int vni_alloc(short mode, char *dev) {
    struct ifreq ifr;
    int fd, err;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
        return fd;

    memset(&ifr, 0, sizeof(ifr));

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *
     *        IFF_NO_PI - Do not provide packet information
     */

    if (mode == 1) {
        ifr.ifr_flags = IFF_TUN;
    } else {
        ifr.ifr_flags = IFF_TAP;
    }

    if (*dev)
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
        close(fd);
        return err;
    }
    strcpy(dev, ifr.ifr_name);
    return fd;
}

int vni_configure(const char *dev,
                  int callback(struct ifreq *ifr, struct sockaddr_in *sai, int socket_fd, uint8_t *callback_arguments),
                  uint8_t *callback_arguments) {
    struct ifreq ifr;
    struct sockaddr_in sai;
    memset(&ifr, 0, sizeof(ifr));
    memset(&sai, 0, sizeof(struct sockaddr));

    int socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0) {
        return socket_fd;
    }
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    sai.sin_family = AF_INET;
    sai.sin_port = 0;

    return callback(&ifr, &sai, socket_fd, callback_arguments);
}

int set_vni_flags(const char *dev, int flag) {
    return vni_configure(dev, tun_set_flags_callback, (uint8_t *) flag);
}

int tun_init(const char *dev) {
    return set_vni_flags(dev, (uint8_t *) (IFF_UP | IFF_POINTOPOINT | IFF_RUNNING | IFF_NOARP | IFF_MULTICAST));
}

int set_mtu(const char *dev, int mtu) {
    return vni_configure(dev, set_mtu_callback, (uint8_t *) mtu);
}

int set_vni_address(const char *dev, uint32_t address, uint32_t netmask) {
    uint32_t address_and_netmask[2];
    address_and_netmask[0] = address;
    address_and_netmask[1] = netmask;

    return vni_configure(dev, set_vni_address_callback, (uint8_t *) address_and_netmask);
}

int set_vni_address_by_ascii(const char *dev, const char *address, const char *netmask) {
    struct in_addr ip, mask;
    if (!inet_aton(address, &ip)) {
        return -1;
    }
    if (!inet_aton(netmask, &mask)) {
        return -1;
    }

    return set_vni_address(dev, ip.s_addr, mask.s_addr);
}

int set_tun_destination_address(const char *dev, uint32_t address) {
    return vni_configure(dev, set_tun_destination_address_callback, (uint8_t *) address);
}

int set_tun_destination_address_by_ascii(const char *dev, const char *address) {
    struct in_addr in_val;
    if (!inet_aton(address, &in_val)) {
        return -1;
    }
    return set_tun_destination_address(dev, in_val.s_addr);
}

static int
tun_set_flags_callback(struct ifreq *ifr, struct sockaddr_in *sai, int socket_fd, uint8_t *callback_arguments) {
    int flag = (int) callback_arguments;
    ifr->ifr_flags = flag;
    return ioctl(socket_fd, SIOCSIFFLAGS, ifr);
}

static int set_mtu_callback(struct ifreq *ifr, struct sockaddr_in *sai, int socket_fd, uint8_t *callback_arguments) {
    ifr->ifr_mtu = (int) callback_arguments;
    return ioctl(socket_fd, SIOCSIFMTU, ifr);
}

static int
set_vni_address_callback(struct ifreq *ifr, struct sockaddr_in *sai, int socket_fd, uint8_t *callback_arguments) {
    uint32_t *address_and_netmask = (uint32_t *) callback_arguments;
    memcpy(&ifr->ifr_addr, sai, sizeof(struct sockaddr));

    sai = (struct sockaddr_in *) &ifr->ifr_addr;

    sai->sin_addr.s_addr = address_and_netmask[0];
    if (ioctl(socket_fd, SIOCSIFADDR, ifr) < 0) {
        return -1;
    }

    if (address_and_netmask[1]) {
        sai->sin_addr.s_addr = address_and_netmask[1];
        return ioctl(socket_fd, SIOCSIFNETMASK, ifr);
    }
    return 0;
}

static int set_tun_destination_address_callback(struct ifreq *ifr, struct sockaddr_in *sai, int socket_fd,
                                                uint8_t *callback_arguments) {
    uint32_t address = (uint32_t) callback_arguments;
    sai->sin_addr.s_addr = address;
    memcpy(&ifr->ifr_addr, sai, sizeof(struct sockaddr));
    return ioctl(socket_fd, SIOCSIFDSTADDR, ifr);
}