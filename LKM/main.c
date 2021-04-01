#include "include/rule.h"
#include "include/main.h"
#include "include/chain.h"
#include "include/handle.h"
#include "include/proto.h"
#include "include/log.h"
#include "include/cmd.h"


//the pro file to communicate the user-space
static struct proc_dir_entry *proc_entry;
static struct proc_dir_entry *log_proc_entry;

static struct nf_hook_ops outcome_filter;
static struct nf_hook_ops income_filter;
static struct nf_hook_ops forward_filter;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MARCH1917");
MODULE_DESCRIPTION("A SIMPLE FIREWALL");

//chain declarations
extern chain_ptr_t income_chain,outcome_chain,forward_chain;

//switch which indicates the firewall status
switch_t firewall_switch=OFF;
switch_t firewall_enable=OFF;


//handle declarations
/*
extern ssize_t handle_read(struct file *file,char __user *ubuf,size_t count,loff_t *ppos);
extern ssize_t handle_write(struct file *file,const char __user *ubuf,size_t count,loff_t *ppos);
*/

//log buffer
extern char log_tmp_buffer[LOG_TMP_SIZE];
extern char log_to_read_buffer[LOG_BUFFER_SIZE];

unsigned int income_filter_hookfunc(void* priv,struct sk_buff *skb,const struct nf_hook_state *state){
	rule_ptr_t tmp_rule=income_chain->head->next;
	int i;
	for(i=0;i<income_chain->rule_num;i++,tmp_rule=tmp_rule->next){
		target_t target;
		if((target=check_rule(skb,tmp_rule))!=INIT_TARGET){
			//match success

			//to-do log

			if(tmp_rule->alarm==ALARM_ON){
				//to-do alarm
			}

			printk("function income filter match result:%d",target);
			return target;
		}
	}
		
	return income_chain->default_policy;
}

unsigned int outcome_filter_hookfunc(void* priv,struct sk_buff *skb,const struct nf_hook_state *state){
	
	rule_ptr_t tmp_rule=outcome_chain->head->next;
	int i;
	for(i=0;i<outcome_chain->rule_num;i++,tmp_rule=tmp_rule->next){
		target_t target;
		if((target=check_rule(skb,tmp_rule))!=INIT_TARGET){
			//to-do log


			if(tmp_rule->alarm==ALARM_ON){
				//to-do alarm
			}

			return target;
		}
	}	
	return outcome_chain->default_policy;
}

unsigned int forward_filter_hookfunc(void* priv,struct sk_buff *skb,const struct nf_hook_state *state){
	
	rule_ptr_t tmp_rule=forward_chain->head->next;
	int i;
	for(i=0;i<forward_chain->rule_num;i++,tmp_rule=tmp_rule->next){
		target_t target;
		if((target=check_rule(skb,tmp_rule))!=INIT_TARGET){
			//to-do log


			if(tmp_rule->alarm==ALARM_ON){
				//to-do alarm
			}

			return target;
		}
	}	
	return forward_chain->default_policy;
}

int init_firewall(void){
	//print the entry infomation
	printk(KERN_INFO "firewall module loaded\n");
	
	//register the chains table
	init_chains();

	//set the value of three default filters.
	init_filters();

	//register the hook functions
	//register_hookfunctions();

	//prepare the proc file to receive and send message
	init_proc();

	return 0;
}


/*cleanup function called on the module exit*/
void exit_firewall(void){
	printk(KERN_INFO "exiting the firewall module\n");
	proc_remove(proc_entry);
	proc_remove(log_proc_entry);
	unregister_hookfunctions();
}

void init_filters(){
	//register the local in hook with input_filteer_hookfc
	outcome_filter.hooknum=NF_INET_LOCAL_OUT;
	outcome_filter.hook=(nf_hookfn*)outcome_filter_hookfunc;
	outcome_filter.pf=PF_INET;
	outcome_filter.priority=NF_IP_PRI_FIRST;

	//register the local in hook with output_filteer_hookfc
	income_filter.hooknum=NF_INET_LOCAL_IN;
	income_filter.hook=(nf_hookfn*)income_filter_hookfunc;
	income_filter.pf=PF_INET;
	income_filter.priority=NF_IP_PRI_FIRST;

	//register the local in hook with forward_filteer_hookfc
	forward_filter.hooknum=NF_INET_FORWARD;
	forward_filter.hook=(nf_hookfn*)forward_filter_hookfunc;
	forward_filter.pf=PF_INET;
	forward_filter.priority=NF_IP_PRI_FIRST;
}

void register_hookfunctions(){
	nf_register_net_hook(&init_net,&outcome_filter);
	nf_register_net_hook(&init_net,&income_filter);
	nf_register_net_hook(&init_net,&forward_filter);
	printk(KERN_DEBUG "registered the hook functions\n");
}

void unregister_hookfunctions(){
	nf_unregister_net_hook(&init_net,&outcome_filter);
	nf_unregister_net_hook(&init_net,&income_filter);
	nf_unregister_net_hook(&init_net,&forward_filter);
}
void init_proc(void){
	//create the proc virtual files
	proc_entry=proc_create("firewall",0660,NULL,&myops);
	if(proc_entry == NULL){
		printk(KERN_INFO "an error occurred while creating the proc directory");
		return 1;
	}

	log_proc_entry=proc_create("firewall_log",0660,NULL,&myops_1);
	memset(log_to_read_buffer,0,sizeof(log_to_read_buffer));
	if(log_proc_entry == NULL){
		printk(KERN_INFO "an error occurred while creating the log proc file");
		return 1;
	}
	
}
target_t check_rule(struct sk_buff* skb,rule_ptr_t rule){
	
	struct iphdr *ip_header = ip_hdr(skb);
	struct tcphdr *tcp_header = tcp_hdr(skb);
	struct udphdr *udp_header = udp_hdr(skb);

	target_t match_res = INIT_TARGET;


	//source port
	if(rule->rule_behaviour.sport==INIT_PORT){
		//match continue
	}
	else{
		//source port restriction
		unsigned int s_port = ntohs(tcp_header->source);
		if(s_port!=rule->rule_behaviour.sport&&s_port!=rule->rule_behaviour.sport)return INIT_TARGET;
	}

	//destination port
	if(rule->rule_behaviour.dport==INIT_PORT){
		//match continue
	}
	else{
		//destination port restriction
		unsigned int d_port = ntohs(tcp_header->dest);
		if(d_port!=rule->rule_behaviour.dport&&d_port!=rule->rule_behaviour.dport)return INIT_TARGET;
	}


	//source ip addr
	if(if_init_ip(rule->rule_behaviour.sip)){
		printk("sip not set");
		//match continue
	}
	else{
		unsigned char* rule_saddr=(unsigned char *)rule->rule_behaviour.sip;
		unsigned char* target_saddr = (unsigned char*)&ip_header->saddr;
		printk("check rule function:the rule addr:%x,the target daddr is %x",rule_saddr,target_saddr);
		printk("the rule ip addr:%pI4",rule_saddr);
		printk("the target ip addr:%pI4",target_saddr);
		if(memcmp(rule_saddr,target_saddr,IP_LENGTH))return INIT_TARGET;
	}


	//destination ip addr	
	if(if_init_ip(rule->rule_behaviour.dip)){
		//match continue
		printk("dip not set");

	}
	else{
		unsigned char* rule_daddr=(unsigned char *)rule->rule_behaviour.dip;
		unsigned char* target_daddr = (unsigned char*)&ip_header->daddr;
		printk("check rule function:the rule addr:%x,the target daddr is %x",rule_daddr,target_daddr);
		printk("the rule ip addr:%pI4",rule_daddr);
		printk("the target ip addr:%pI4",target_daddr);
		if(memcmp(rule_daddr,target_daddr,IP_LENGTH))return INIT_TARGET;
	}

	//protocal
	printk("proto number %d",ip_header->protocol);
	if(rule->rule_behaviour.proto==INIT_PROTO){
		printk("proto not set");
		//match continue
	}
	else{

		if(rule->rule_behaviour.proto==TCP&&ip_header->protocol!=6)return INIT_TARGET;
		if(rule->rule_behaviour.proto==UDP&&ip_header->protocol!=17)return INIT_TARGET;
		if(rule->rule_behaviour.proto==ICMP&&ip_header->protocol!=1)return INIT_TARGET;

	}	

	printk("check rule function: rule match");
	return rule->target;
}
/*declare entry and exit functions*/
module_init(init_firewall);
module_exit(exit_firewall);
