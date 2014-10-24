#ifndef __IARCH_MGMT_TYPES_H__
#define __IARCH_MGMT_TYPES_H__


/* begin types */

/* struct im_seed_req */
struct _im_seed_req
{ 
  char * user;
  uint32_t seed_len;
  void * seed;
};
typedef struct _im_seed_req im_seed_req_t;

int im_seed_req_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _im_seed_req *in);
int im_seed_req_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _im_seed_req **out);
void im_seed_req_free(struct _im_seed_req *ptr);
/* struct im_seed_rsp */
struct _im_seed_rsp
{ 
  int32_t sesid;
  char * user;
  uint32_t seed_len;
  void * seed;
};
typedef struct _im_seed_rsp im_seed_rsp_t;

int im_seed_rsp_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _im_seed_rsp *in);
int im_seed_rsp_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _im_seed_rsp **out);
void im_seed_rsp_free(struct _im_seed_rsp *ptr);
/* struct im_auth_req */
struct _im_auth_req
{ 
  char * user;
  char * where;
  uint32_t cred_len;
  void * cred;
  int32_t version;
};
typedef struct _im_auth_req im_auth_req_t;

int im_auth_req_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _im_auth_req *in);
int im_auth_req_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _im_auth_req **out);
void im_auth_req_free(struct _im_auth_req *ptr);
/* struct im_auth_rsp */
struct _im_auth_rsp
{ 
  int64_t uid;
  int32_t version;
};
typedef struct _im_auth_rsp im_auth_rsp_t;

int im_auth_rsp_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _im_auth_rsp *in);
int im_auth_rsp_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _im_auth_rsp **out);
void im_auth_rsp_free(struct _im_auth_rsp *ptr);
/* struct im_user_add_req */
struct _im_user_add_req
{ 
  char * user;
  int32_t type;
  uint32_t cred_len;
  void * cred;
};
typedef struct _im_user_add_req im_user_add_req_t;

int im_user_add_req_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _im_user_add_req *in);
int im_user_add_req_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _im_user_add_req **out);
void im_user_add_req_free(struct _im_user_add_req *ptr);
/* struct im_user_add_rsp */
struct _im_user_add_rsp
{ 
  int64_t uid;
};
typedef struct _im_user_add_rsp im_user_add_rsp_t;

int im_user_add_rsp_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _im_user_add_rsp *in);
int im_user_add_rsp_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _im_user_add_rsp **out);
void im_user_add_rsp_free(struct _im_user_add_rsp *ptr);
/* struct im_user_del_req */
struct _im_user_del_req
{ 
  char * user;
};
typedef struct _im_user_del_req im_user_del_req_t;

int im_user_del_req_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _im_user_del_req *in);
int im_user_del_req_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _im_user_del_req **out);
void im_user_del_req_free(struct _im_user_del_req *ptr);
/* struct im_user_del_rsp */
struct _im_user_del_rsp
{ 
  int64_t uid;
};
typedef struct _im_user_del_rsp im_user_del_rsp_t;

int im_user_del_rsp_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _im_user_del_rsp *in);
int im_user_del_rsp_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _im_user_del_rsp **out);
void im_user_del_rsp_free(struct _im_user_del_rsp *ptr);
/* struct im_user_list_req */
struct _im_user_list_req
{ 
  int32_t type;
};
typedef struct _im_user_list_req im_user_list_req_t;

int im_user_list_req_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _im_user_list_req *in);
int im_user_list_req_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _im_user_list_req **out);
void im_user_list_req_free(struct _im_user_list_req *ptr);
/* struct user */
struct _user
{ 
  int64_t uid;
  int32_t type;
  int32_t status;
};
typedef struct _user user_t;

int user_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _user *in);
int user_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _user **out);
void user_free(struct _user *ptr);
/* struct im_user_list_rsp */
struct _im_user_list_rsp
{ 
  fixarray_t * users;
};
typedef struct _im_user_list_rsp im_user_list_rsp_t;

int im_user_list_rsp_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _im_user_list_rsp *in);
int im_user_list_rsp_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _im_user_list_rsp **out);
void im_user_list_rsp_free(struct _im_user_list_rsp *ptr);
/* struct im_user_update_req */
struct _im_user_update_req
{ 
  int64_t uid;
  char * user;
  int32_t type;
  uint32_t cred_len;
  void * cred;
};
typedef struct _im_user_update_req im_user_update_req_t;

int im_user_update_req_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _im_user_update_req *in);
int im_user_update_req_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _im_user_update_req **out);
void im_user_update_req_free(struct _im_user_update_req *ptr);
/* struct im_user_update_rsp */
struct _im_user_update_rsp
{ 
  int32_t errcode;
};
typedef struct _im_user_update_rsp im_user_update_rsp_t;

int im_user_update_rsp_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _im_user_update_rsp *in);
int im_user_update_rsp_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _im_user_update_rsp **out);
void im_user_update_rsp_free(struct _im_user_update_rsp *ptr);
/* struct im_user_lookup_req */
struct _im_user_lookup_req
{ 
  char * user;
};
typedef struct _im_user_lookup_req im_user_lookup_req_t;

int im_user_lookup_req_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _im_user_lookup_req *in);
int im_user_lookup_req_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _im_user_lookup_req **out);
void im_user_lookup_req_free(struct _im_user_lookup_req *ptr);
/* struct im_user_lookup_rsp */
struct _im_user_lookup_rsp
{ 
  int64_t uid;
  int32_t type;
  int32_t status;
};
typedef struct _im_user_lookup_rsp im_user_lookup_rsp_t;

int im_user_lookup_rsp_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _im_user_lookup_rsp *in);
int im_user_lookup_rsp_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _im_user_lookup_rsp **out);
void im_user_lookup_rsp_free(struct _im_user_lookup_rsp *ptr);
/* struct im_system_status_req */
struct _im_system_status_req
{ 
  int32_t fake;
};
typedef struct _im_system_status_req im_system_status_req_t;

int im_system_status_req_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _im_system_status_req *in);
int im_system_status_req_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _im_system_status_req **out);
void im_system_status_req_free(struct _im_system_status_req *ptr);
/* struct nif */
struct _nif
{ 
  char * name;
  char * mac;
  char * addr;
};
typedef struct _nif nif_t;

int nif_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _nif *in);
int nif_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _nif **out);
void nif_free(struct _nif *ptr);
/* struct im_system_status_rsp */
struct _im_system_status_rsp
{ 
  int32_t capacity_used;
  int32_t capacity_free;
  int32_t battery;
  fixarray_t * nifs;
};
typedef struct _im_system_status_rsp im_system_status_rsp_t;

int im_system_status_rsp_mashal(void *ptr, cmp_reader reader, cmp_writer writer, const struct _im_system_status_rsp *in);
int im_system_status_rsp_unmash(void *ptr, cmp_reader reader, cmp_writer writer, struct _im_system_status_rsp **out);
void im_system_status_rsp_free(struct _im_system_status_rsp *ptr);
/* constants */

#endif /* IARCH_MGMT_TYPES_H */
