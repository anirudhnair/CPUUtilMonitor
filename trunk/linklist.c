#include <linux/module.h>    // included for all kernel modules
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>      // included for __init and __exit macros
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/rwsem.h>

#include "common.h"
#include "linklist.h"
#define BUF_SIZE 1024

// structure that stores info regarding each process

struct process_info {
	int pid;
	unsigned long cpu_time;
	struct list_head list; 
	/* list_head is added to the struct which need to be a list. This apporcah is
	*  different from conventional approach of add the struct to a list. Here it 
	*  is the reverse.
	*/
};

// read-write lock
struct rw_semaphore *sem = NULL;


// declare and intilaize the list
static struct process_info proc_list;
static int list_size = 0;

int ll_list_size(void) {
	return list_size;
}

int ll_initialize_list(void)
{ 
	//printk(KERN_INFO "Linklist initialize begin\n");
	// intialize the linklist
	INIT_LIST_HEAD(&proc_list.list);
	
	//printk(KERN_INFO "Linklist initialize done\n");
	// initialize the rwsem
	//printk(KERN_INFO "RW lock initialize begin\n");
	sem  = (struct rw_semaphore*)kmalloc(sizeof(struct rw_semaphore),GFP_KERNEL);
	init_rwsem(sem);
	//printk(KERN_INFO "RW lock initialize done\n");
	return SUCCESS;
}


int ll_is_pid_in_list(int pid)
{
	struct process_info *proc_iter = NULL;
	down_read(sem); //read lock acquire
	list_for_each_entry(proc_iter,&proc_list.list,list) {
		if(proc_iter->pid == pid) 
			return TRUE;
	}
	up_read(sem); //read lock release
	return FALSE;
}

int ll_generate_cpu_info_string(char **buf, int *count_)
{
	int count = 0;
	struct process_info *proc_iter = NULL;
	*buf = (char *)kmalloc(BUF_SIZE, GFP_KERNEL);
	down_read(sem); // acquire read lock
	list_for_each_entry(proc_iter,&proc_list.list,list) {
		count += sprintf(*buf+count,"%d : %lu ms\n",proc_iter->pid,proc_iter->cpu_time);
	}
	(*buf)[count] = '\0';
	up_read(sem); // release read lock
	//printk(KERN_INFO "===========generate string(): count=%d\n", count + 1);
	//printk(KERN_INFO "===========String: %s\n",*buf);
    *count_ = count + 1;
	return SUCCESS;

}


int ll_update_time(int pid,unsigned long cpu_use)
{
	struct process_info *proc_iter = NULL;
	//printk(KERN_INFO "update_time starts for pid=%d\n",pid);
	down_write(sem); // acquire write lock
	list_for_each_entry(proc_iter,&proc_list.list,list) {
		 if( proc_iter->pid == pid )
		 {
			 proc_iter->cpu_time = cpu_use;
			 //printk(KERN_INFO "update time ends for pid=%d\n",pid);
		 	 up_write(sem);
			 return SUCCESS;
		 }
	}
	up_write(sem); // release write lock
	//printk(KERN_INFO "update_time() pid=%d not found in the list\n",pid);
	return FAIL;
}

int ll_add_to_list(int pid)
{
	//printk(KERN_INFO "adding pid=%d to list\n",pid);
	if( ll_is_pid_in_list(pid) == FALSE )
	{
		struct process_info *new_proc = NULL;
		new_proc = (struct process_info *)kmalloc(sizeof(struct process_info),GFP_KERNEL);
        new_proc->pid = pid;
		INIT_LIST_HEAD(&new_proc->list);
		down_write(sem);
		list_add_tail(&(new_proc->list),&(proc_list.list));
		up_write(sem);
		//printk(KERN_INFO "added pid=%d to list\n",pid);
		list_size++;
		return SUCCESS;
	}
	else
	{
		//printk(KERN_INFO "pid=%d found to be duplicate. not added to the list\n",pid);
		return DUPLICATE;
	}
}

int ll_cleanup(void)
{
	struct process_info *proc_iter = NULL;
	//printk(KERN_INFO "linklist cleanup starts\n");
	down_write(sem);
	list_for_each_entry(proc_iter,&proc_list.list,list) {
		list_del(&proc_iter->list);
		kfree(proc_iter);
	}
	up_write(sem);
	kfree(sem);
	//printk(KERN_INFO "linklist cleanup ends\n");
    return SUCCESS;
}

int ll_get_pids(int **pids, int *count)
{
	//create memory for list_size of pids
	struct process_info *proc_iter = NULL;
	int index = 0;
	*count = list_size;
	//printk(KERN_INFO "linklist get_pids()\n");
	if ( list_size > 0 )
	{
		*pids = (int *)kmalloc(sizeof(int)*list_size,GFP_KERNEL);
		//printk(KERN_INFO "linklist get_pids() trying to acquire lock\n");
		down_read(sem); // acquire read lock
		//printk(KERN_INFO "linklist get_pids() lock acquired\n");
		list_for_each_entry(proc_iter,&proc_list.list,list) {
			(*pids)[index++] = proc_iter->pid;
		}
		up_read(sem);

	    //printk(KERN_INFO "linklist get_pids() lock release\n");
	}
	return SUCCESS;
}

int ll_delete_pid(int pid)
{
	struct process_info *proc_iter = NULL;
    down_write(sem);	
	list_for_each_entry(proc_iter,&proc_list.list,list) {
		if (proc_iter->pid == pid )
		{
			list_del(&proc_iter->list);
			kfree(proc_iter);
			list_size--;
			up_write(sem);
			return SUCCESS;
		}
	}
	up_write(sem);
	return NOT_FOUND;
}

void ll_print_list(void)
{
	char **buf = NULL;
	int count = 0;
	ll_generate_cpu_info_string(buf,&count);
	//printk(KERN_INFO "%s",*buf);
	kfree(*buf);
}
