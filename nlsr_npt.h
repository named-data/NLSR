#ifndef _NLSR_NPT_H_
#define _NLSR_NPT_H_

#define NO_FACE -12345
#define ZERO_FACE 0

struct npt_entry
{
	char *orig_router;
	struct hashtb *name_list;
	struct hashtb *face_list;
};

struct name_list_entry
{
	char *name;
};


int add_npt_entry(char *orig_router, char *name_prefix, int num_face, int *faces, int *route_costs);
int delete_npt_entry_by_router_and_name_prefix(char *orig_router, char *name_prefix);
void print_npt(void);
void delete_orig_router_from_npt(char *orig_router);
void update_npt_with_new_route(void);
void destroy_all_face_by_nlsr(void);
void sort_faces_by_distance(int *faces,int *route_costs,int start,int element);
void update_ccnd_fib_for_orig_router(char *orig_router);
void get_all_faces_for_orig_router_from_npt(char *orig_router, int *faces, int *route_costs, int num_faces);
void destroy_name_list(struct hashtb *name_list);
void destroy_face_list(struct hashtb *face_list);
void destroy_npt(void);

#endif
