#include "bin.h"

//binary heap implementation by github.com/aatishnn
//taken from https://gist.github.com/aatishnn/8265656
//modifications by Raef Coles for genearality
//any questions to rccoles@github.com or /u/rccoles
// Array representation according to Wikipedia article but starts from 0 index.

int *heap, size, count;
int initial_size = 4;

void heap_init(bin_heap *h)
{
	h->count = 0;
	h->size = initial_size;
	h->heaparr = (heap_node*) malloc(sizeof(heap_node) * 4);
	if(!h->heaparr) {
		exit(-1);
	}

}

void max_heapify(heap_node *data, int loc, int count) {
	int left, right, largest;
  heap_node temp;
	left = 2*(loc) + 1;
	right = left + 1;
	largest = loc;
	

	if (left <= count - 1 && data[left].priority > data[largest].priority) {
		largest = left;
	} 
	if (right <= count - 1 && data[right].priority > data[largest].priority) {
		largest = right;
	} 
	
	if(largest != loc) {
		temp = data[loc];
		data[loc] = data[largest];
		data[largest] = temp;
		max_heapify(data, largest, count);
	}

}

void heap_push(bin_heap *h, void *value, int priority)
{
  heap_node node;
  node.priority = priority;
  node.data = value;

	int index, parent;
 
	// Resize the heap if it is too small to hold all the data
	if (h->count == h->size)
	{
		h->size += 1;
		h->heaparr = realloc(h->heaparr, sizeof(heap_node) * h->size);
		if (!h->heaparr) exit(-1); // Exit if the memory allocation fails
	}
 	
 	index = h->count++; // First insert at last of array

 	// Find out where to put the element and put it
	for(;index; index = parent)
	{
		parent = (index - 1) / 2;
		if (h->heaparr[parent].priority >= priority) break;
		h->heaparr[index] = h->heaparr[parent];
	}
	h->heaparr[index] = node;
}

heap_node* heap_find(bin_heap *h, void *value){
	for(int I = 0; I < h->count; I++){
		if (h->heaparr[I].data == value){
			return &(h->heaparr[I]);
		}
	}
	return NULL;
}

int heap_find_index(bin_heap *h, void *value){
	for(int I = 0; I < h->count; I++){
		if (h->heaparr[I].data == value){
			return I;
		}
	}
	return -1;
}

void heap_remove(bin_heap *h, void *value){
	int I = heap_find_index(h, value);
	memcpy(&(h->heaparr[I]), &(h->heaparr[I + 1]), sizeof(heap_node) * (h->count - I - 1));
	h->count--;
}

void heap_display(bin_heap *h) {
	int i;
	for(i=0; i<h->count; ++i) {
		printf("|%d:0x%08x|", h->heaparr[i].priority, h->heaparr[i].data);
	}
	printf("\n");
}

void* heap_pop(bin_heap *h)
{
	heap_node removed;
	heap_node temp = h->heaparr[--h->count];
 	
	
	if ((h->count <= (h->size + 2)) && (h->size > initial_size))
	{
		h->size -= 1;
		h->heaparr = realloc(h->heaparr, sizeof(heap_node) * h->size);
		if (!h->heaparr) exit(-1); // Exit if the memory allocation fails
	}
 	removed = h->heaparr[0];
 	h->heaparr[0] = temp;
 	max_heapify(h->heaparr, 0, h->count);
 	return removed.data;
}


int emptyPQ(bin_heap *pq) {
	int i;
	while(pq->count != 0) {
		printf("<<%d", heap_pop(pq));
	}
}

// int main() {
// 	bin_heap h;
// 	heap_init(&h);
// 	heap_push(&h,1);
// 	heap_push(&h,5);
// 	heap_push(&h,3);
// 	heap_push(&h,7);
// 	heap_push(&h,9);
// 	heap_push(&h,8);
// 	heap_display(&h);
// 	heap_display(&h);
// 	emptyPQ(&h);
// 	return 0;

// }