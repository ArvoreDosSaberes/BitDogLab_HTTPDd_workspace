/**
 * @file    lwipopts.h
 * @brief   Configurações do lwIP para BitDogLab HTTP Server
 * 
 * @project BitDogLab_HTTPDd_workspace
 * @url     https://github.com/ArvoreDosSaberes/BitDogLab_HTTPDd_workspace
 * 
 * @author  Carlos Delfino
 * @email   consultoria@carlosdelfino.eti.br
 * @website https://carlosdelfino.eti.br
 * @github  https://github.com/CarlosDelfino
 * 
 * @license CC BY 4.0 - https://creativecommons.org/licenses/by/4.0/
 * 
 * @see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html
 */

#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// ===== Sistema =====
#ifndef NO_SYS
#define NO_SYS                      1
#endif
#ifndef LWIP_SOCKET
#define LWIP_SOCKET                 0
#endif

// ===== Alocação de Memória =====
#if PICO_CYW43_ARCH_POLL
#define MEM_LIBC_MALLOC             1
#else
// MEM_LIBC_MALLOC is incompatible with non polling versions
#define MEM_LIBC_MALLOC             0
#endif
#define MEM_ALIGNMENT               4

// Calculado para suportar:
// - 12 conexões TCP simultâneas (~250 bytes cada = 3000 bytes)
// - Processamento SSI com 14 tags (~500 bytes por requisição)
// - Buffer POST de 512 bytes para matrix.cgi
// - Overhead de alocação dinâmica (~30%)
// Total: ~14820 bytes, arredondado para 16000
#define MEM_SIZE                    16000

// ===== TCP/IP Stack =====
#define TCP_MSS                     1460
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))
#define LWIP_TCP_KEEPALIVE          1

// ===== Memory Pools =====
// TCP segments na fila de saída
// Calculado: (TCP_SND_BUF * 4 / TCP_MSS) para múltiplas conexões
#define MEMP_NUM_TCP_SEG            40
#define MEMP_NUM_TCP_PCB            12
#define MEMP_NUM_ARP_QUEUE          10
#define MEMP_NUM_SYS_TIMEOUT        (LWIP_NUM_SYS_TIMEOUT_INTERNAL + 3 + 5)

// Pool de pbufs para recepção/transmissão
// Maior arquivo: index.shtml (11146 bytes) = 8 pbufs de 1460 bytes
// Com múltiplas conexões: 8 × 3 conexões ativas = 24 mínimo
// Aumentado para 32 para margem de segurança
#define PBUF_POOL_SIZE              32

// ===== Protocolos =====
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define LWIP_IPV4                   1
#define LWIP_TCP                    1
#define LWIP_UDP                    1
#define LWIP_DNS                    1
#define LWIP_DHCP                   1
#define LWIP_NETCONN                0
#define LWIP_CHKSUM_ALGORITHM       3

// ===== Network Interface =====
#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1
#define LWIP_NETIF_HOSTNAME         1
#define LWIP_NETIF_TX_SINGLE_PBUF   1

// ===== DHCP =====
#define DHCP_DOES_ARP_CHECK         0
#define LWIP_DHCP_DOES_ACD_CHECK    0

// ===== mDNS =====
#define LWIP_MDNS_RESPONDER         1
#define LWIP_IGMP                   1
#define LWIP_NUM_NETIF_CLIENT_DATA  1
#define MDNS_RESP_USENETIF_EXTCALLBACK 1

// ===== HTTP Server Features =====
#define LWIP_HTTPD_CGI              1
#define LWIP_HTTPD_SSI              1
#define LWIP_HTTPD_SSI_MULTIPART    1
#define LWIP_HTTPD_SUPPORT_POST     1
#define LWIP_HTTPD_SSI_INCLUDE_TAG  0
#define HTTPD_FSDATA_FILE           "pico_fsdata.inc"

// ===== HTTP Server Memory Tuning =====
// Tamanho máximo de inserção SSI (para tags grandes como "table")
#define LWIP_HTTPD_MAX_TAG_INSERT_LEN       256
// Tamanho máximo de nome de tag SSI (maior tag: "ledstate" = 8 chars)
#define LWIP_HTTPD_MAX_TAG_NAME_LEN         16
// Número máximo de parâmetros CGI por requisição
#define LWIP_HTTPD_MAX_CGI_PARAMETERS       10
// POST request buffer - adequado para matrix.cgi (25 LEDs × 7 chars = 175 bytes)
#define LWIP_HTTPD_POST_MAX_PAYLOAD_LEN     512
// Buffer para URI de resposta POST
#define LWIP_HTTPD_POST_MAX_RESPONSE_URI_LEN 64
#define HTTPD_PRECALCULATED_CHECKSUM        0

// ===== Estatísticas =====
#define MEM_STATS                   0
#define SYS_STATS                   0
#define MEMP_STATS                  0
#define LINK_STATS                  0

#ifndef NDEBUG
#define LWIP_DEBUG                  1
#define LWIP_STATS                  1
#define LWIP_STATS_DISPLAY          1
#endif

// ===== Debug Flags =====
#define ETHARP_DEBUG                LWIP_DBG_OFF
#define NETIF_DEBUG                 LWIP_DBG_OFF
#define PBUF_DEBUG                  LWIP_DBG_OFF
#define API_LIB_DEBUG               LWIP_DBG_OFF
#define API_MSG_DEBUG               LWIP_DBG_OFF
#define SOCKETS_DEBUG               LWIP_DBG_OFF
#define ICMP_DEBUG                  LWIP_DBG_OFF
#define INET_DEBUG                  LWIP_DBG_OFF
#define IP_DEBUG                    LWIP_DBG_OFF
#define IP_REASS_DEBUG              LWIP_DBG_OFF
#define RAW_DEBUG                   LWIP_DBG_OFF
#define MEM_DEBUG                   LWIP_DBG_OFF
#define MEMP_DEBUG                  LWIP_DBG_OFF
#define SYS_DEBUG                   LWIP_DBG_OFF
#define TCP_DEBUG                   LWIP_DBG_OFF
#define TCP_INPUT_DEBUG             LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG            LWIP_DBG_OFF
#define TCP_RTO_DEBUG               LWIP_DBG_OFF
#define TCP_CWND_DEBUG              LWIP_DBG_OFF
#define TCP_WND_DEBUG               LWIP_DBG_OFF
#define TCP_FR_DEBUG                LWIP_DBG_OFF
#define TCP_QLEN_DEBUG              LWIP_DBG_OFF
#define TCP_RST_DEBUG               LWIP_DBG_OFF
#define UDP_DEBUG                   LWIP_DBG_OFF
#define TCPIP_DEBUG                 LWIP_DBG_OFF
#define PPP_DEBUG                   LWIP_DBG_OFF
#define SLIP_DEBUG                  LWIP_DBG_OFF
#define DHCP_DEBUG                  LWIP_DBG_OFF

#endif /* _LWIPOPTS_H */
