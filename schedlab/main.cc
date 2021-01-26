#include "policy.h"
#include <map>
#include <iostream>
using namespace std;

map<int, Event::Task> TaskQueue; 
map<int, Event::Task> TaskIO;

int now_time = -1; 

Action policy(const std::vector<Event>& events, int current_cpu,
              int current_io) {
	
	int event_len=events.size(); 
	
	for(int i = 0; i < event_len; i++)
	{
		int this_type = (int)events[i].type;
		if(this_type == 0)//中断 
			now_time = events[i].time;
		else if(this_type == 1)//新任务 
			TaskQueue.insert(map<int,Event::Task>::value_type(events[i].task.deadline, events[i].task));
		else if(this_type == 2)//任务完成 
		{
			map<int,Event::Task>::iterator iter;
			iter = TaskQueue.begin();
			while(iter != TaskQueue.end())
			{
				if(iter->second.taskId == events[i].task.taskId)
				{
					TaskQueue.erase(iter);
					break;
				}
				iter++;
			}
		}
		else if(this_type == 3)//任务请求I/O
		{
			TaskIO.insert(map<int,Event::Task>::value_type(events[i].task.deadline, events[i].task));
			map<int,Event::Task>::iterator iter;
			iter = TaskQueue.begin();
			while(iter != TaskQueue.end())
			{
				if(iter->second.taskId == events[i].task.taskId)
				{
					TaskQueue.erase(iter);
					break;
				}
				iter++;
			}
		} 
		else if(this_type == 4)//任务I/O完成
		{
			TaskQueue.insert(map<int,Event::Task>::value_type(events[i].task.deadline, events[i].task));
			map<int,Event::Task>::iterator iter;
			iter = TaskIO.begin();
			while(iter != TaskIO.end())
			{
				if(iter->second.taskId == events[i].task.taskId)
				{
					TaskIO.erase(iter);
					break;
				}
				iter++;
			}
		}
	} 
	

	Action choose;
	choose.cpuTask = current_cpu;
	choose.ioTask = current_io;
	 
    if(current_io == 0)
	{
		if(!TaskIO.empty()) 
		{ 
			map<int,Event::Task>::iterator nextIO;
			nextIO = TaskIO.begin();
			while(nextIO != TaskIO.end())
			{
				if(nextIO->first > now_time)
					break;
				nextIO++; 
			}
			if(nextIO == TaskIO.end())//全超过截止时间 
				nextIO = TaskIO.begin();
			choose.ioTask = nextIO->second.taskId;
		} 	
	}
	 
	map<int,Event::Task>::iterator nextCPU;
	nextCPU = TaskQueue.begin();
	while(nextCPU != TaskQueue.end())
	{
		if(nextCPU->first > now_time)
			break;
		nextCPU++;
	}
	if(nextCPU == TaskQueue.end())
		nextCPU = TaskQueue.begin();
		
	choose.cpuTask = nextCPU->second.taskId;
	
    return choose;
}
