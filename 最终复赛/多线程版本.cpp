#include <iostream> 
#include<stdio.h>
#include <string> 
#include <fstream> 
#include <vector> 
#include<ctime> 
#include<bits/stdc++.h>
#include<unordered_map>
#include <thread>
using  namespace std;

#define     MAX_STEP		8			//最大步长
#define     MIN_STEP		2			//最小步长
#define		INT32_T_LEN		4
#define		WRITE_BUFF		100000000	
#define		BOUND			WRITE_BUFF - 50000
#define		BACKWARD_STEP	3
#define		FORWARD_STEP	4

// 本地调试路径
const char* File_dir = "test_data.txt";
const char* output_dir = "result.txt";
//-- 上传路径
//const char* File_dir = "/data/test_data.txt";
//const char* output_dir = "/projects/student/result.txt";
//手工栈------------------------------------------------------------------------------------------------------------------------
typedef pair<uint32_t, double> id_money;
vector<uint32_t> input_data;
vector<double> money;
vector<vector<id_money>> G;
vector<vector<id_money>> G_inv;
vector<uint32_t> in_degree;
vector<uint32_t> out_degree;

vector <uint32_t> vertex;
unordered_map<uint32_t, uint32_t> vertex_map;
char write_buf[WRITE_BUFF];

uint32_t vertex_num;		//顶点数
uint32_t edge_num;			//记录数


//------------------------------------------------------------------------------------------------------------------------------
void topsort()
{
	stack<uint32_t> in_stack;
	stack<uint32_t> out_stack;
	uint32_t i, j, id;

	for (i = 0; i < vertex_num; i++)
	{
		if (in_degree[i] == 0)
			in_stack.push(i);
		if (out_degree[i] == 0)
			out_stack.push(i);
	}
	//删除入度为0的节点
	while (!in_stack.empty())
	{
		i = in_stack.top();
		in_stack.pop();
		for (j = 0; j < G[i].size(); j++)
		{
			id = G[i][j].first;
			if (--in_degree[id] == 0)
				in_stack.push(id);
		}
	}
	//删除出度为0的节点
	while (!out_stack.empty())
	{
		i = out_stack.top();
		out_stack.pop();
		for (j = 0; j < G_inv[i].size(); j++)
		{
			id = G_inv[i][j].first;
			if (--out_degree[id] == 0)
				out_stack.push(id);
		}
	}
}

void build_graph(const uint32_t& cnt)
{
	uint32_t i = 0, j = 0;
	//统计顶点数
	vertex.reserve(cnt);
	vertex.assign(input_data.begin(), input_data.end());
	sort(vertex.begin(), vertex.end());
	vertex.erase(unique(vertex.begin(), vertex.end()), vertex.end());
	vertex_num = vertex.size();
	vertex_map.rehash(vertex_num);
	in_degree.resize(vertex_num);
	out_degree.resize(vertex_num);
	//id重映射
	for (i = 0; i < vertex_num; i++)
		vertex_map.insert(pair<uint32_t, uint32_t>(vertex[i], i));

	//建表
	uint32_t father, child;
	G.resize(vertex_num);
	G_inv.resize(vertex_num);
	i = 0;
	j = 0;
	id_money temp;
	while (i < cnt)
	{
		father = vertex_map[input_data[i++]];
		child = vertex_map[input_data[i++]];
		temp.second = money[j++];
		if (temp.second == 0)
			continue;

		temp.first = child;
		G[father].emplace_back(temp);
		out_degree[father]++;			//出度++

		temp.first = father;
		G_inv[child].emplace_back(temp);
		in_degree[child]++;				//入度++
	}
	//拓扑排序
	topsort();
	//排序
	vector<id_money>::iterator it;
	for (auto& g : G)
	{
		it = g.begin();
		while (it != g.end())
		{
			if (out_degree[(*it).first] == 0)
				it = g.erase(it);
			else
				it++;
		}
		if (!g.empty())
			sort(g.begin(), g.end(), [](const id_money& x, const id_money& y)
				{
					return x.first < y.first;
				});
	}
	for (auto& g : G_inv)
	{
		it = g.begin();
		while (it != g.end())
		{
			if (in_degree[(*it).first] == 0)
				it = g.erase(it);
			else
				it++;
		}
	}
	edge_num = cnt / 2;		//记录数
}

bool file_input(const char* File_dir)
{
	uint32_t father_id, child_id;
	double moneys;
	FILE* file = fopen(File_dir, "r");
	if (file)
	{
		while (fscanf(file, "%u,%u,%lf", &father_id, &child_id, &moneys) != EOF)
		{
			input_data.push_back(father_id);
			input_data.push_back(child_id);
			money.push_back(moneys);
		}
		build_graph(input_data.size());
		vector<uint32_t>a;
		vector<double> c;
		unordered_map<uint32_t, uint32_t> b;

		input_data.swap(a);
		money.swap(c);
		vertex_map.swap(b);
		return true;
	}
	else
	{
		cout << "open files erro !" << endl;
		return false;
	}

}

vector<bool> erase_vis;
vector<uint32_t> erase_inv_mark;
//================================= 线程1 =============================================
vector<vector<uint32_t>> ans3, ans4, ans5, ans6, ans7, ans8;
vector<uint32_t> temp_ans3(3), temp_ans4(4), temp_ans5(5), temp_ans6(6), temp_ans7(7), temp_ans8(8);
vector<bool> vis;
vector<bool> vis_inv;
vector<uint32_t> inv_mark;
uint32_t mark_index = 0;
uint32_t path[10];
uint32_t path_index = 0;
uint32_t cycle_count;
void dfs_backward(const uint32_t& target_id, uint32_t cur_id)
{
	path[path_index++] = cur_id;
	vis_inv[cur_id] = true;
	inv_mark[mark_index++] = cur_id;
	if (path_index <= BACKWARD_STEP)
	{
		for (auto& x : G_inv[cur_id])
		{
			if (x.first > target_id)
				dfs_backward(target_id, x.first);
		}
	}
	path_index--;
}
double money_stack[10];
void dfs_forward(const uint32_t& target_id, uint32_t cur_id, double moneys)
{
	bool inLimit = false;
	money_stack[path_index] = moneys;

	path[path_index++] = cur_id;
	vis[cur_id] = true;
	for (auto& x : G[cur_id])
	{

		inLimit = (moneys <= 5ll * x.second && x.second <= 3ll * moneys);

		if (x.first == target_id && path_index > MIN_STEP && inLimit)
		{
			inLimit = (x.second <= 5.0 * money_stack[1] && money_stack[1] <= 3.0 * x.second);
			if (!inLimit)
				continue;
			switch (path_index)
			{
			case 3:
				temp_ans3.assign(&path[0], &path[path_index]);
				ans3.emplace_back(temp_ans3);
				break;
			case 4:
				temp_ans4.assign(&path[0], &path[path_index]);
				ans4.emplace_back(temp_ans4);
				break;
			case 5:
				temp_ans5.assign(&path[0], &path[path_index]);
				ans5.emplace_back(temp_ans5);
				break;
			case 6:
				temp_ans6.assign(&path[0], &path[path_index]);
				ans6.emplace_back(temp_ans6);
				break;
			case 7:
				temp_ans7.assign(&path[0], &path[path_index]);
				ans7.emplace_back(temp_ans7);
				break;
			case 8:
				temp_ans8.assign(&path[0], &path[path_index]);
				ans8.emplace_back(temp_ans8);
				break;
			default:
				break;
			}
			cycle_count++;
		}
		else if ((path_index <= FORWARD_STEP || vis_inv[x.first] && path_index < MAX_STEP) && !vis[x.first] && x.first > target_id)
		{
			if (moneys == 0.0 || inLimit)
				dfs_forward(target_id, x.first, x.second);
		}

	}
	vis[cur_id] = false;
	path_index--;
}
void dfs_run(uint32_t start, uint32_t end)
{
	vis.resize(vertex_num);
	vis_inv.resize(vertex_num);
	inv_mark.resize(vertex_num);
	for (uint32_t i = start; i < end; i++)
	{
		//if (i % 1000 == 0)
			//cout << i << "/" << vertex_num << endl;
		if (!G[i].empty() && !G_inv[i].empty())
		{
			dfs_backward(i, i);
			dfs_forward(i, i, 0.0);
			while (mark_index > 0)
				vis_inv[inv_mark[--mark_index]] = false;
		}
	}

	vis.swap(erase_vis);
	vis_inv.swap(erase_vis);
	inv_mark.swap(erase_inv_mark);
	//cout << "dfs_run exit!" << endl;
}

//================================= 线程2 =============================================
vector<vector<uint32_t>> ans3_2, ans4_2, ans5_2, ans6_2, ans7_2, ans8_2;
vector<uint32_t> temp_ans3_2(3), temp_ans4_2(4), temp_ans5_2(5), temp_ans6_2(6), temp_ans7_2(7), temp_ans8_2(8);
vector<bool> vis_2;
vector<bool> vis_inv_2;
double money_stack_2[10];
vector<uint32_t> inv_mark_2;
uint32_t mark_index_2 = 0;
uint32_t path_2[10];
uint32_t path_index_2 = 0;
uint32_t cycle_count_2;
void dfs_backward_2(const uint32_t& target_id, uint32_t cur_id)
{
	path_2[path_index_2++] = cur_id;
	vis_inv_2[cur_id] = true;
	inv_mark_2[mark_index_2++] = cur_id;
	if (path_index_2 <= BACKWARD_STEP)
	{
		for (auto& x : G_inv[cur_id])
		{
			if (x.first > target_id)
				dfs_backward_2(target_id, x.first);
		}
	}
	path_index_2--;
}

void dfs_forward_2(const uint32_t& target_id, uint32_t cur_id, double moneys)
{
	bool inLimit = false;
	money_stack_2[path_index_2] = moneys;

	path_2[path_index_2++] = cur_id;
	vis_2[cur_id] = true;
	for (auto& x : G[cur_id])
	{

		inLimit = (moneys <= 5ll * x.second && x.second <= 3ll * moneys);

		if (x.first == target_id && path_index_2 > MIN_STEP && inLimit)
		{
			inLimit = (x.second <= 5.0 * money_stack_2[1] && money_stack_2[1] <= 3.0 * x.second);
			if (!inLimit)
				continue;
			switch (path_index_2)
			{
			case 3:
				temp_ans3_2.assign(&path_2[0], &path_2[path_index_2]);
				ans3_2.emplace_back(temp_ans3_2);
				break;
			case 4:
				temp_ans4_2.assign(&path_2[0], &path_2[path_index_2]);
				ans4_2.emplace_back(temp_ans4_2);
				break;
			case 5:
				temp_ans5_2.assign(&path_2[0], &path_2[path_index_2]);
				ans5_2.emplace_back(temp_ans5_2);
				break;
			case 6:
				temp_ans6_2.assign(&path_2[0], &path_2[path_index_2]);
				ans6_2.emplace_back(temp_ans6_2);
				break;
			case 7:
				temp_ans7_2.assign(&path_2[0], &path_2[path_index_2]);
				ans7_2.emplace_back(temp_ans7_2);
				break;
			case 8:
				temp_ans8_2.assign(&path_2[0], &path_2[path_index_2]);
				ans8_2.emplace_back(temp_ans8_2);
				break;
			default:
				break;
			}
			cycle_count_2++;
		}
		else if ((path_index_2 <= FORWARD_STEP || vis_inv_2[x.first] && path_index_2 < MAX_STEP) && !vis_2[x.first] && x.first > target_id)
		{
			if (moneys == 0.0 || inLimit)
				dfs_forward_2(target_id, x.first, x.second);
		}

	}
	vis_2[cur_id] = false;
	path_index_2--;
}
void dfs_run_2(uint32_t start, uint32_t end)
{
	vis_2.resize(vertex_num);
	vis_inv_2.resize(vertex_num);
	inv_mark_2.resize(vertex_num);
	for (uint32_t i = start; i < end; i++)
	{
		//if (i % 1000 == 0)
			//cout << i << "/" << vertex_num << endl;
		if (!G[i].empty() && !G_inv[i].empty())
		{
			dfs_backward_2(i, i);
			dfs_forward_2(i, i, 0.0);
			while (mark_index_2 > 0)
				vis_inv_2[inv_mark_2[--mark_index_2]] = false;
		}
	}
	vis_2.swap(erase_vis);
	vis_inv_2.swap(erase_vis);
	inv_mark_2.swap(erase_inv_mark);
	//cout << "dfs_run_2 exit!" << endl;
}
//================================= 线程3 =============================================
vector<vector<uint32_t>> ans3_3, ans4_3, ans5_3, ans6_3, ans7_3, ans8_3;
vector<uint32_t> temp_ans3_3(3), temp_ans4_3(4), temp_ans5_3(5), temp_ans6_3(6), temp_ans7_3(7), temp_ans8_3(8);
vector<bool> vis_3;
vector<bool> vis_inv_3;
double money_stack_3[10];
vector<uint32_t> inv_mark_3;
uint32_t mark_index_3 = 0;
uint32_t path_3[10];
uint32_t path_index_3 = 0;
uint32_t cycle_count_3;
void dfs_backward_3(const uint32_t& target_id, uint32_t cur_id)
{
	path_3[path_index_3++] = cur_id;
	vis_inv_3[cur_id] = true;
	inv_mark_3[mark_index_3++] = cur_id;
	if (path_index_3 <= BACKWARD_STEP)
	{
		for (auto& x : G_inv[cur_id])
		{
			if (x.first > target_id)
				dfs_backward_3(target_id, x.first);
		}
	}
	path_index_3--;
}

void dfs_forward_3(const uint32_t& target_id, uint32_t cur_id, double moneys)
{
	bool inLimit = false;
	money_stack_3[path_index_3] = moneys;

	path_3[path_index_3++] = cur_id;
	vis_3[cur_id] = true;
	for (auto& x : G[cur_id])
	{

		inLimit = (moneys <= 5ll * x.second && x.second <= 3ll * moneys);

		if (x.first == target_id && path_index_3 > MIN_STEP && inLimit)
		{
			inLimit = (x.second <= 5.0 * money_stack_3[1] && money_stack_3[1] <= 3.0 * x.second);
			if (!inLimit)
				continue;
			switch (path_index_3)
			{
			case 3:
				temp_ans3_3.assign(&path_3[0], &path_3[path_index_3]);
				ans3_3.emplace_back(temp_ans3_3);
				break;
			case 4:
				temp_ans4_3.assign(&path_3[0], &path_3[path_index_3]);
				ans4_3.emplace_back(temp_ans4_3);
				break;
			case 5:
				temp_ans5_3.assign(&path_3[0], &path_3[path_index_3]);
				ans5_3.emplace_back(temp_ans5_3);
				break;
			case 6:
				temp_ans6_3.assign(&path_3[0], &path_3[path_index_3]);
				ans6_3.emplace_back(temp_ans6_3);
				break;
			case 7:
				temp_ans7_3.assign(&path_3[0], &path_3[path_index_3]);
				ans7_3.emplace_back(temp_ans7_3);
				break;
			case 8:
				temp_ans8_3.assign(&path_3[0], &path_3[path_index_3]);
				ans8_3.emplace_back(temp_ans8_3);
				break;
			default:
				break;
			}
			cycle_count_3++;
		}
		else if ((path_index_3 <= FORWARD_STEP || vis_inv_3[x.first] && path_index_3 < MAX_STEP) && !vis_3[x.first] && x.first > target_id)
		{
			if (moneys == 0.0 || inLimit)
				dfs_forward_3(target_id, x.first, x.second);
		}

	}
	vis_3[cur_id] = false;
	path_index_3--;
}
void dfs_run_3(uint32_t start, uint32_t end)
{
	vis_3.resize(vertex_num);
	vis_inv_3.resize(vertex_num);
	inv_mark_3.resize(vertex_num);
	for (uint32_t i = start; i < end; i++)
	{
		//if (i % 1000 == 0)
			//cout << i << "/" << vertex_num << endl;
		if (!G[i].empty() && !G_inv[i].empty())
		{
			dfs_backward_3(i, i);
			dfs_forward_3(i, i, 0.0);
			while (mark_index_3 > 0)
				vis_inv_3[inv_mark_3[--mark_index_3]] = false;
		}
	}
	vis_3.swap(erase_vis);
	vis_inv_3.swap(erase_vis);
	inv_mark_3.swap(erase_inv_mark);
	//cout << "dfs_run_3 exit!" << endl;
}
//================================= 线程4 =============================================
vector<vector<uint32_t>> ans3_4, ans4_4, ans5_4, ans6_4, ans7_4, ans8_4;
vector<uint32_t> temp_ans3_4(3), temp_ans4_4(4), temp_ans5_4(5), temp_ans6_4(6), temp_ans7_4(7), temp_ans8_4(8);
vector<bool> vis_4;
vector<bool> vis_inv_4;
double money_stack_4[10];
vector<uint32_t> inv_mark_4;
uint32_t mark_index_4 = 0;
uint32_t path_4[10];
uint32_t path_index_4 = 0;
uint32_t cycle_count_4;
void dfs_backward_4(const uint32_t& target_id, uint32_t cur_id)
{
	path_4[path_index_4++] = cur_id;
	vis_inv_4[cur_id] = true;
	inv_mark_4[mark_index_4++] = cur_id;
	if (path_index_4 <= BACKWARD_STEP)
	{
		for (auto& x : G_inv[cur_id])
		{
			if (x.first > target_id)
				dfs_backward_4(target_id, x.first);
		}
	}
	path_index_4--;
}

void dfs_forward_4(const uint32_t& target_id, uint32_t cur_id, double moneys)
{
	bool inLimit = false;
	money_stack_4[path_index_4] = moneys;

	path_4[path_index_4++] = cur_id;
	vis_4[cur_id] = true;
	for (auto& x : G[cur_id])
	{

		inLimit = (moneys <= 5ll * x.second && x.second <= 3ll * moneys);

		if (x.first == target_id && path_index_4 > MIN_STEP && inLimit)
		{
			inLimit = (x.second <= 5.0 * money_stack_4[1] && money_stack_4[1] <= 3.0 * x.second);
			if (!inLimit)
				continue;
			switch (path_index_4)
			{
			case 3:
				temp_ans3_4.assign(&path_4[0], &path_4[path_index_4]);
				ans3_4.emplace_back(temp_ans3_4);
				break;
			case 4:
				temp_ans4_4.assign(&path_4[0], &path_4[path_index_4]);
				ans4_4.emplace_back(temp_ans4_4);
				break;
			case 5:
				temp_ans5_4.assign(&path_4[0], &path_4[path_index_4]);
				ans5_4.emplace_back(temp_ans5_4);
				break;
			case 6:
				temp_ans6_4.assign(&path_4[0], &path_4[path_index_4]);
				ans6_4.emplace_back(temp_ans6_4);
				break;
			case 7:
				temp_ans7_4.assign(&path_4[0], &path_4[path_index_4]);
				ans7_4.emplace_back(temp_ans7_4);
				break;
			case 8:
				temp_ans8_4.assign(&path_4[0], &path_4[path_index_4]);
				ans8_4.emplace_back(temp_ans8_4);
				break;
			default:
				break;
			}
			cycle_count_4++;
		}
		else if ((path_index_4 <= FORWARD_STEP || vis_inv_4[x.first] && path_index_4 < MAX_STEP) && !vis_4[x.first] && x.first > target_id)
		{
			if (moneys == 0.0 || inLimit)
				dfs_forward_4(target_id, x.first, x.second);
		}

	}
	vis_4[cur_id] = false;
	path_index_4--;
}
void dfs_run_4(uint32_t start, uint32_t end)
{
	vis_4.resize(vertex_num);
	vis_inv_4.resize(vertex_num);
	inv_mark_4.resize(vertex_num);
	for (uint32_t i = start; i < end; i++)
	{
		//if (i % 1000 == 0)
			//cout << i << "/" << vertex_num << endl;
		if (!G[i].empty() && !G_inv[i].empty())
		{
			dfs_backward_4(i, i);
			dfs_forward_4(i, i, 0.0);
			while (mark_index_4 > 0)
				vis_inv_4[inv_mark_4[--mark_index_4]] = false;
		}
	}
	vis_4.swap(erase_vis);
	vis_inv_4.swap(erase_vis);
	inv_mark_4.swap(erase_inv_mark);
	//cout << "dfs_run_4 exit!" << endl;
}
//=================================== 字符串处理 ========================================
void num2string(char*& buff, uint32_t num)
{
	static char string_arr[50] = { 0 };
	static char* p_end = &string_arr[50];
	uint32_t len = 0;
	char* p = p_end - 1;
	if (num == 0)
		*(p--) = '0';
	else
	{
		while (num > 0)
		{
			*(p--) = num % 10 + '0';
			num /= 10;
		}
	}
	p++;
	len = p_end - p;
	memcpy(buff, p, len);
	buff += len;		//指向下一个空元素
}
void write_ans(vector<vector<uint32_t>>& ans, FILE* stream)
{
	uint32_t len = 0;
	static char* bound = &write_buf[BOUND];
	char* buff = write_buf;
	for (auto& x : ans)
	{
		for (auto& i : x)
		{
			num2string(buff, vertex[i]);
			*(buff++) = ',';
		}
		*(buff - 1) = '\n';
		if (buff > bound)
		{
			len = buff - write_buf;
			buff = write_buf;
			fwrite(buff, sizeof(char), len, stream);
		}
	}
	if (buff > write_buf)
	{
		len = buff - write_buf;
		buff = write_buf;
		fwrite(buff, sizeof(char), len, stream);
	}
}
void write_cycle(FILE* stream)
{
	uint32_t len = 0;
	char* buff = write_buf;
	num2string(buff, cycle_count);
	*(buff++) = '\n';
	len = buff - write_buf;
	buff = write_buf;
	fwrite(buff, sizeof(char), len, stream);
}
void file_output(const char* output_dir)
{
	FILE* stream = fopen(output_dir, "w");
	cycle_count = cycle_count + cycle_count_2 + cycle_count_3 + cycle_count_4;
	//-- 写环
	write_cycle(stream);
	//-- ans3
	write_ans(ans3, stream); write_ans(ans3_2, stream); write_ans(ans3_3, stream); write_ans(ans3_4, stream);
	//-- ans4
	write_ans(ans4, stream); write_ans(ans4_2, stream); write_ans(ans4_3, stream); write_ans(ans4_4, stream);
	//-- ans5
	write_ans(ans5, stream); write_ans(ans5_2, stream); write_ans(ans5_3, stream); write_ans(ans5_4, stream);
	//-- ans6
	write_ans(ans6, stream); write_ans(ans6_2, stream); write_ans(ans6_3, stream); write_ans(ans6_4, stream);
	//-- ans7
	write_ans(ans7, stream); write_ans(ans7_2, stream); write_ans(ans7_3, stream); write_ans(ans7_4, stream);
	//-- ans8
	write_ans(ans8, stream); write_ans(ans8_2, stream); write_ans(ans8_3, stream); write_ans(ans8_4, stream);
	fclose(stream);
}

int main()
{
	int threadNum = 4;
	vector<uint32_t> start(4);
	vector<uint32_t> end(4);
	vector<thread> threads(threadNum);
	if (file_input(File_dir))
	{
		start[0] = 0;
		end[0] = 18 * vertex_num / 400;
		threads[0] = thread(dfs_run, start[0], end[0]);

		start[1] = end[0];
		end[1] = 43 * vertex_num / 400;
		threads[1] = thread(dfs_run_2, start[1], end[1]);

		start[2] = end[1];
		end[2] = 88 * vertex_num / 400;
		threads[2] = thread(dfs_run_3, start[2], end[2]);

		start[3] = end[2];
		end[3] = vertex_num;
		threads[3] = thread(dfs_run_4, start[3], end[3]);

		threads[0].join();
		threads[1].join();
		threads[2].join();
		threads[3].join();

		file_output(output_dir);
		cout << cycle_count << endl;
	}
	return 0;
}

