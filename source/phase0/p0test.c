#include <stdio.h>
#include <stdlib.h>
#include "clist.h"
//#include "clist_stampe_debug.h"
#define LEN 256

struct clist myclist=CLIST_INIT;

struct clistofint {
	int value;
	struct clist list;
};

char buf[LEN];

int main()
{
	struct clistofint *elem,*scan;
	void *tmp=NULL;
	int value;
	while (fgets(buf,LEN,stdin) != NULL) {
		switch (*buf) {
			case 'l':
				clist_foreach(scan, &myclist, list, tmp) 
				printf("== %p %d\n",scan,scan->value);
				break;
			case 'a':
				elem=malloc(sizeof(struct clistofint));
				elem->value=strtol(buf+1,NULL,0);
				clist_enqueue(elem, &myclist, list);
				break;
			case 'h':
				elem=clist_head(elem, myclist, list);
				printf("head-> %p %d\n",elem,elem->value); 
				break;
			case 't':
				elem=clist_tail(elem, myclist, list);
				printf("tail-> %p %d\n",elem,elem->value); 
				break;
			case 'e':
				printf("empty? -> %s\n",clist_empty(myclist)?"true":"false");
				break;
			case 'p':
				elem=malloc(sizeof(struct clistofint));
				elem->value=strtol(buf+1,NULL,0);
				clist_push(elem, &myclist, list);
				break;
			case 'd':
				elem=clist_head(elem, myclist, list);
				if(elem) {
					printf("dequeue/pop-> %p %d\n",elem,elem->value);
					clist_dequeue(&myclist);
					free(elem);
				}
				break;
			case 'x':
				elem=(struct clistofint *)strtol(buf+1,NULL,0);
				if (clist_delete(elem, &myclist, list) == 0) {
					printf("delete %p->done\n",elem);
					free(elem);
				} else
					printf("delete %p->err\n",elem);
				
				break;
				
			case '-':
				value=strtol(buf+1,NULL,0);
				clist_foreach(scan, &myclist, list, tmp) {
					if (value==scan->value)
					{					
						clist_foreach_delete(scan, &myclist, list, tmp);
						printf("delete %p->done\n",scan);
						free(scan);
						break;
					}
				}
				break;
			case '<':
				elem=malloc(sizeof(struct clistofint));
				elem->value=strtol(buf+1,NULL,0);
				clist_foreach(scan, &myclist, list, tmp) {
					if (elem->value < scan->value) {
						clist_foreach_add(elem, scan, &myclist, list, tmp);
						break;
					}
				}
				if (clist_foreach_all(scan, &myclist, list,tmp)){ 
					clist_enqueue(elem, &myclist, list);
				}
				break;
		}
	}
	return 0;
}
