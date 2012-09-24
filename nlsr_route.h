#ifndef _NLSR_ROUTE_H_
#define _NLSR_ROUTE_H_

#define EMPTY_PARENT -12345
#define INF_DISTANCE 2147483647

#define NO_NEXT_HOP -12345
#define NO_MAPPING_NUM -1

struct map_entry
{
	char *router;
	int mapping;
};


struct routing_table_entry
{
	char *dest_router;
	//int next_hop_face;
	struct hashtb *face_list;
};

struct face_list_entry
{
	int next_hop_face;
	int route_cost;
};

int route_calculate(struct ccn_schedule *sched, void *clienth, struct ccn_scheduled_event *ev, int flags);
void make_map(void);
void add_map_entry(char *router);
void add_adj_data_to_map(char *orig_router, char *body, int no_link);
void print_map(void);
void assign_mapping_number(void);
void make_adj_matrix(int **adj_matrix,int map_element);
void init_adj_matrix(int **adj_matrix,int map_element);
void print_adj_matrix(int **adj_matrix, int map_element);
int get_mapping_no(char *router);
void calculate_path(int **adj_matrix, long int *parent,long int *dist, long int V, long int S);
void sort_queue_by_distance(long int *Q,long int *dist,long int start,long int element);
int is_not_explored(long int *Q, long int u,long int start, long int element);
void print_path(long int *parent, long int dest);
void print_all_path_from_source(long int *parent,long int source);
void add_rev_map_entry(long int mapping_number, char *router);
void print_rev_map(void);
char * get_router_from_rev_map(long int mapping_number);
int get_no_link_from_adj_matrix(int **adj_matrix,long int V, long int S);
void get_links_from_adj_matrix(int **adj_matrix, long int V ,long int *links, long int *link_costs,long int S);
void adjust_adj_matrix(int **adj_matrix, long int V, long int S, long int link,long int link_cost);

/* Routing Table Relates function */

int get_next_hop(char *dest_router,int *faces, int *route_costs);
int get_number_of_next_hop(char *dest_router);
void add_next_hop_router(char *dest_router);
void add_next_hop_from_lsa_adj_body(char *body, int no_link);
void print_routing_table(void);
void do_old_routing_table_updates();
void clear_old_routing_table();
void update_routing_table_with_new_route(long int *parent,long int *dist, long int source);

long int get_next_hop_from_calculation(long int *parent, long int dest,long int source);
void print_all_next_hop(long int *parent,long int source);
int does_face_exist_for_router(char *dest_router, int face_id);

#endif
