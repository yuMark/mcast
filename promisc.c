#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>

/**
 * Set misc mode for interface
 * \param if_name interface name we will set
 * \param sockfd the socket id we will set
 * */
int set_promisc (char *if_name, int sockfd)
{
    struct ifreq ifr;

    strcpy (ifr.ifr_name, if_name);
    if (0 != ioctl (sockfd, SIOCGIFFLAGS, &ifr))
    {
        printf ("Get interface flag failed\n");
        return -1;
    }

    /* add the misc mode */
    ifr.ifr_flags |= IFF_PROMISC;

    if (0 != ioctl (sockfd, SIOCSIFFLAGS, &ifr))
    {
        printf ("Set interface flag failed\n");
        return -1;
    }
}

int main (int argc, char *argv[])
{
    int sockfd;
    int ret = 0;
    char buffer[1518] = {0};
    char *eth_head = NULL;

   if ((sockfd = socket (PF_PACKET, SOCK_RAW, htons (ETH_P_ALL))) < 0)
   {
       printf ("create socket failed\n");
       return -1;
   }

   if (0 != set_promisc ("eth0", sockfd))
   {
       printf ("Failed to set interface promisc mode\n");
   }

   while (1)
   {
        memset (buffer, 0x0, sizeof (buffer));
        ret = recvfrom (sockfd, buffer, sizeof (buffer), 0, NULL, NULL);
        printf ("recview package length : %d\n", ret);

        eth_head = buffer;
        printf ("PACKAGE START\n");
        /* get source and dectination mac address */
        printf ("dectination mac:%02x-%02x-%02x-%02x-%02x-%02x,"
                  "source mac:%02x-%02x-%02x-%02x-%02x-%02x;\n", eth_head[0],
                  eth_head[1], eth_head[2], eth_head[3], eth_head[4],
                  eth_head[5], eth_head[6], eth_head[7], eth_head[8],
                  eth_head[9], eth_head[10], eth_head[11]);
        printf ("eth_type:%02x%02x\n", eth_head[12], eth_head[13]);

        /* ARP protocol flag */
        if (0x08 == eth_head[12] && 0x06 == eth_head[13])
        {
            printf ("ARP source ip:%d.%d.%d.%d,destination ip:%d.%d.%d.%d;\n",
                      eth_head[28], eth_head[29], eth_head[30], eth_head[31],
                      eth_head[38], eth_head[39], eth_head[40], eth_head[41]);
        }
        /* IPv4 protocol flag */
        else if (0x08 == eth_head[12] && 0x00 == eth_head[13])
        {
            if (0x45 == eth_head[14])
            {
                printf ("IPv4 source ip:%d.%d.%d.%d,destination ip:%d.%d.%d."
                          "%d;\n", eth_head[26], eth_head[27], eth_head[28],
                          eth_head[29], eth_head[30], eth_head[31],
                          eth_head[32], eth_head[33]);
            }
            else
            {
                printf ("p_head:%02x\n", eth_head[14]);
            }
        }
        printf ("PACKAGE END\n");
   }

    return 0;
}