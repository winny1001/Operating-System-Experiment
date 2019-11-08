#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define	PROCESS_NAME_LEN	32	//进程名长度
#define	MIN_SLICE	10	//最小碎片大小
#define	DEFAULT_MEM_SIZE	1024	//默认内存大小
#define DEFAULT_MEM_START	0	//内存起始位置

#define	MA_FF	1
#define MA_BF	2
#define MA_WF	3

int mem_size = DEFAULT_MEM_SIZE;	//可用内存大小，初始化为默认内存大小
int mem_size_total = DEFAULT_MEM_SIZE;	//总共内存大小，初始化为默认大小
int ma_algorithm = MA_FF;	//当前内存分配算法，初始化为FF
static int pid = 0;	//进程pid号，初始值为0
int flag = 0;	//设置内存大小标志，防止重新设置

//描述每一个空闲块的数据结构
struct free_block_type
{
	int	size;
	int start_addr;
	struct free_block_type* next;
};
//描述每个进程分配到的内存块
struct allocated_block
{
	int pid;
	int size;
	int start_addr;
	char process_name[PROCESS_NAME_LEN];
	struct allocated_block* next;
};
//指向内存中空闲块链表的首指针
struct free_block_type* free_block_head = NULL;
//进程分配内存块链表的首指针
struct allocated_block* allocated_block_head = NULL;

/***************************************函数声明*****************************************/
//初始化空闲块，默认为一块，可以指定大小及起始位置
struct free_block_type* init_free_block(int mem_size);
//显示菜单
void display_menu();
//退出程序
void do_exit();
//设置内存大小
void set_mem_size();
//设置当前的分配算法
void set_algorithm();
//按指定算法重新排列空闲区链表
void rearrange(int algorithm);
//FF算法
void rearrange_FF();
//BF算法
void rearrange_BF();
//WF算法
void rearrange_WF();
//创建新的进程，主要是获取内存的申请数量
void new_process();
//删除进程，归还分配的存储空间，并删除描述该进程内存分配的节点
void kill_process();
//分配内存模块
int allocate_mem(struct allocated_block* ab);
//将ab所表示的已分配区归还，并进行可能的合并
int free_mem(struct allocated_block* ab);
//释放ab数据结构节点
int dispose(struct allocated_block* free_ab);
//显示当前内存的使用情况，包括空闲区的情况和已经分配的情况
int display_mem_usage();
//找到要删除的进程
struct allocated_block* find_process(int pid);
//紧缩处理
void free_memory_rearrange(int memory_reduce_size, int allocated_size);
/***************************************函数声明*****************************************/

int main()
{
	int choice;
	free_block_head = init_free_block(mem_size);

	while (1)
	{
		display_menu();	//显示菜单
		fflush(stdin);	//清空缓冲区，防止误选
		scanf("%d", &choice);	//获取用户输入
		if (choice <= 5 && choice >= 0)
		{
			switch (choice)
			{
			case 1: set_mem_size(); break;	//设置内存大小
			case 2: set_algorithm(); flag = 1; break;	//设置算法
			case 3: new_process(); flag = 1; break;	//创建新进程
			case 4: kill_process(); flag = 1; break;	//删除进程
			case 5: display_mem_usage(); flag = 1; break;	//显示内存使用
			case 0: do_exit(); exit(0);	//释放链表并退出
			default: break;
			}
		}

		else
		{
			printf("\nInvalid choice, please select again!\n\n");
		}

	}

	return 0;
}

struct free_block_type* init_free_block(int mem_size)
{
	struct free_block_type* fb;
	fb = (struct free_block_type*)malloc(sizeof(struct free_block_type));

	if (fb == NULL)
	{
		printf("No memory!\n");
		return NULL;
	}

	fb->size = mem_size;
	fb->start_addr = DEFAULT_MEM_START;
	fb->next = NULL;

	return fb;
}

void display_menu()
{

	printf("\n");
	printf("----------------------------------------\n");
	printf("      Memory Management Experiment\n");
	printf("          (c) WPX 2176112425\n");
	printf("               2019.11.3\n");
	printf("----------------------------------------\n\n");
	printf("current algo: %d\tcurrent mem_size: %d\n\n", ma_algorithm, mem_size_total);
	printf("Please enter a number to select the appropriate function:\n\n");
	printf("1 -- Set memory size(default=%d)\n", DEFAULT_MEM_SIZE);
	printf("2 -- Select memory allocation algorithm\n");
	printf("3 -- New process\n");
	printf("4 -- Terminate a process\n");
	printf("5 -- Display memory usage\n");
	printf("0 -- Exit\n\n");
	printf(">> ");
}

void set_mem_size()
{
	int size;

	if (flag)	//检查是否重复设置
	{
		printf("Cannot set memory size again!\n");
		return;
	}

	printf("\nTotal memory size = ");
	fflush(stdin);
	scanf("%d", &size);

	if (size >= 0)	//检查内存大小是否合法
	{
		mem_size = size;
		mem_size_total = size;
		free_block_head->size = mem_size;
		flag = 1;
	}

	else printf("Memory size is not valid!\n");
}

void set_algorithm()
{
	int algorithm;
	printf("Please enter a number to select the appropriate algorithm:\n\n");
	printf("1 -- First Fit\n");
	printf("2 -- Best Fit\n");
	printf("3 -- Worst Fit\n");
	printf(">> ");
	scanf("%d", &algorithm);

	if (algorithm >= 1 && algorithm <= 3)
	{
		ma_algorithm = algorithm;
		rearrange(ma_algorithm);
	}
	else
		printf("Invalid choice!\n");
}

void rearrange(int algorithm)
{
	switch (algorithm)
	{
	case MA_FF: rearrange_FF(); break;
	case MA_BF:	rearrange_BF(); break;
	case MA_WF: rearrange_WF(); break;
	}
}

void new_process()
{
	struct allocated_block* ab;
	int size;
	//给进程分配内存的结果
	int allocate_ret;

	//创建一个进程
	ab = (struct allocated_block*)malloc(sizeof(struct allocated_block));
	if (!ab)	exit(1);

	//确定进程属性
	ab->next = NULL;
	pid++;
	sprintf(ab->process_name, "PROCESS-%02d", pid);
	ab->pid = pid;
	printf("\nPlease input the memory for PROCESS-%02d: ", pid);
	scanf("%d", &size);
	if (size > 0)	ab->size = size;
	else
	{
		printf("\nInvalid memory size!\n");
		return;
	}

	//从空闲区分配内存，返回分配结果。1表示分配成功，-1表示分配失败
	allocate_ret = allocate_mem(ab);

	if ((allocate_ret == 1) && (allocated_block_head == NULL))	//如果是第一个节点
	{
		allocated_block_head = ab;
	}

	else if ((allocate_ret == 1) && (allocated_block_head != NULL))	//如果不是第一个节点
	{
		ab->next = allocated_block_head;
		allocated_block_head = ab;
	}

	else if (allocate_ret == -1)	//如果分配失败
	{
		printf("\nAllocation failed!\n");
		free(ab);
		return;
	}

	printf("\nAllocation success!\n");
}

int allocate_mem(struct allocated_block* ab)
{
	struct free_block_type* fbt, * pre, * ne, * p1, * p2;
	int request_size = ab->size;
	fbt = pre = ne = p1 = p2 = free_block_head;
	int allocate_flag = 0;	//判断是否已经找到匹配空闲块
	
	//根据当前算法在空闲分区链表中搜索合适空闲分区进行分配，分配时注意以下情况：
	// 1. 找到可满足空闲分区且分配后剩余空间足够大，则分割
	// 2. 找到可满足空闲分区且但分配后剩余空间比较小，则一起分配
	// 3. 找不到可满足需要的空闲分区但空闲分区之和能满足需要，则采用内存紧缩技术，进行空闲分区的合并，然后再分配
	// 4. 在成功分配内存后，应保持空闲分区按照相应算法有序
	// 5. 分配成功则返回1，否则返回-1

	if (mem_size <= 0)	return -1;
	
	//遍历查找匹配空闲块
	if (ne->next)	//如果空闲块不止一个	
	{
		if (ma_algorithm == 1)	//如果是FF算法，遍历每一个空闲块
		{
			while (ne)
			{
				if (ne != free_block_head)	p2 = p1;
				p1 = ne;
				ne = ne->next;
				if (request_size <= p1->size)
				{
					fbt = p2;
					pre = p1;
					allocate_flag = 1;
				}
			}
		}
		else if (ma_algorithm == 2)	//如果是BF算法，则遍历每一个大小满足要求的空闲块
		{
			while ((ne != NULL) && (request_size <= ne->size))	
			{
				if (allocate_flag)	fbt = pre;
				pre = ne;
				ne = ne->next;
				allocate_flag = 1;
			}
		}
		else if (ma_algorithm == 3)	//如果是WF算法，则直接查找最后一个空闲块
		{
			while (ne)
			{
				if (ne != free_block_head)	fbt = pre;
				pre = ne;
				ne = ne->next;
			}
			if (pre->size >= request_size)	allocate_flag = 1;
		}
	}
	else
	{
		if (request_size <= pre->size)
			allocate_flag = 1;
	}
	

	if (allocate_flag)	//找到可用空闲区，判断需不需要一起分配剩余内存空间
	{
		if ((pre->size - request_size) >= MIN_SLICE)	//找到可满足空闲分区且分配后剩余空间比较大，则正常分配
		{
			pre->size = pre->size - request_size;
			ab->start_addr = pre->start_addr;
			pre->start_addr += ab->size;
		}

		else	//找到可满足空闲分区且分配后剩余空间比较小，则一起分配，删除该节点
		{
			if (fbt == pre)	//如果头块满足条件
			{
				fbt = pre->next;
				free_block_head = fbt;
			}
			else	//中间空闲块满足条件
				fbt->next = pre->next;

			ab->start_addr = pre->start_addr;
			ab->size = pre->size;
			free(pre);	//释放节点
		}

		mem_size -= ab->size;
		rearrange(ma_algorithm);
		return 1;
	}

	else	//找不到空闲区，则进行内存紧缩
	{
		if (mem_size >= request_size)
		{
			if (mem_size >= request_size + MIN_SLICE)	//分配完内存后还留有空闲内存
				free_memory_rearrange(mem_size - request_size, request_size);
			else	//分配完内存后无空闲内存
				free_memory_rearrange(0, mem_size);
			return 0;
		}

		else
			return -1;
	}

}

void free_memory_rearrange(int memory_reduce_size, int allocated_size)
{
	struct free_block_type* f1, * f2;
	struct allocated_block* a1, * a2;

	//空闲块处理
	if (memory_reduce_size != 0)	//分配完剩余空间大于最小内存碎片
	{
		f1 = free_block_head;
		f2 = f1->next;

		f1->start_addr = mem_size_total - memory_reduce_size;
		f1->size = memory_reduce_size;
		f1->next = NULL;

		mem_size = memory_reduce_size;
		
	}
	else
	{
		f2 = free_block_head;
		free_block_head = NULL;
		mem_size = 0;
	}

	while (f2 != NULL)	//逐一释放空闲内存块节点
	{
		f1 = f2;
		f2 = f2->next;
		free(f1);
	}

	//加载块处理
	a1 = (struct allocated_block*)malloc(sizeof(struct allocated_block));
	a1->pid = pid;
	a1->size = allocated_size;
	a1->start_addr = mem_size_total - memory_reduce_size - a1->size;
	sprintf(a1->process_name, "PROCESS-%02d", pid);

	a1->next = allocated_block_head;
	a2 = allocated_block_head;
	allocated_block_head = a1;

	while (a2 != NULL)	//逐一将加载块相邻放置
	{
		a2->start_addr = a1->start_addr - a2->size;
		a1 = a2;
		a2 = a2->next;
	}

}

void kill_process()
{
	struct allocated_block* ab;
	int pid;
	printf("\nKill Process, pid = ");
	scanf("%d", &pid);
	ab = find_process(pid);
	if (ab != NULL)
	{
		free_mem(ab);	//释放ab所表示的分配区
		dispose(ab);	//释放ab数据结构节点
	}
}

struct allocated_block* find_process(int pid)
{
	struct allocated_block* p;
	p = allocated_block_head;
	while (p)	//遍历链表找pid对应进程
	{
		if (p->pid == pid)
			return p;	//找到则返回struct
		p = p->next;
	}

	printf("\nProcess not found!\n");	//没有找到则报错并返回NULL
	return NULL;
}

int free_mem(struct allocated_block* ab)
{
	int algorithm = ma_algorithm;
	struct free_block_type* fbt, * left, * right;	//链表结构认为从左指向右，fbt存储要释放的分区
													//left为插入后左边（靠近表头）的空闲分区、right为插入后右边（远离表头）的空闲分区
	mem_size += ab->size;

	fbt = (struct free_block_type*)malloc(sizeof(struct free_block_type));
	if (!fbt)	return -1;

	//回收内存4种情况：
	// 1. 当前空闲分区和右边空闲分区相邻，合并为同一个分区，且释放右边分区 
	// 2. 当前空闲分区和左边空闲分区相邻，合并为同一个分区，且释放当前分区
	// 3. 当前空闲分区和左右空闲分区都相邻，合并为同一个分区，且释放当前和右边分区
	// 4. 无相邻空闲分区，则插入一个新表项

	fbt->size = ab->size;
	fbt->start_addr = ab->start_addr;
	fbt->next = NULL;
	rearrange(MA_FF);

	left = NULL;
	//从头开始按照起始地址顺序遍历，判断插入链表的位置
	right = free_block_head;
	while ((right != NULL) && (fbt->start_addr < right->start_addr))
	{
		left = right;
		right = right->next;
	}

	if (!left)	//插入位置为链表头
	{
		if (!right)	//如果释放内存前已经没有空闲分区
			free_block_head = fbt;
		else
		{
			fbt->next = right;
			free_block_head = fbt;
			if (right->start_addr + right->size == fbt->start_addr)	//判断释放的空闲区间和右边空闲分区是否相邻，是则合并
			{
				fbt->next = right->next;
				fbt->start_addr = right->start_addr;
				fbt->size = fbt->size + right->size;
				free(right);
			}
		}
	}

	else	
	{
		if (!right)	//如果插入的位置在链表尾
		{
			left->next = fbt;

			if (fbt->start_addr + fbt->size == left->start_addr)	//判断释放的空闲区间和左边空闲分区是否相邻，是则合并
			{
				left->next = right;
				left->size = fbt->size + left->size;
				left->start_addr = fbt->start_addr;
				free(fbt);
			}
		}

		else	//如果插入的位置在链表中间
		{
			fbt->next = right;
			left->next = fbt;

			if ((fbt->start_addr + fbt->size == left->start_addr) && (right->start_addr + right->size == fbt->start_addr))	//和左右都相邻
			{
				left->next = right->next;
				left->size += fbt->size + right->size;
				left->start_addr = right->start_addr;
				free(fbt);
				free(right);
			}
			else if (fbt->start_addr + fbt->size == left->start_addr)	//和左边相邻
			{
				left->next = right;
				left->size = fbt->size + left->size;
				left->start_addr = fbt->start_addr;
				free(fbt);
			}
			else if (right->start_addr + right->size == fbt->start_addr)	//和右边相邻
			{
				fbt->next = right->next;
				fbt->start_addr = right->start_addr;
				fbt->size = fbt->size + right->size;
				free(right);
			}

		}
	}

	rearrange(ma_algorithm);
	return 1;
}

int dispose(struct allocated_block* free_ab)
{
	struct allocated_block* pre, * ab;
	if (free_ab == allocated_block_head)	//如果释放头节点
	{
		allocated_block_head = allocated_block_head->next;
		free(free_ab);
		return 1;
	}

	pre = allocated_block_head;
	ab = allocated_block_head->next;
	while (ab != free_ab)	//遍历链表找到要释放的节点
	{
		pre = ab;
		ab = ab->next;
	}
	pre->next = ab->next;
	free(ab);
	return 2;
}

int display_mem_usage()
{
	struct free_block_type* fbt = free_block_head;
	struct allocated_block* ab = allocated_block_head;
	
	printf("-------------------------------------------------------------\n");

	//显示空闲区
	printf("Free Memory:\n");
	printf("%20s %20s\n", "start_addr", "size");
	while (fbt != NULL)
	{
		printf("%20d %20d\n", fbt->start_addr, fbt->size);
		fbt = fbt->next;
	}

	//显示已分配区
	printf("\nUsed Memory:\n");
	printf("%10s %20s %10s %10s\n", "PID", "Process Name", "start_addr", "size");
	while (ab != NULL)
	{
		printf("%10d %20s %10d %10d\n", ab->pid, ab->process_name, ab->start_addr, ab->size);
		ab = ab->next;
	}
	printf("-------------------------------------------------------------\n");
	return 0;
}

void rearrange_FF()
{
	struct free_block_type* p, * p1, * p2;
	struct free_block_type* last_block;
	p1 = (struct free_block_type*)malloc(sizeof(struct free_block_type));
	p1->next = free_block_head;
	free_block_head = p1;
	if (free_block_head != NULL)
	{
		for (last_block = NULL; last_block != free_block_head; last_block = p)	//冒泡排序，按起始地址从大到小排
		{
			for (p = p1 = free_block_head; p1->next != NULL && p1->next->next != NULL && p1->next->next != last_block; p1 = p1->next)
			{
				if (p1->next->start_addr < p1->next->next->start_addr)
				{
					p2 = p1->next->next;
					p1->next->next = p2->next;

					p2->next = p1->next;
					p1->next = p2;

					p = p1->next->next;
				}
			}
		}
	}

	p1 = free_block_head;
	free_block_head = free_block_head->next;
	free(p1);
}

void rearrange_BF()
{
	struct free_block_type* p, * p1, * p2;
	struct free_block_type* last_block;
	p1 = (struct free_block_type*)malloc(sizeof(struct free_block_type));
	p1->next = free_block_head;
	free_block_head = p1;
	if (free_block_head != NULL)
	{
		for (last_block = NULL; last_block != free_block_head; last_block = p)	//冒泡排序，按块大小从大到小排
		{
			for (p = p1 = free_block_head; p1->next != NULL && p1->next->next != NULL && p1->next->next != last_block; p1 = p1->next)
			{
				if (p1->next->size < p1->next->next->size)
				{
					p2 = p1->next->next;
					p1->next->next = p2->next;

					p2->next = p1->next;
					p1->next = p2;

					p = p1->next->next;
				}
			}
		}
	}
	p1 = free_block_head;
	free_block_head = free_block_head->next;
	free(p1);
	p1 = NULL;
}

void rearrange_WF()
{
	struct free_block_type* p, * p1, * p2;
	struct free_block_type* last_block;
	p1 = (struct free_block_type*)malloc(sizeof(struct free_block_type));
	p1->next = free_block_head;
	free_block_head = p1;
	if (free_block_head != NULL) {
		for (last_block = NULL; last_block != free_block_head; last_block = p)	//冒泡排序，按块大小从小到大排
		{
			for (p = p1 = free_block_head; p1->next != NULL && p1->next->next != NULL && p1->next->next != last_block; p1 = p1->next)
			{
				if (p1->next->size > p1->next->next->size)
				{
					p2 = p1->next->next;
					p1->next->next = p2->next; 

					p2->next = p1->next;
					p1->next = p2;

					p = p1->next->next;
				}
			}
		}


	}

	p1 = free_block_head;
	free_block_head = free_block_head->next;
	free(p1);
	p1 = NULL;
}

void do_exit()
{
	struct free_block_type* p1, * p2;
	struct allocated_block* a1, * a2;
	p1 = free_block_head;
	if (p1 != NULL)
	{
		p2 = p1->next;
		for (; p2 != NULL; p1 = p2, p2 = p2->next)
		{
			free(p1);
		}
		free(p1);
	}
	a1 = allocated_block_head;
	if (a1 != NULL)
	{
		a2 = a1->next;
		for (; a2 != NULL; a1 = a2, a2 = a2->next)
		{
			free(a1);
		}
		free(a1);
	}
}