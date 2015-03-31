#include<iostream>
#include<fstream>
#include<vector>
#include<set>
#include<map>
#include<cassert>
#include<string>
using namespace std;
//baseNum used for file addressing
int baseNum = 0;
string makeNew()
{
	//Return a new file address
	string ans = "AAAAAA";
	int curr = baseNum;
	baseNum++;
	for(int i=5;i>=0;i--)
	{
		ans[i] += curr % 26;
		curr /= 26;
	}
	return ans;
}
//Max keys in a node
int maxKeys = 0;
void readMaxKeys()
{
	//Read maxKey from predefined file
	string fileName = "bplustree.config";
	ifstream fin(fileName, ios::in);
	fin>>maxKeys;
	fin.close();
}
//Node to write 
struct node
{
	string fName;	//Filename (addressed by 6 characters)
	bool isRoot;	//To denote whether this node is a root or not
	bool isLeaf;	//To denote whether a this is a leaf or not
	int numKeys;	//number of keys in this node
	vector<int> keys;	//keys stored
	vector<string> children;	//Children pointers
};

int main()
{
	readMaxKeys();
	return 0;
}
