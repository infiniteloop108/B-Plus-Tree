#include<iostream>
#include<fstream>
#include<vector>
#include<set>
#include<map>
#include<cassert>
#include<string>
using namespace std;
//baseNumNode used for file addressing
int baseNumNode = 0;
int baseDataFile = 0;
string makeNewNode()
{
	//Return a new file address for the node
	string ans = "AAAAAA";
	int curr = baseNumNode;
	baseNumNode++;
	for(int i=5;i>=0;i--)
	{
		ans[i] += curr % 26;
		curr /= 26;
	}
	return "nodes/" + ans  + ".nd";
}
string makeNewDataFile()
{
	//Return a new file address for the node
	string ans = "AAAAAA";
	int curr = baseDataFile;
	baseDataFile++;
	for(int i=5;i>=0;i--)
	{
		ans[i] += curr % 26;
		curr /= 26;
	}
	return "data/" + ans + ".dat";
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
//Node for B+ tree
struct node
{
	string fName;	//Filename (addressed by 6 characters)
	bool isRoot;	//To denote whether this node is a root or not
	bool isLeaf;	//To denote whether a this is a leaf or not
	int numKeys;	//number of keys in this node
	vector<int> keys;	//keys stored
	vector<string> children;	//Children pointers
};
//Write this node to its file
void writeNode(node x)
{
	ofstream fout(x.fName, ios::out);
	fout<<x.isRoot<<endl;
	fout<<x.isLeaf<<endl;
	fout<<x.numKeys<<endl;
	for(int i=0;i<x.numKeys;++i)
		fout<<x.keys[i]<<" ";
	fout<<endl;
	for(int i=0;i<=x.numKeys;++i)
		fout<<x.children[i]<<" ";
}
//Write the data in a file
void writeData(string fName, string data)
{
	ofstream fout(fName,ios::out);
	fout<<data;
	fout.close();
}
//Read a node from a file
node readNode(string fName)
{
	node ans;
	ifstream fin(fName, ios::in);
	fin>>ans.isRoot;
	fin>>ans.isLeaf;
	fin>>ans.numKeys;
	ans.keys.resize(ans.numKeys);
	ans.children.resize(ans.numKeys+1);
	for(int i=0;i<ans.numKeys;++i)fin>>ans.keys[i];
	for(int i=0;i<=ans.numKeys;++i)fin>>ans.children[i];
	return ans;
}
int main()
{
	readMaxKeys();
	
	return 0;
}
