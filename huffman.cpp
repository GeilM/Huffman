#include<iostream>
#include<map>
#include<vector>
#include<string>
#include<fstream>
using namespace std;


class Huffman{
public:
	struct Huffman_node{
		char id;
		int freq;
		string code;
		Huffman_node* left;
		Huffman_node* right;
		Huffman_node(char i, int f, Huffman_node* l = NULL, Huffman_node* r = NULL)
		{
			id = i; freq = f; left = l; right = r;
		}
	};
	typedef Huffman_node* ptr;

	ptr root;//huffman����ͷ���
	fstream in, out;
	string in_file, out_file;//����������ļ���
	
	vector<pair<char, unsigned long long> > transfer;//�ѱ���ת��Ϊunsigned long long ����
	
	vector<ptr>v;//����huffman���Ľ���
	map<char, int> raw;//��ÿ����ĸ�ĳ���Ƶ��ͳ�Ƴ���
	map<char, string> coding;//��Ӧ�ַ��ͱ���
//	vector<pair<char, int> > buffer;//���������ַ���Ƶ�ʵ���ʱ����
	int makeup;//��λ

	void insert(ptr& root, char, unsigned long long, unsigned long long);//�ָ���
	void make_code();//�õ�string���͵ı���
	void get_code();//�õ�unsigned long long ���͵ı���
	void inorder(ptr root, string code);//���뺯��
	void rebuild();//�ؽ����ĺ���

	Huffman(string ifile, string ofile) :in_file(ifile), out_file(ofile), root(NULL){}
	void count();//���ı��е��ַ�����Ƶ��ͳ��
	void buildTree();//����huffman��
	void compress();//ѹ���ļ�
	void undo();//��ѹ�ļ�

};

void Huffman::count(){
	in.open(in_file.c_str(), ios::in | ios::binary);
	char c;
	while (true)
	{
		in.get(c);
		raw[c]++;
		if (in.peek() == EOF)
			break;
	}
	in.close();
	/*map<char, int>::iterator it = raw.begin();
	while (it != raw.end())
	{
		it++;
	}
	*/
}

void Huffman::insert(ptr& root, char ch, unsigned long long code, unsigned long long pos){
	if (!root)
	{
		root = new Huffman_node('\0', 0);

	}
	if (pos == 0)
	{
		root->id = ch;
		return;
	}
	if (pos&code)
	{
		insert(root->right, ch, code, pos >> 1);
	}
	else
	{
		insert(root->left, ch, code, pos >> 1);
	}
}
void Huffman::buildTree(){
	map<char, int>::iterator it = raw.begin();
	while (it != raw.end())
	{
		v.push_back(new Huffman_node(it->first, it->second));
		it++;
	}
	size_t i, j, cur = 0;
	size_t len = v.size();
	while (cur < len - 1)
	{
		for (i = cur; i < cur + 2 && i<len; i++)
		for (j = i + 1; j < len; j++)
		if (v[i]->freq > v[j]->freq){
			root = v[i]; v[i] = v[j]; v[j] = root;
		}
		v[cur + 1] = new Huffman_node('\0', v[cur]->freq + v[cur + 1]->freq, v[cur], v[cur + 1]);
		cur++;
	}
	root = v[cur];
	v.clear();
	raw.clear();
	make_code();
}

void Huffman::make_code(){
	inorder(root, "");
}


void Huffman::inorder(ptr root, string code)
{
	if (!root)
		return;
	if (root->left == NULL&&root->right == NULL){
		coding[root->id] = code;
		root->code = code;
		return;
	}
	inorder(root->left, code + '0');
	inorder(root->right, code + '1');
}
void Huffman::get_code()
{
	transfer.clear();
	unsigned long long code;
	map<char, string>::iterator it = coding.begin();
	while (it != coding.end())
	{
		code = 1;//�ȵ�1����1�����λΪ����
		int len = it->second.length();
		for (int i = 0; i < len; i++)
		{
			code <<= 1;
			if (it->second[i] == '1')
				code |= 1;
		}
		transfer.push_back(make_pair(it->first, code));
		it++;
	}
}
void Huffman::compress()
{
	get_code();

	in.open(in_file.c_str(), ios::in | ios::binary);
	out.open(out_file.c_str(), ios::out | ios::binary);
	if (out.fail())
	{
		out << out_file << endl;
		cout << "���ļ�ʧ��\n";
		getchar();
	}
	makeup = 0;
	long con = transfer.size();
	char ch; unsigned long long code;
	out.write((char*)&makeup, sizeof(makeup));//Ϊ��λ��Ϣ��λ
	out.write((char*)&con, sizeof(con));//���������Ϣ
	//int len = transfer.size();
	string temp; int count = 1;
	char buffer = 0;
	for (size_t i = 0; i < con; i++)
	{
		ch = transfer[i].first;
		code = transfer[i].second;
		out.write((char*)&ch, sizeof(ch));
		out.write((char*)&code, sizeof(code));
	}

	while (true)
	{
		in.get(ch);
		temp = coding[ch];
		for (size_t i = 0; i < temp.length(); i++)
		{
			buffer = (buffer << 1) + temp[i] - '0';
			count++;
			if (count == 8)
			{
				out << buffer;
				count = 1; buffer = 0;
			}
		}
		if (in.eof())
			break;
	}

	if (count - 1)
	{
		buffer <<= (8 - count);
		makeup = 8 - count;
		out << buffer;
	}
	else makeup = 0;
	//����λ��Ϣ�����ļ�ͷ
	out.seekg(0, ios::beg);
	out.seekp(0, ios::beg);
	out.write((char*)&makeup, sizeof(makeup));

	in.close();
	out.close();
}

void Huffman::undo()
{
	in.open(in_file.c_str(), ios::in | ios::binary);
	out.open(out_file.c_str(), ios::out | ios::binary);
	long con = 0;
	in.read((char*)&makeup, sizeof(makeup));
	in.read((char*)&con, sizeof(con));

//	int count = 0;
	unsigned long long code;
	char ch;
	while (con--)
	{
		in.read((char*)&ch, sizeof(ch));
		in.read((char*)&code, sizeof(code));
		transfer.push_back(make_pair(ch, code));
	}
	rebuild();

	int pos = 1;
	ptr p = root;


	while (true)
	{

		in.get(ch);
		pos = (1 << 6);
		if (in.peek() == EOF){
			ch >>= makeup;
			pos >>= makeup;
		}

		while (pos)
		{
			if (p->left == NULL && p->right == NULL)
			{
				out << p->id;
				//count++;
				p = root;
			}

			if (ch&pos){
				p = p->right;
			}

			else{
				p = p->left;
			}
			pos >>= 1;
		}
		if (in.peek() == EOF)
			break;
	}

	in.close();
	out.close();

}

void Huffman::rebuild(){
	root = NULL;
	unsigned long long pos;
	string s;
	size_t i = 0;
	for (i; i < transfer.size(); i++)
	{
		pos = 1ll << 63;
		while (!(transfer[i].second&pos)) pos >>= 1;
		insert(root, transfer[i].first, transfer[i].second, pos >> 1);
	}
}

/*
int main(int argc,char* argv[]){
	if (argc!=3){
		cout << "parameter is not enough.\n";
		return 0;
	}
	string in(argv[1]);
	string out(argv[2]);
	fstream buf(in.c_str());
	if (buf.fail())
	{
		cout << "open " << in << " failed\n";
		return 0;
	}
	Huffman h(in,out);
	h.count();
	h.buildTree();
	h.compress();
	//getchar();
	cout << "Compress is done.\n";
	return 0;
}
*/
int main(int argc,char* argv[])
{
      if(argc!=3)
	  {
	     cout<<"parameter is not enough.\n";
		 return 0;
	  }
	  string in(argv[1]);
	  string out(argv[2]);
	  fstream buf(in.c_str());
	  if (buf.fail())
	  {
		  cout << "open " << in << " failed\n";
		  return 0;
	  }
	  Huffman h(in,out);
	  h.undo();
	  cout<<"Uncompress is done\n";
	  return 0;
}
