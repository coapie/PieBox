
#ifndef __PROTO_H__
#define __PROTO_H__

#include <stdint.h>

#define PROTO_ALIGN(_ap,_as)   ((_ap)+(_as)-1)&(~((_as)-1))

typedef enum{
    PROTO_CMD_RESET = 0,

    // session setup/close
    PROTO_CMD_SEED,
    PROTO_CMD_SETUP,
    PROTO_CMD_CLOSE,

    PROTO_CMD_USER_ADD,
    PROTO_CMD_USER_DEL,
    PROTO_CMD_USER_LIST,
    PROTO_CMD_USER_UPDATE,
    PROTO_CMD_USER_LOOKUP,

    PROTO_CMD_SYSTEM_STATUS,

    // lookup
    PROTO_CMD_LOOKUP_CREATE,
    PROTO_CMD_LOOKUP_DESTROY,
    PROTO_CMD_LOOKUP_NEXT,

    // dset operations
    PROTO_CMD_FSTAT,
    PROTO_CMD_FOPEN,
    PROTO_CMD_FCLOSE,
    PROTO_CMD_FREAD,
    PROTO_CMD_FWRITE,
    PROTO_CMD_FTRUNCATE,

    // dset operations
    PROTO_CMD_FMOVETO,
    PROTO_CMD_FCREATE,
    PROTO_CMD_FDELETE,
    
}proto_type_t;

typedef enum {
    PROTO_RSP_ECODE_SUCCESS = 0,
    PROTO_RSP_ECODE_INVALID,
    PROTO_RSP_ECODE_NOENTRY,
    PROTO_RSP_ECODE_EXISTS,
}proto_rsp_ecode_t;

typedef enum{
    RFILE_TYPE_VIDEO = 0,
    RFILE_TYPE_AUDIO,
    RFILE_TYPE_PHOTO,
    RFILE_TYPE_DOCS,
    RFILE_TYPE_END
}rfile_type_t;

#define RFOLDER_TYPE_BIT  0x00008000

typedef struct _proto_header{
    uint16_t    size;
    uint8_t     cmdcode;    // command / response
    uint8_t     errcode;    // error code in response

    uint32_t    sesid;      // session id
    uint32_t    seqid;      // sequence id
    uint32_t    cred;       // cred for safety

}__attribute__((packed)) proto_header_t;

#define PROTO_HEADER_LEN sizeof(proto_header_t)

//
// misc functions
//
#if 0
static inline uint64_t ntoh64(const uint64_t input)
{
    uint64_t rval;
    uint8_t *data = (uint8_t *)&rval;

    data[0] = input >> 56;
    data[1] = input >> 48;
    data[2] = input >> 40;
    data[3] = input >> 32;
    data[4] = input >> 24;
    data[5] = input >> 16;
    data[6] = input >> 8;
    data[7] = input >> 0;

    return rval;
}

static inline uint64_t hton64(const uint64_t input)
{
    return (ntoh64(input));
}
#endif
int proto_dump(proto_header_t *proto, int len);

#endif

