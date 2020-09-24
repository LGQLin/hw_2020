#include <iostream> 
#include<stdio.h>
#include <string> 
#include <fstream> 
#include <vector> 
#include<ctime> 
#include<bits/stdc++.h>
using  namespace std;

#define     MAX_STEP    7		//最大步长
#define     MIN_STEP    2		//最小步长
#define		MAX_EDGE	280000	//最大边数
#define		MAX_VERTEX	280000	//最大顶点数
#define		MAX_DEGREE	50		//最大出入度数
#define		INT32_T_LEN 4


const char* File_dir = "test_data.txt";
const char* output_dir = "result.txt";
//const char* File_dir = "/data/test_data.txt";
//const char* output_dir = "/projects/student/result.txt";
//手工栈------------------------------------------------------------------------------------------------------------------------
int32_t input_data[MAX_VERTEX * 2];

vector<vector<int32_t>> G;
vector<vector<int32_t>> G_inv;

vector <int32_t> vertex;
vector<int32_t> vertex_map(MAX_VERTEX);
char write_buf[150000000];

int32_t vertex_num;
int32_t edge_num;

int32_t ans_num[8];
int32_t ans3[3 * 500000];
int32_t ans4[4 * 500000];
int32_t ans5[5 * 1000000];
int32_t ans6[6 * 2000000];
int32_t ans7[7 * 3000000];
int32_t* ans[] = { NULL, NULL, NULL, ans3, ans4, ans5, ans6, ans7 };


//------------------------------------------------------------------------------------------------------------------------------
bool cmp(const int32_t& x, const int32_t& y)
{
	return x > y;
}
void build_graph(const int32_t& cnt)
{
	int32_t i = 0;
	//统计顶点数
	vertex.reserve(cnt);
	vertex.assign(&input_data[0], &input_data[cnt]);
	sort(vertex.begin(), vertex.end());
	vertex.erase(unique(vertex.begin(), vertex.end()), vertex.end());
	vertex_num = vertex.size();
	//id重映射
	for (auto& x : vertex)
		vertex_map[x] = i++;
	//建表
	int32_t father, child;
	G.resize(vertex_num);
	G_inv.resize(vertex_num);
	i = 0;
	while (i < cnt)
	{
		father = vertex_map[input_data[i++]];
		child = vertex_map[input_data[i++]];

		G[father].push_back(child);

		G_inv[child].push_back(father);
	}
	//排序
	for (auto& g : G)
	{
		if (!g.empty())
			sort(g.begin(), g.end(), cmp);
	}
	edge_num = cnt / 2;
}
void data_analyse(char* buf, uint32_t& len)
{
	int32_t cnt = 0, res;
	int32_t num_front = 0, num_back = 0;

	while (num_front < len)
	{
		//-----------------------------------------------
		res = 0;
		while (buf[num_front] != ',')
			num_front++;
		while (num_back < num_front)
			res = res * 10 + buf[num_back++] - '0';
		input_data[cnt++] = res;
		num_back = ++num_front;
		//-----------------------------------------------
		res = 0;
		while (buf[num_front] != ',')
			num_front++;
		while (num_back < num_front)
			res = res * 10 + buf[num_back++] - '0';
		input_data[cnt++] = res;
		//-----------------------------------------------
		while (buf[num_front] != '\n' && num_front < len)
			num_front++;

		num_back = ++num_front;
	}
	build_graph(cnt);
}

bool file_input(const char* File_dir)
{
	FILE* stream = NULL;
	uint32_t char_num, len;
	char* p;
	stream = freopen(File_dir, "r", stdin);
	if (stream)
	{
		fseek(stream, 0, SEEK_END);
		char_num = ftell(stream);
		fseek(stream, 0, SEEK_SET);

		p = new char[char_num];
		len = fread(p, 1, char_num, stdin) - 1;
		data_analyse(p, len);

		delete[] p;
		p = NULL;
		fclose(stream);
		return true;
	}
	else
	{
		cout << "open files erro !" << endl;
		return false;
	}
}

int32_t cycle_count;
bool vis[MAX_VERTEX];
bool vis_inv[MAX_VERTEX];


//mstack path(10);
//mstack inv_mark(MAX_VERTEX);
int32_t path[10];
int32_t inv_mark[MAX_VERTEX];
int32_t dfs_stack[1000];



void dfs_run(const vector<vector<int32_t>>& G)
{
	int32_t top_id;
	int32_t top_index = 0, mark_index = 0, path_index = 0;
	int8_t back_step = 3, forward_step = 3;
	for (int32_t i = 0; i < vertex_num; i++)
	{
		if (i % 100 == 0)
			cout << i << "/" << vertex_num << endl;

		if (!G[i].empty() && !G_inv[i].empty())	//出入度不为0
		{

			//backward serch
			dfs_stack[top_index++] = i;
			while (top_index != 0)
			{
				top_id = dfs_stack[--top_index];
				if (top_id == 0xFFFFFFFF)
				{
					path_index--;
				}
				else
				{
					//__builtin_prefetch(&G_inv[top_id],0,1);
					path[path_index++] = top_id;
					vis_inv[top_id] = true;
					inv_mark[mark_index++] = top_id;	//记录下标记位置，便于清零				
					dfs_stack[top_index++] = 0xFFFFFFFF;
					if (path_index <= back_step)
					{
						for (auto& x : G_inv[top_id])
						{
							if (x > i)
								dfs_stack[top_index++] = x;
						}
					}
				}
			}
			//forward serch
			dfs_stack[top_index++] = i;
			while (top_index != 0)
			{
				top_id = dfs_stack[--top_index];

				if (top_id == 0xFFFFFFFF)
				{
					top_id = path[--path_index];
					vis[top_id] = false;
				}
				else
				{
					//__builtin_prefetch(&G[top_id],0,1);
					path[path_index++] = top_id;
					vis[top_id] = true;
					dfs_stack[top_index++] = 0xFFFFFFFF;
					for (auto& x : G[top_id])
					{
						if (x == i && path_index > 2)
						{
							//拷贝路径
							memcpy(ans[path_index], &path[0], path_index * INT32_T_LEN);
							ans[path_index] += path_index;
							ans_num[path_index]++;
							cycle_count++;
							//cout << path.top_index << endl;
						}

						else if ((path_index <= forward_step || vis_inv[x] && path_index < MAX_STEP) && !vis[x] && x > i)
							dfs_stack[top_index++] = x;
					}
				}
			}
			//clear vis_inv
			while (mark_index > 0)
				vis_inv[inv_mark[--mark_index]] = false;

		}
	}
}

char* array2string(int32_t& len)
{
	int32_t cnt = 0;
	int32_t temp;
	int32_t* num_p;
	char* p_end = &write_buf[150000000];
	char* p = p_end - 1;
	ans[3] = ans3;
	ans[4] = ans4;
	ans[5] = ans5;
	ans[6] = ans6;
	ans[7] = ans7;
	for (int i = 7; i >= 3; i--)
	{
		num_p = ans[i] + ans_num[i] * i - 1;
		while (true)
		{
			*(p--) = '\n';
			while (cnt < i)
			{
				temp = vertex[*(num_p--)];
				//__builtin_prefetch(&vertex[*(num_p)],0,1);
				if (temp == 0)
					*(p--) = '0';
				else
				{
					while (temp > 0)
					{
						*(p--) = temp % 10 + '0';
						temp /= 10;
					}
				}
				*(p--) = ',';
				cnt++;
			}
			p++;
			cnt = 0;
			if (num_p == ans[i] - 1)
				break;
		}
	}
	*(p--) = '\n';
	temp = cycle_count;
	while (temp > 0)
	{
		while (temp > 0)
		{
			*(p--) = temp % 10 + '0';
			temp /= 10;
		}
	}
	p++;
	len = p_end - p;
	return p;
}
void file_output(const char* output_dir)
{
	char* p = NULL;
	int32_t len = 0;
	FILE* stream = fopen(output_dir, "w");
	p = array2string(len);
	fwrite(p, sizeof(char), len, stream);
	fclose(stream);

}

int main()
{
	if (file_input(File_dir))
	{

		dfs_run(G);
		file_output(output_dir);

	}
	return 0;
}

