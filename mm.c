#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define	PROCESS_NAME_LEN	32	//����������
#define	MIN_SLICE	10	//��С��Ƭ��С
#define	DEFAULT_MEM_SIZE	1024	//Ĭ���ڴ��С
#define DEFAULT_MEM_START	0	//�ڴ���ʼλ��

#define	MA_FF	1
#define MA_BF	2
#define MA_WF	3

int mem_size = DEFAULT_MEM_SIZE;	//�����ڴ��С����ʼ��ΪĬ���ڴ��С
int mem_size_total = DEFAULT_MEM_SIZE;	//�ܹ��ڴ��С����ʼ��ΪĬ�ϴ�С
int ma_algorithm = MA_FF;	//��ǰ�ڴ�����㷨����ʼ��ΪFF
static int pid = 0;	//����pid�ţ���ʼֵΪ0
int flag = 0;	//�����ڴ��С��־����ֹ��������

//����ÿһ�����п�����ݽṹ
struct free_block_type
{
	int	size;
	int start_addr;
	struct free_block_type* next;
};
//����ÿ�����̷��䵽���ڴ��
struct allocated_block
{
	int pid;
	int size;
	int start_addr;
	char process_name[PROCESS_NAME_LEN];
	struct allocated_block* next;
};
//ָ���ڴ��п��п��������ָ��
struct free_block_type* free_block_head = NULL;
//���̷����ڴ���������ָ��
struct allocated_block* allocated_block_head = NULL;

/***************************************��������*****************************************/
//��ʼ�����п飬Ĭ��Ϊһ�飬����ָ����С����ʼλ��
struct free_block_type* init_free_block(int mem_size);
//��ʾ�˵�
void display_menu();
//�˳�����
void do_exit();
//�����ڴ��С
void set_mem_size();
//���õ�ǰ�ķ����㷨
void set_algorithm();
//��ָ���㷨�������п���������
void rearrange(int algorithm);
//FF�㷨
void rearrange_FF();
//BF�㷨
void rearrange_BF();
//WF�㷨
void rearrange_WF();
//�����µĽ��̣���Ҫ�ǻ�ȡ�ڴ����������
void new_process();
//ɾ�����̣��黹����Ĵ洢�ռ䣬��ɾ�������ý����ڴ����Ľڵ�
void kill_process();
//�����ڴ�ģ��
int allocate_mem(struct allocated_block* ab);
//��ab����ʾ���ѷ������黹�������п��ܵĺϲ�
int free_mem(struct allocated_block* ab);
//�ͷ�ab���ݽṹ�ڵ�
int dispose(struct allocated_block* free_ab);
//��ʾ��ǰ�ڴ��ʹ�������������������������Ѿ���������
int display_mem_usage();
//�ҵ�Ҫɾ���Ľ���
struct allocated_block* find_process(int pid);
//��������
void free_memory_rearrange(int memory_reduce_size, int allocated_size);
/***************************************��������*****************************************/

int main()
{
	int choice;
	free_block_head = init_free_block(mem_size);

	while (1)
	{
		display_menu();	//��ʾ�˵�
		fflush(stdin);	//��ջ���������ֹ��ѡ
		scanf("%d", &choice);	//��ȡ�û�����
		if (choice <= 5 && choice >= 0)
		{
			switch (choice)
			{
			case 1: set_mem_size(); break;	//�����ڴ��С
			case 2: set_algorithm(); flag = 1; break;	//�����㷨
			case 3: new_process(); flag = 1; break;	//�����½���
			case 4: kill_process(); flag = 1; break;	//ɾ������
			case 5: display_mem_usage(); flag = 1; break;	//��ʾ�ڴ�ʹ��
			case 0: do_exit(); exit(0);	//�ͷ������˳�
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

	if (flag)	//����Ƿ��ظ�����
	{
		printf("Cannot set memory size again!\n");
		return;
	}

	printf("\nTotal memory size = ");
	fflush(stdin);
	scanf("%d", &size);

	if (size >= 0)	//����ڴ��С�Ƿ�Ϸ�
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
	//�����̷����ڴ�Ľ��
	int allocate_ret;

	//����һ������
	ab = (struct allocated_block*)malloc(sizeof(struct allocated_block));
	if (!ab)	exit(1);

	//ȷ����������
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

	//�ӿ����������ڴ棬���ط�������1��ʾ����ɹ���-1��ʾ����ʧ��
	allocate_ret = allocate_mem(ab);

	if ((allocate_ret == 1) && (allocated_block_head == NULL))	//����ǵ�һ���ڵ�
	{
		allocated_block_head = ab;
	}

	else if ((allocate_ret == 1) && (allocated_block_head != NULL))	//������ǵ�һ���ڵ�
	{
		ab->next = allocated_block_head;
		allocated_block_head = ab;
	}

	else if (allocate_ret == -1)	//�������ʧ��
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
	int allocate_flag = 0;	//�ж��Ƿ��Ѿ��ҵ�ƥ����п�
	
	//���ݵ�ǰ�㷨�ڿ��з����������������ʿ��з������з��䣬����ʱע�����������
	// 1. �ҵ���������з����ҷ����ʣ��ռ��㹻����ָ�
	// 2. �ҵ���������з����ҵ������ʣ��ռ�Ƚ�С����һ�����
	// 3. �Ҳ�����������Ҫ�Ŀ��з��������з���֮����������Ҫ��������ڴ�������������п��з����ĺϲ���Ȼ���ٷ���
	// 4. �ڳɹ������ڴ��Ӧ���ֿ��з���������Ӧ�㷨����
	// 5. ����ɹ��򷵻�1�����򷵻�-1

	if (mem_size <= 0)	return -1;
	
	//��������ƥ����п�
	if (ne->next)	//������п鲻ֹһ��	
	{
		if (ma_algorithm == 1)	//�����FF�㷨������ÿһ�����п�
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
		else if (ma_algorithm == 2)	//�����BF�㷨�������ÿһ����С����Ҫ��Ŀ��п�
		{
			while ((ne != NULL) && (request_size <= ne->size))	
			{
				if (allocate_flag)	fbt = pre;
				pre = ne;
				ne = ne->next;
				allocate_flag = 1;
			}
		}
		else if (ma_algorithm == 3)	//�����WF�㷨����ֱ�Ӳ������һ�����п�
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
	

	if (allocate_flag)	//�ҵ����ÿ��������ж��費��Ҫһ�����ʣ���ڴ�ռ�
	{
		if ((pre->size - request_size) >= MIN_SLICE)	//�ҵ���������з����ҷ����ʣ��ռ�Ƚϴ�����������
		{
			pre->size = pre->size - request_size;
			ab->start_addr = pre->start_addr;
			pre->start_addr += ab->size;
		}

		else	//�ҵ���������з����ҷ����ʣ��ռ�Ƚ�С����һ����䣬ɾ���ýڵ�
		{
			if (fbt == pre)	//���ͷ����������
			{
				fbt = pre->next;
				free_block_head = fbt;
			}
			else	//�м���п���������
				fbt->next = pre->next;

			ab->start_addr = pre->start_addr;
			ab->size = pre->size;
			free(pre);	//�ͷŽڵ�
		}

		mem_size -= ab->size;
		rearrange(ma_algorithm);
		return 1;
	}

	else	//�Ҳ�����������������ڴ����
	{
		if (mem_size >= request_size)
		{
			if (mem_size >= request_size + MIN_SLICE)	//�������ڴ�����п����ڴ�
				free_memory_rearrange(mem_size - request_size, request_size);
			else	//�������ڴ���޿����ڴ�
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

	//���п鴦��
	if (memory_reduce_size != 0)	//������ʣ��ռ������С�ڴ���Ƭ
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

	while (f2 != NULL)	//��һ�ͷſ����ڴ��ڵ�
	{
		f1 = f2;
		f2 = f2->next;
		free(f1);
	}

	//���ؿ鴦��
	a1 = (struct allocated_block*)malloc(sizeof(struct allocated_block));
	a1->pid = pid;
	a1->size = allocated_size;
	a1->start_addr = mem_size_total - memory_reduce_size - a1->size;
	sprintf(a1->process_name, "PROCESS-%02d", pid);

	a1->next = allocated_block_head;
	a2 = allocated_block_head;
	allocated_block_head = a1;

	while (a2 != NULL)	//��һ�����ؿ����ڷ���
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
		free_mem(ab);	//�ͷ�ab����ʾ�ķ�����
		dispose(ab);	//�ͷ�ab���ݽṹ�ڵ�
	}
}

struct allocated_block* find_process(int pid)
{
	struct allocated_block* p;
	p = allocated_block_head;
	while (p)	//����������pid��Ӧ����
	{
		if (p->pid == pid)
			return p;	//�ҵ��򷵻�struct
		p = p->next;
	}

	printf("\nProcess not found!\n");	//û���ҵ��򱨴�����NULL
	return NULL;
}

int free_mem(struct allocated_block* ab)
{
	int algorithm = ma_algorithm;
	struct free_block_type* fbt, * left, * right;	//����ṹ��Ϊ����ָ���ң�fbt�洢Ҫ�ͷŵķ���
													//leftΪ�������ߣ�������ͷ���Ŀ��з�����rightΪ������ұߣ�Զ���ͷ���Ŀ��з���
	mem_size += ab->size;

	fbt = (struct free_block_type*)malloc(sizeof(struct free_block_type));
	if (!fbt)	return -1;

	//�����ڴ�4�������
	// 1. ��ǰ���з������ұ߿��з������ڣ��ϲ�Ϊͬһ�����������ͷ��ұ߷��� 
	// 2. ��ǰ���з�������߿��з������ڣ��ϲ�Ϊͬһ�����������ͷŵ�ǰ����
	// 3. ��ǰ���з��������ҿ��з��������ڣ��ϲ�Ϊͬһ�����������ͷŵ�ǰ���ұ߷���
	// 4. �����ڿ��з����������һ���±���

	fbt->size = ab->size;
	fbt->start_addr = ab->start_addr;
	fbt->next = NULL;
	rearrange(MA_FF);

	left = NULL;
	//��ͷ��ʼ������ʼ��ַ˳��������жϲ��������λ��
	right = free_block_head;
	while ((right != NULL) && (fbt->start_addr < right->start_addr))
	{
		left = right;
		right = right->next;
	}

	if (!left)	//����λ��Ϊ����ͷ
	{
		if (!right)	//����ͷ��ڴ�ǰ�Ѿ�û�п��з���
			free_block_head = fbt;
		else
		{
			fbt->next = right;
			free_block_head = fbt;
			if (right->start_addr + right->size == fbt->start_addr)	//�ж��ͷŵĿ���������ұ߿��з����Ƿ����ڣ�����ϲ�
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
		if (!right)	//��������λ��������β
		{
			left->next = fbt;

			if (fbt->start_addr + fbt->size == left->start_addr)	//�ж��ͷŵĿ����������߿��з����Ƿ����ڣ�����ϲ�
			{
				left->next = right;
				left->size = fbt->size + left->size;
				left->start_addr = fbt->start_addr;
				free(fbt);
			}
		}

		else	//��������λ���������м�
		{
			fbt->next = right;
			left->next = fbt;

			if ((fbt->start_addr + fbt->size == left->start_addr) && (right->start_addr + right->size == fbt->start_addr))	//�����Ҷ�����
			{
				left->next = right->next;
				left->size += fbt->size + right->size;
				left->start_addr = right->start_addr;
				free(fbt);
				free(right);
			}
			else if (fbt->start_addr + fbt->size == left->start_addr)	//���������
			{
				left->next = right;
				left->size = fbt->size + left->size;
				left->start_addr = fbt->start_addr;
				free(fbt);
			}
			else if (right->start_addr + right->size == fbt->start_addr)	//���ұ�����
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
	if (free_ab == allocated_block_head)	//����ͷ�ͷ�ڵ�
	{
		allocated_block_head = allocated_block_head->next;
		free(free_ab);
		return 1;
	}

	pre = allocated_block_head;
	ab = allocated_block_head->next;
	while (ab != free_ab)	//���������ҵ�Ҫ�ͷŵĽڵ�
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

	//��ʾ������
	printf("Free Memory:\n");
	printf("%20s %20s\n", "start_addr", "size");
	while (fbt != NULL)
	{
		printf("%20d %20d\n", fbt->start_addr, fbt->size);
		fbt = fbt->next;
	}

	//��ʾ�ѷ�����
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
		for (last_block = NULL; last_block != free_block_head; last_block = p)	//ð�����򣬰���ʼ��ַ�Ӵ�С��
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
		for (last_block = NULL; last_block != free_block_head; last_block = p)	//ð�����򣬰����С�Ӵ�С��
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
		for (last_block = NULL; last_block != free_block_head; last_block = p)	//ð�����򣬰����С��С������
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