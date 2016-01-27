
/*
 * pcapreader.c 
 *
 * Test for qpCore module.
 */


#include <qpCore.h>


#define  MAX_SNAPLEN    65535


typedef struct {
    qp_uint32_t    magic;
    qp_ushort_t    ver_major;
    qp_ushort_t    ver_minor;
    qp_uint32_t    tz;
    qp_uint32_t    sigfigs;
    qp_uint32_t    snaplen;
    qp_uint32_t    linktype;
}pcap_file_header;


typedef struct {
    qp_uint32_t    tv_sec;
    qp_uint32_t    tv_usec;
    qp_uint32_t    caplen;
    qp_uint32_t    len;
}pcap_pkthdr;

typedef int (*pcap_handler)(qp_uchar_t* user, pcap_pkthdr* hdr, qp_uchar_t* data);


typedef struct {
    pcap_file_header  file_hdr;
    FILE*             file;
    pcap_pkthdr       pkt_hdr;
    size_t            pkt_num;
    pcap_handler      pkt_process;
    qp_stack_frame_result_t  pkt_result;
    struct stat       stat;
    qp_uchar_t        pkt_data[MAX_SNAPLEN];
    bool              run;
}pcap;


int 
pcap_process(qp_uchar_t* user, pcap_pkthdr* hdr, qp_uchar_t* data);
        
int
pcap_open(const char* path, pcap* cap);

int
pcap_close(pcap* cap);

int 
pcap_read(pcap* cap);


int        
pcap_process(qp_uchar_t* user, pcap_pkthdr* hdr, qp_uchar_t* data)
{
    pcap* cap = (pcap*)user;
    
    qp_stack_parse(data, hdr->caplen, 1, &cap->pkt_result, \
        QP_STACK_LEVEL_TRANSMIT);
    
    if (QP_STACK_PROTO_TCP != cap->pkt_result.l4_type) {
        return QP_SUCCESS;
    }
    
    QP_LOGOUT_LOG("[%lu]"
        "\n\tsrc mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x"
        "\n\tdst mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x"
        "\n\tsrc ip: %u.%u.%u.%u"
        "\n\tdst ip: %u.%u.%u.%u"
        "\n\tsrc port: %u"
        "\n\tdst port: %u",
        cap->pkt_num,
        cap->pkt_result.smac->sit1, cap->pkt_result.smac->sit2,
        cap->pkt_result.smac->sit3, cap->pkt_result.smac->sit4,
        cap->pkt_result.smac->sit5, cap->pkt_result.smac->sit6,
        cap->pkt_result.dmac->sit1, cap->pkt_result.dmac->sit2,
        cap->pkt_result.dmac->sit3, cap->pkt_result.dmac->sit4,
        cap->pkt_result.dmac->sit5, cap->pkt_result.dmac->sit6,
        cap->pkt_result.src.ipv4->to_byte.sit1, 
        cap->pkt_result.src.ipv4->to_byte.sit2,
        cap->pkt_result.src.ipv4->to_byte.sit3,
        cap->pkt_result.src.ipv4->to_byte.sit4,
        cap->pkt_result.dst.ipv4->to_byte.sit1,
        cap->pkt_result.dst.ipv4->to_byte.sit2,
        cap->pkt_result.dst.ipv4->to_byte.sit3,
        cap->pkt_result.dst.ipv4->to_byte.sit4,
        ntohs(*cap->pkt_result.sport),
        ntohs(*cap->pkt_result.dport));
    
    return QP_SUCCESS;
}


int
pcap_open(const char* path, pcap* cap) 
{
    if (NULL == path) {
        return QP_ERROR;
    }
    
    cap->file = fopen(path, "r");
    
    if (!cap->file) {
        return QP_ERROR;
    }
    
    cap->pkt_num = 0;
    
    if (sizeof(pcap_file_header) != \
        fread(&cap->file_hdr, 1, sizeof(pcap_file_header), cap->file))
    {
        pcap_close(cap);
        QP_LOGOUT_LOG("[pcap_open] %s open fail.", path);
        return QP_ERROR;
    }
    
    qp_uint32_t magic = 0xa1b2c3d4;
    
    if (cap->file_hdr.snaplen > MAX_SNAPLEN 
       || memcmp(&cap->file_hdr.magic, &magic, sizeof(qp_uint32_t)))
    {
        pcap_close(cap);
        QP_LOGOUT_LOG("[pcap_open] File header error.");
        return QP_ERROR;
    }
    
    fstat(fileno(cap->file), &cap->stat);
    
    QP_LOGOUT_LOG("[pcap_open]File info : "
            "\n\tpcap magic: %.4x"
            "\n\tpcap size: %luKB"
            "\n\tpcap version: %d.%d"
            "\n\tpcap snaplen: %d",
            cap->file_hdr.magic,
            cap->stat.st_size/1024,
            cap->file_hdr.ver_major, cap->file_hdr.ver_minor,
            cap->file_hdr.snaplen);
    return QP_SUCCESS;
}


int
pcap_close(pcap* cap)
{
    if (cap->file) {
        fclose(cap->file);
        cap->file = NULL;
    }
    
    return QP_SUCCESS;
}


int 
pcap_read(pcap* cap)
{
    if (cap->file) {
        size_t offset = 0;
        size_t all = 0;
        cap->run = true;
        
        while (cap->run) {
            all = 0;
            
            while (cap->run) {
                offset = fread((qp_uchar_t*)&cap->pkt_hdr + all, 1, \
                    sizeof(pcap_pkthdr) - all, cap->file);
                all += offset;
                
                if (all != sizeof(pcap_pkthdr)) {
                    
                    if (feof(cap->file)) {
                        return QP_SUCCESS;;
                    }
                }
                
                break;
            }
            
            if (cap->pkt_hdr.caplen > cap->file_hdr.snaplen) {
                QP_LOGOUT_LOG("[pcap_read] Packet caplen error.");
                return QP_ERROR;
            }
            
            all = 0;
            
            while (cap->run) {
                offset = fread((qp_uchar_t*)&cap->pkt_data + all, 1, \
                    cap->pkt_hdr.caplen - all, cap->file);
                all += offset;
                
                if (all != cap->pkt_hdr.caplen) {
                    
                    if (feof(cap->file)) {
                        return QP_SUCCESS;;
                    }
                }
                
                break;
            }
            
            if (cap->pkt_process) {
                cap->pkt_process((qp_uchar_t*)cap, &cap->pkt_hdr, cap->pkt_data);
            }
            
            cap->pkt_num++;
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}


void
print_help()
{
    fprintf(stderr, "Usage: pcapreader -i <file>.");
}


int 
main(int argc, char* argv[])
{
    printf("%lu", sizeof(struct aiocb));
    return 0;
    pcap cap;
    
    
    
    if (QP_ERROR == pcap_open("test.pcap", &cap)) {
        return QP_ERROR;
    }
    
    cap.pkt_process = pcap_process;

    pcap_read(&cap);
    
    pcap_close(&cap);
    
    return 0;
}
