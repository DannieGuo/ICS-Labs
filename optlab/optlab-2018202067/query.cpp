#include <cstdio>
#include "dataload.h"
#define limit_orderdate 19950630

const char lineorder_name[] = "lineorder.tbl";

static __inline__ uint64_t curtick() {
	uint64_t tick;
	unsigned long lo,hi;
	__asm__ __volatile__ ("rdtsc":"=a"(lo),"=d"(hi));
	tick = (uint64_t) hi << 32 | lo;
	return tick;
}

static __inline__ void startTimer(uint64_t *t) {
	(*t) = curtick();
}

static __inline__ void stopTimer(uint64_t *t) {
	(*t) = curtick() - *t;
}


int main() {
	table_info lineorder_table_info;
	FILE * lineorder_file;
	
	//load lineorder table from file
	lineorder_file = fopen(lineorder_name,"r");	
	loadTable(lineorder_file, &lineorder_table_info);

	unsigned int quantity_sum = 0;
	double discount_total_price = 0;
	double tax_discount_total_price = 0;
	unsigned int quantity_sum_with_condition = 0;
	double discount_total_price_with_condition = 0;
	double tax_discount_total_price_with_condition = 0;
	
	uint64_t beg;
	startTimer(&beg);
	
	//you should editor the following the part to accelerate the calculation
	/*--------------------------------*/
	//2*2
	long rows=lineorder_table_info.rows;
	long length=rows-1;
	int i;
	unsigned int quantity_sum1 = 0;
	double discount_total_price1 = 0;
	double tax_discount_total_price1 = 0;
	unsigned int quantity_sum_with_condition1 = 0;
	double discount_total_price_with_condition1 = 0;
	double tax_discount_total_price_with_condition1 = 0;
	
	for (i = 0; i < length ; i+=2) {
		
		int quantity0 = lineorder_table_info.table -> lo_quantity[i];
		int extendedprice0 = lineorder_table_info.table -> lo_extendedprice[i];
		double tax0 = 1 + lineorder_table_info.table -> lo_tax[i];
		double discount0 = 1 - lineorder_table_info.table -> lo_discount[i];
		double dis_x_price0 = discount0 * extendedprice0;
		double dis_x_price_x_tax0 = tax0 * dis_x_price0;
		
		int quantity1 = lineorder_table_info.table -> lo_quantity[i+1];
		int extendedprice1 = lineorder_table_info.table -> lo_extendedprice[i+1];
		double tax1 = 1 + lineorder_table_info.table -> lo_tax[i+1];
		double discount1 = 1 - lineorder_table_info.table -> lo_discount[i+1];
		double dis_x_price1 = discount1 * extendedprice1;
		double dis_x_price_x_tax1 = tax1 * dis_x_price1;
		
		
		
		quantity_sum = quantity_sum + quantity0;
		quantity_sum1 = quantity_sum1 + quantity1;
		discount_total_price = discount_total_price + dis_x_price0;
		discount_total_price1 = discount_total_price1 + dis_x_price1;
		tax_discount_total_price = tax_discount_total_price + dis_x_price_x_tax0;
		tax_discount_total_price1 = tax_discount_total_price1+dis_x_price_x_tax1;

		if (lineorder_table_info.table -> lo_orderdate[i] <= limit_orderdate) {
			quantity_sum_with_condition = quantity_sum_with_condition + quantity0;
			
			discount_total_price_with_condition = discount_total_price_with_condition 
			+ dis_x_price0;
			
			tax_discount_total_price_with_condition = tax_discount_total_price_with_condition 
			+ dis_x_price_x_tax0;
		}
		
		if (lineorder_table_info.table -> lo_orderdate[i+1] <= limit_orderdate) {
			quantity_sum_with_condition1 = quantity_sum_with_condition1 + quantity1;
			
			discount_total_price_with_condition1 = discount_total_price_with_condition1 
			+ dis_x_price1;
			
			tax_discount_total_price_with_condition1 = tax_discount_total_price_with_condition1 
			+ dis_x_price_x_tax1;
		}
	}
	
	for(;i< rows;i++)
	{
		int quantity = lineorder_table_info.table -> lo_quantity[i];
		int extendedprice = lineorder_table_info.table -> lo_extendedprice[i];
		double tax = 1 + lineorder_table_info.table -> lo_tax[i];
		double discount = 1 - lineorder_table_info.table -> lo_discount[i];
		double dis_x_price = discount * extendedprice;
		double dis_x_price_x_tax = tax * dis_x_price;
		
		quantity_sum = quantity_sum + quantity;
		discount_total_price = discount_total_price + dis_x_price;
		tax_discount_total_price = tax_discount_total_price + dis_x_price_x_tax;

		if (lineorder_table_info.table -> lo_orderdate[i] <= limit_orderdate) {
			quantity_sum_with_condition = quantity_sum_with_condition + quantity;
			
			discount_total_price_with_condition = discount_total_price_with_condition 
			+ dis_x_price;
			
			tax_discount_total_price_with_condition = tax_discount_total_price_with_condition 
			+ dis_x_price_x_tax;
		}
	}
	quantity_sum = quantity_sum + quantity_sum1;
	discount_total_price = discount_total_price + discount_total_price1;
	tax_discount_total_price = tax_discount_total_price + tax_discount_total_price1;
	quantity_sum_with_condition = quantity_sum_with_condition + quantity_sum_with_condition1;
	discount_total_price_with_condition = discount_total_price_with_condition + discount_total_price_with_condition1;
	tax_discount_total_price_with_condition = tax_discount_total_price_with_condition + tax_discount_total_price_with_condition1;
	
	/*--------------------------------*/
	
	
	stopTimer(&beg); 

	//output
	printf("%d\n",quantity_sum);
	printf("%0.6lf\n",discount_total_price);
	printf("%0.6lf\n",tax_discount_total_price);
	printf("%d\n",quantity_sum_with_condition);
	printf("%0.6lf\n",discount_total_price_with_condition);
	printf("%0.6lf\n",tax_discount_total_price_with_condition);
	printf("running time is %ld\n", (long)(beg));
}
